"""Inspecção PE apenas de leitura; candidatos limitados para REA/Ghidra."""
from __future__ import annotations

import argparse
import bisect
import hashlib
import json
from pathlib import Path
import re
import struct
import sys
import zipfile

ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / "deps"))
from capstone import Cs, CS_ARCH_X86, CS_MODE_64, CS_GRP_JUMP, CS_GRP_RET
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_RIP
import capstone

EXPECTED_SHA = "5380708aaf59992814912122afc3f71d6b18bd41062cde1a5e04fcec4e9f191b"
TARGET = Path("C:/Program Files (x86)/Steam/steamapps/common/Fantasynth/Fantasynth/Binaries/Win64/Fantasynth-Win64-Shipping.exe")
NAMES = ["RCIM_Linear", "RCIM_Constant", "RCIM_Cubic", "ERichCurveInterpMode", "RichCurveKey", "InterpMode", "SetActorHiddenInGame", "SetPlaybackPosition"]
GHIDRA = Path("C:/ORCA/Fantasynt/tools/vendor/ghidra_12.1.4_PUBLIC")


def sha(data):
    return hashlib.sha256(data).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


class PE:
    def __init__(self, data):
        self.data = data
        require(len(data) >= 64 and data[:2] == b"MZ", "DOS signature/header")
        nt = self.u32(0x3c)
        self.check(nt, 24)
        require(data[nt:nt + 4] == b"PE\0\0", "PE signature")
        machine, count = struct.unpack_from("<HH", data, nt + 4)
        require(machine == 0x8664, "Not AMD64")
        require(0 < count <= 96, "Section count")
        optional_size = self.u16(nt + 20)
        self.optional = nt + 24
        self.check(self.optional, optional_size)
        require(optional_size >= 144, "Optional header truncated")
        require(self.u16(self.optional) == 0x20b, "Not PE32+")
        self.imagebase = self.u64(self.optional + 24)
        self.imagesize = self.u32(self.optional + 56)
        self.headersize = self.u32(self.optional + 60)
        require(0 < self.headersize <= len(data), "SizeOfHeaders bounds")
        directories = self.u32(self.optional + 108)
        require(4 <= directories <= 16, "Data directory count")
        require(optional_size >= 112 + directories * 8, "Directory/header bounds")
        self.sections = []
        table = self.optional + optional_size
        self.check(table, count * 40)
        for index in range(count):
            offset = table + index * 40
            name = data[offset:offset + 8].rstrip(b"\0").decode("ascii", "strict")
            virtual_size, rva, raw_size, raw = struct.unpack_from("<IIII", data, offset + 8)
            flags = self.u32(offset + 36)
            if raw_size:
                self.check(raw, raw_size)
            require(rva + max(virtual_size, raw_size) <= self.imagesize, "Section image bounds")
            self.sections.append(dict(name=name, rva=rva, virtual_size=virtual_size, raw_size=raw_size, raw=raw, flags=flags, header_offset=offset, raw_sha256=sha(data[raw:raw + raw_size])))
        for i, a in enumerate(self.sections):
            for b in self.sections[i + 1:]:
                require(not (a["raw_size"] and b["raw_size"] and max(a["raw"], b["raw"]) < min(a["raw"] + a["raw_size"], b["raw"] + b["raw_size"])), "Raw section overlap")
                require(max(a["rva"], b["rva"]) >= min(a["rva"] + max(a["virtual_size"], a["raw_size"]), b["rva"] + max(b["virtual_size"], b["raw_size"])), "Virtual section overlap")
        self.exception_rva, self.exception_size = struct.unpack_from("<II", data, self.optional + 112 + 3 * 8)
        require(self.exception_rva and self.exception_size and self.exception_size % 12 == 0, "Exception directory shape")
        self.exception_offset = self.rva_offset(self.exception_rva, self.exception_size)
        self.ranges = []
        previous_end = -1
        for index in range(self.exception_size // 12):
            offset = self.exception_offset + index * 12
            begin, end, unwind = struct.unpack_from("<III", data, offset)
            require(previous_end <= begin < end, "Runtime function order/overlap/range")
            sec = self.section_at(begin)
            require(sec and sec["flags"] & 0x20000000, "Runtime function not executable")
            self.rva_offset(begin, end - begin)
            unwind_offset = self.rva_offset(unwind, 4)
            unwind_header = data[unwind_offset:unwind_offset + 4]
            version, flags = unwind_header[0] & 7, unwind_header[0] >> 3
            require(version == 1 and flags in (0, 1, 2, 3, 4), "Unsupported UNWIND_INFO version/flags")
            unwind_size = (4 + 2 * unwind_header[2] + 3) & ~3
            self.rva_offset(unwind, unwind_size + (12 if flags == 4 else 4 if flags & 3 else 0))
            self.ranges.append(dict(begin=begin, end=end, unwind=unwind, entry_offset=offset, index=index, unwind_header=unwind_header.hex(), unwind_version=unwind_header[0] & 7, unwind_flags=unwind_header[0] >> 3))
            previous_end = end
        self.begins = [r["begin"] for r in self.ranges]
        self.runtime_index = {(r["begin"], r["end"], r["unwind"]): r for r in self.ranges}

    def check(self, offset, size):
        require(offset >= 0 and size >= 0 and offset + size <= len(self.data), "File bounds")

    def u16(self, offset):
        self.check(offset, 2)
        return struct.unpack_from("<H", self.data, offset)[0]

    def u32(self, offset):
        self.check(offset, 4)
        return struct.unpack_from("<I", self.data, offset)[0]

    def u64(self, offset):
        self.check(offset, 8)
        return struct.unpack_from("<Q", self.data, offset)[0]

    def section_at(self, rva):
        return next((s for s in self.sections if s["rva"] <= rva < s["rva"] + max(s["virtual_size"], s["raw_size"])), None)

    def rva_offset(self, rva, size=1):
        if rva < self.headersize and rva + size <= self.headersize:
            self.check(rva, size)
            return rva
        sec = self.section_at(rva)
        require(sec is not None, "Unmapped RVA")
        delta = rva - sec["rva"]
        require(delta + size <= sec["raw_size"], "RVA beyond file-backed section")
        self.check(sec["raw"] + delta, size)
        return sec["raw"] + delta

    def offset_rva(self, offset):
        sec = next((s for s in self.sections if s["raw"] <= offset < s["raw"] + s["raw_size"]), None)
        require(sec is not None, "Offset outside sections")
        return sec["rva"] + offset - sec["raw"]

    def containing(self, rva):
        i = bisect.bisect_right(self.begins, rva) - 1
        return self.ranges[i] if i >= 0 and rva < self.ranges[i]["end"] else None

    def primary_runtime(self, runtime):
        chain = []
        seen = set()
        while runtime["unwind_flags"] & 4:
            require(runtime["unwind_version"] == 1, "Unsupported chained UNWIND_INFO version")
            require(runtime["unwind_flags"] == 4, "Invalid chained handler flag combination")
            require(runtime["unwind"] not in seen and len(chain) < 32, "Unwind chain cycle/limit")
            seen.add(runtime["unwind"])
            offset = self.rva_offset(runtime["unwind"], 4)
            count = self.data[offset + 2]
            relative = (4 + count * 2 + 3) & ~3
            tail = self.rva_offset(runtime["unwind"] + relative, 12)
            fields = struct.unpack_from("<III", self.data, tail)
            require(fields in self.runtime_index, "Chained runtime record absent from .pdata")
            chain.append(dict(begin_va=hex(self.imagebase + runtime["begin"]), pdata_index=runtime["index"], unwind_rva=hex(runtime["unwind"]), chained_record_file_offset=hex(tail), chained_record_sha256=sha(self.data[tail:tail + 12]), parent_begin_va=hex(self.imagebase + fields[0])))
            runtime = self.runtime_index[fields]
        return runtime, chain

    def location(self, rva):
        return dict(rva=hex(rva), va=hex(self.imagebase + rva), file_offset=hex(self.rva_offset(rva)), section=self.section_at(rva)["name"])


def occurrences(data, pattern):
    start = 0
    while True:
        start = data.find(pattern, start)
        if start < 0:
            return
        yield start
        start += 1


def reject_checks(data, pe):
    cases = []

    def rejected(name, callback):
        try:
            callback()
        except (ValueError, struct.error, UnicodeDecodeError):
            cases.append(dict(name=name, rejected=True))
        else:
            raise AssertionError("Did not reject: " + name)

    rejected("Truncated DOS header", lambda: PE(data[:32]))
    changed = bytearray(data)
    changed[0] = 0
    rejected("Corrupt DOS signature", lambda: PE(changed))
    changed = bytearray(data)
    struct.pack_into("<I", changed, 0x3c, len(data) + 1)
    rejected("Out-of-bounds e_lfanew", lambda: PE(changed))
    changed = bytearray(data)
    struct.pack_into("<I", changed, pe.sections[0]["header_offset"] + 20, len(data))
    rejected("Out-of-bounds raw section", lambda: PE(changed))
    changed = bytearray(data)
    struct.pack_into("<I", changed, pe.optional + 112 + 3 * 8 + 4, pe.exception_size - 1)
    rejected("Malformed exception-directory length", lambda: PE(changed))
    changed = bytearray(data)
    struct.pack_into("<I", changed, pe.exception_offset + 4, pe.ranges[0]["begin"])
    rejected("Empty runtime-function range", lambda: PE(changed))
    rejected("Unmapped RVA", lambda: pe.rva_offset(pe.imagesize + 1))
    rejected("Expected target digest mismatch", lambda: require(sha(data) == "0" * 64, "Digest mismatch"))
    return cases


def decode_range(pe, runtime, decoder):
    """Percorre fluxo local desde BeginAddress; não atravessa limites .pdata."""
    begin, end = runtime["begin"], runtime["end"]
    if end - begin > 131072:
        return {}, dict(status="range-size-limit", bytes=end - begin)
    offset = pe.rva_offset(begin, end - begin)
    code = pe.data[offset:offset + end - begin]
    pending = [begin]
    instructions = {}
    failures = []
    occupied = set()
    capped = False
    while pending and len(instructions) < 30000:
        rva = pending.pop()
        if not begin <= rva < end or rva in instructions:
            continue
        if rva in occupied:
            failures.append(dict(rva=hex(rva), reason="branch-target-inside-decoded-instruction"))
            continue
        next_rva = rva
        stopped = False
        for insn in decoder.disasm(code[rva - begin:], pe.imagebase + rva):
            current = insn.address - pe.imagebase
            if len(instructions) >= 30000:
                capped = True
                stopped = True
                break
            if current in instructions:
                stopped = True
                break
            if current + insn.size > end or any(b in occupied for b in range(current, current + insn.size)):
                failures.append(dict(rva=hex(current), reason="range-boundary-or-instruction-overlap"))
                stopped = True
                break
            instructions[current] = insn
            occupied.update(range(current, current + insn.size))
            next_rva = current + insn.size
            if insn.group(CS_GRP_RET) or insn.mnemonic in ("int3", "ud2", "hlt"):
                stopped = True
                break
            if insn.group(CS_GRP_JUMP):
                for operand in insn.operands:
                    if operand.type == X86_OP_IMM:
                        target = operand.imm - pe.imagebase
                        if begin <= target < end:
                            pending.append(target)
                if insn.mnemonic == "jmp":
                    stopped = True
                    break
        if not stopped and next_rva < end:
            failures.append(dict(rva=hex(next_rva), reason="undecodable-tail"))
    return instructions, dict(status="instruction-limit" if capped or pending else "bounded-local-cfg", decoded_instructions=len(instructions), failed_blocks=failures, complete_function_claim=False)


def analyzer_evidence():
    archive = GHIDRA / "Ghidra/Features/Base/lib/Base-src.zip"
    classes = ["ghidra/app/plugin/core/analysis/OperandReferenceAnalyzer.java", "ghidra/app/plugin/core/disassembler/EntryPointAnalyzer.java", "ghidra/app/plugin/core/function/StackVariableAnalyzer.java"]
    result = dict(archive=str(archive), archive_sha256=sha(archive.read_bytes()), classes=[])
    with zipfile.ZipFile(archive) as z:
        for member in classes:
            source = z.read(member)
            lines = source.decode().splitlines()
            line, declaration = next((i + 1, text.strip()) for i, text in enumerate(lines) if re.search(r"String NAME\s*=", text))
            result["classes"].append(dict(member=member, source_sha256=sha(source), line=line, declaration=declaration, display_name=re.search(r'"([^"]+)"', declaration)[1]))
    fid_archive = GHIDRA / "Ghidra/Features/FunctionID/lib/FunctionID-src.zip"
    with zipfile.ZipFile(fid_archive) as z:
        member = "ghidra/feature/fid/analyzer/FidAnalyzer.java"
        source = z.read(member)
        line, declaration = next((i + 1, text.strip()) for i, text in enumerate(source.decode().splitlines()) if re.search(r"String NAME\s*=", text))
        result["classes"].append(dict(archive=str(fid_archive), archive_sha256=sha(fid_archive.read_bytes()), member=member, source_sha256=sha(source), line=line, declaration=declaration, display_name=re.search(r'"([^"]+)"', declaration)[1]))
    return result


def decoder_checks():
    class SyntheticPE:
        imagebase = 0x140000000

        def __init__(self, code):
            self.data = code

        def rva_offset(self, rva, size=1):
            require(0 <= rva and rva + size <= len(self.data), "Synthetic bounds")
            return rva

    decoder = Cs(CS_ARCH_X86, CS_MODE_64)
    decoder.detail = True
    results = []
    for name, code, expected in [("Undecodable tail reported", b"\x90\x0f", "tail"), ("Instruction cap inside single block", b"\x90" * 30001, "cap"), ("Branch into instruction rejected", b"\xeb\xff", "overlap")]:
        instructions, coverage = decode_range(SyntheticPE(code), dict(begin=0, end=len(code)), decoder)
        passed = bool(coverage["failed_blocks"]) if expected != "cap" else coverage["status"] == "instruction-limit" and len(instructions) == 30000
        require(passed, "Decoder regression: " + name)
        results.append(dict(name=name, passed=passed, coverage=coverage))
    return results


def run(target):
    resolved = target.resolve(strict=True)
    require(resolved != ROOT and ROOT not in resolved.parents, "Original must be outside output directory")
    stat_before = resolved.stat()
    with resolved.open("rb") as f:
        data = f.read()
    digest = sha(data)
    require(digest == EXPECTED_SHA, "Target SHA-256 mismatch; no results emitted")
    pe = PE(data)
    checks = reject_checks(data, pe)
    strings = []
    for name in NAMES:
        for encoding in ("ascii", "utf-16le"):
            pattern = name.encode(encoding)
            for offset in occurrences(data, pattern):
                try:
                    rva = pe.offset_rva(offset)
                    pe.rva_offset(rva, len(pattern))
                except ValueError:
                    continue
                terminator = b"\0" if encoding == "ascii" else b"\0\0"
                strings.append(dict(name=name, encoding=encoding, **pe.location(rva), byte_length=len(pattern), nul_terminated=data[offset + len(pattern):offset + len(pattern) + len(terminator)] == terminator, previous_bytes=data[max(0, offset - len(terminator)):offset].hex()))
    pointer_refs = []
    for string in strings:
        for sec in pe.sections:
            if sec["flags"] & 0x20000000 or not sec["raw_size"]:
                continue
            area = data[sec["raw"]:sec["raw"] + sec["raw_size"]]
            for relative in occurrences(area, struct.pack("<Q", int(string["va"], 16))):
                rva = sec["rva"] + relative
                pointer_refs.append(dict(**pe.location(rva), name=string["name"], encoding=string["encoding"], target_va=string["va"], aligned_8=rva % 8 == 0))
    targets = {}
    for value, kind in [(s, "string") for s in strings] + [(p, "string-pointer-slot") for p in pointer_refs]:
        targets.setdefault(int(value["va"], 16), []).append(dict(name=value["name"], encoding=value["encoding"], kind=kind))
    raw_refs = []
    # Filtro de bytes; só a descodificação de fluxo a partir de .pdata admite referências.
    pattern = re.compile(rb"(?=([\x8d\x8b][\x05\x0d\x15\x1d\x25\x2d\x35\x3d][\x00-\xff]{4}))")
    for sec in pe.sections:
        if sec["name"] != ".text":
            continue
        area = data[sec["raw"]:sec["raw"] + sec["raw_size"]]
        for match in pattern.finditer(area):
            rva = sec["rva"] + match.start()
            filtered = match[1]
            target_va = pe.imagebase + rva + len(filtered) + struct.unpack("<i", filtered[-4:])[0]
            if target_va in targets:
                raw_refs.append(dict(rva=rva, target_va=target_va, bytes=filtered.hex(), runtime=pe.containing(rva)))
    decoder = Cs(CS_ARCH_X86, CS_MODE_64)
    decoder.detail = True
    decoded = {}

    def get_decode(runtime):
        begin = runtime["begin"]
        if begin not in decoded:
            decoded[begin] = decode_range(pe, runtime, decoder)
        return decoded[begin]

    verified_refs, rejected_refs = [], []
    candidates = {}

    def add_candidate(runtime, names, reason, extra=None):
        fragment = runtime
        runtime, chain = pe.primary_runtime(runtime)
        begin = runtime["begin"]
        instructions, coverage = get_decode(runtime)
        if begin not in instructions:
            return False
        candidate = candidates.setdefault(begin, dict(**pe.location(begin), end_rva=hex(runtime["end"]), unwind_rva=hex(runtime["unwind"]), unwind_file_offset=hex(pe.rva_offset(runtime["unwind"])), unwind_header=runtime["unwind_header"], unwind_version=runtime["unwind_version"], unwind_flags=runtime["unwind_flags"], pdata_index=runtime["index"], pdata_entry_file_offset=hex(runtime["entry_offset"]), pdata_entry_sha256=sha(data[runtime["entry_offset"]:runtime["entry_offset"] + 12]), range_sha256=sha(data[pe.rva_offset(begin):pe.rva_offset(begin) + runtime["end"] - begin]), names=[], reasons=[], entry_instructions=[dict(va=hex(i.address), bytes=i.bytes.hex(), mnemonic=i.mnemonic, operands=i.op_str) for _, i in sorted(instructions.items())[:8]], coverage=coverage))
        candidate["names"] = sorted(set(candidate["names"] + names))
        candidate["reasons"].append(dict(kind=reason, detail=extra, referencing_range_begin_va=hex(pe.imagebase + fragment["begin"]), unwind_chain=chain))
        return True

    for raw in raw_refs:
        runtime = raw["runtime"]
        reference = dict(**pe.location(raw["rva"]), target_va=hex(raw["target_va"]), byte_filter=raw["bytes"], targets=targets[raw["target_va"]])
        if not runtime:
            rejected_refs.append(dict(**reference, reason="No containing .pdata entry"))
            continue
        instructions, coverage = get_decode(runtime)
        starts = sorted(instructions)
        position = bisect.bisect_right(starts, raw["rva"]) - 1
        insn = instructions[starts[position]] if position >= 0 else None
        # O opcode filtrado tem de pertencer a uma instrução descodificada real.
        match = insn and insn.address - pe.imagebase <= raw["rva"] < insn.address - pe.imagebase + insn.size and insn.mnemonic in ("lea", "mov") and any(op.type == X86_OP_MEM and op.mem.base == X86_REG_RIP and insn.address + insn.size + op.mem.disp == raw["target_va"] for op in insn.operands)
        if not match:
            rejected_refs.append(dict(**reference, reason="Not a decoded reachable RIP LEA/MOV", coverage=coverage))
            continue
        reference.update(instruction_va=hex(insn.address), instruction_file_offset=hex(pe.rva_offset(insn.address - pe.imagebase)), mnemonic=insn.mnemonic, operands=insn.op_str, bytes=insn.bytes.hex(), containing_begin_va=hex(pe.imagebase + runtime["begin"]), pdata_index=runtime["index"])
        primary, chain = pe.primary_runtime(runtime)
        reference.update(primary_unwind_begin_va=hex(pe.imagebase + primary["begin"]), unwind_chain=chain)
        verified_refs.append(reference)
        add_candidate(runtime, [x["name"] for x in targets[raw["target_va"]]], "verified-rip-reference", dict(instruction_va=hex(insn.address), target_va=hex(raw["target_va"])))
    adjacency = []
    for pointer in pointer_refs:
        if not pointer["aligned_8"]:
            continue
        slot = int(pointer["rva"], 16)
        for delta in (-32, -24, -16, -8, 8, 16, 24, 32):
            neighbor = slot + delta
            try:
                sec = pe.section_at(neighbor)
                if not sec or sec["name"] != pointer["section"]:
                    continue
                code_va = pe.u64(pe.rva_offset(neighbor, 8))
            except ValueError:
                continue
            runtime = pe.containing(code_va - pe.imagebase)
            if runtime and code_va - pe.imagebase == runtime["begin"]:
                detail = dict(string_pointer_slot_va=pointer["va"], neighboring_pointer_slot_va=hex(pe.imagebase + neighbor), delta_bytes=delta, target_va=hex(code_va), name=pointer["name"], association="Adjacent pointer only; registration-layout hypothesis, not semantic proof")
                if add_candidate(runtime, [pointer["name"]], "adjacent-code-pointer-hypothesis", detail):
                    adjacency.append(detail)
    def ranking(c):
        rip = any(r["kind"] == "verified-rip-reference" for r in c["reasons"])
        return (not rip, not any(n in ("SetActorHiddenInGame", "SetPlaybackPosition") for n in c["names"]), int(c["va"], 16))
    selected = sorted(candidates.values(), key=ranking)[:30]
    for candidate in selected:
        names = "_".join(candidate["names"][:3])
        candidate["seed_name"] = "candidate_" + names + "_" + candidate["rva"][2:]
        instructions, _ = decoded[int(candidate["rva"], 16)]
        disassembly = "".join(f"0x{i.address:x}\t{i.bytes.hex()}\t{i.mnemonic}\t{i.op_str}\n" for _, i in sorted(instructions.items()))
        path = ROOT / (candidate["rva"] + ".disassembly.tsv")
        path.write_text(disassembly, encoding="utf-8", newline="\n")
        candidate["disassembly_file"] = path.name
        candidate["disassembly_sha256"] = sha(disassembly.encode())
    seeds = "".join(c["va"] + "\tfunction\t" + c["seed_name"] + "\n" for c in selected)
    with resolved.open("rb") as f:
        digest_after = sha(f.read())
    stat_after = resolved.stat()
    require(digest_after == digest and stat_before.st_size == stat_after.st_size and stat_before.st_mtime_ns == stat_after.st_mtime_ns, "Original changed during inspection")
    output = dict(target=str(resolved), sha256=digest, sha256_after=digest_after, original_open_modes=["rb", "rb"], original_size=stat_after.st_size, original_mtime_ns_before=stat_before.st_mtime_ns, original_mtime_ns_after=stat_after.st_mtime_ns, capstone_version=capstone.__version__, script_sha256=sha(Path(__file__).read_bytes()), image_base=hex(pe.imagebase), sections=pe.sections, exception_directory=dict(**pe.location(pe.exception_rva), size=pe.exception_size, entries=len(pe.ranges), sha256=sha(data[pe.exception_offset:pe.exception_offset + pe.exception_size])), strings=strings, data_pointer_refs=pointer_refs, byte_filter_candidate_count=len(raw_refs), verified_rip_refs=verified_refs, rejected_byte_filter_refs=rejected_refs, adjacent_code_pointer_hypotheses=adjacency, candidate_total=len(candidates), selected_candidates=selected, unselected_candidates=[c for c in sorted(candidates.values(), key=ranking) if c not in selected], seeds_sha256=sha(seeds.encode()), rejection_checks=checks, ghidra_analyzers=analyzer_evidence(), limits=["Static evidence only; target never executed", "Unwind runtime ranges are not proof of original C++ names or complete functions; leaf functions may be absent", "RIP LEA/MOV filter covers the specified one-byte opcodes only; no exhaustive xref claim", "Only .text scanned for RIP references; direct pointers scanned in non-executable file-backed sections", "Substring occurrences retained and termination/boundary bytes reported; names are not automatically standalone symbols", "CFG stays inside one .pdata range; indirect jumps and cross-range branches are not followed", "Adjacent pointers within 32 bytes are hypotheses, not established registration layouts", "At most 30 inferential function seeds; seed creation/decompilation still requires REA/Ghidra verification"])
    output["decoder_regression_checks"] = decoder_checks()
    output["referencing_runtime_ranges"] = [r for r in pe.ranges if r["begin"] in {x["runtime"]["begin"] for x in raw_refs if x["runtime"]}]
    output["decoded_range_files"] = []
    for begin, (instructions, coverage) in sorted(decoded.items()):
        disassembly = "".join(f"0x{i.address:x}\t{i.bytes.hex()}\t{i.mnemonic}\t{i.op_str}\n" for _, i in sorted(instructions.items()))
        path = ROOT / (hex(begin) + ".disassembly.tsv")
        path.write_text(disassembly, encoding="utf-8", newline="\n")
        output["decoded_range_files"].append(dict(begin_va=hex(pe.imagebase + begin), path=path.name, sha256=sha(disassembly.encode()), coverage=coverage))
    priorities = [0x140402860, 0x14049B6F0, 0x1403933A0, 0x1403A6930]
    by_address = {int(c["va"], 16): c for c in selected}
    require(all(address in by_address for address in priorities), "Priority addresses absent from verified candidates")
    priority_seeds = "".join(by_address[a]["va"] + "\tfunction\t" + by_address[a]["seed_name"] + "\n" for a in priorities)
    (ROOT / "priority-seeds.tsv").write_text(priority_seeds, encoding="utf-8", newline="\n")
    output["priority_seeds"] = dict(addresses=[hex(a) for a in priorities], sha256=sha(priority_seeds.encode()), limit=4, inference="Primary unwind starts. These are metadata-reference candidates, not confirmed implementation functions.")
    ROOT.mkdir(exist_ok=True)
    (ROOT / "seeds.tsv").write_text(seeds, encoding="utf-8", newline="\n")
    (ROOT / "evidence.json").write_text(json.dumps(output, indent=2, ensure_ascii=False), encoding="utf-8")
    runtime_bytes = "".join(f"{r['index']}\t0x{pe.imagebase+r['begin']:x}\t0x{pe.imagebase+r['end']:x}\t0x{r['unwind']:x}\t0x{r['entry_offset']:x}\n" for r in pe.ranges)
    (ROOT / "pdata-ranges.tsv").write_text(runtime_bytes, encoding="utf-8", newline="\n")
    rows = "\n".join(f"| {c['va']} | {', '.join(c['names'])} | {c['pdata_index']} |" for c in selected)
    report = f"""# Inspecção nativa estática limitada

O original foi aberto duas vezes em `rb`, sem execução, e conserva SHA-256 `{digest}` e data de modificação. A análise não chamou REA/Ghidra nem alterou a instalação, as fontes upstream ou o jogo.

`priority-seeds.tsv` contém quatro entradas primárias de desenrolamento verificadas e executáveis: enumeração, propriedades RichCurveKey/InterpMode, SetActorHiddenInGame e SetPlaybackPosition. `seeds.tsv` contém os 25 candidatos primários. Os 28 intervalos que contêm referências, incluindo fragmentos encadeados, estão em `evidence.json` (`referencing_runtime_ranges`).

Foram encontradas 49 ocorrências ASCII/UTF-16LE, 34 referências RIP LEA/MOV verificadas por Capstone 5.0.6 e nenhum ponteiro absoluto de 64 bits nas secções não executáveis. A pesquisa de ponteiros é limitada a esse formato e não exclui outros esquemas. `.pdata` contém 136107 entradas; o resumo SHA-256 do directório é `{output['exception_directory']['sha256']}`. Todos os intervalos constam de `pdata-ranges.tsv`; os resumos das secções, entradas seleccionadas, fragmentos encadeados e descodificações constam da evidência.

O intervalo `0x140402921` referencia ERichCurveInterpMode e RCIM_Linear/Constant/Cubic, e encadeia para a entrada primária `0x140402860`. O candidato `0x14049b6f0` reúne referências RichCurveKey e InterpMode por fragmentos encadeados. Estas observações sustentam metadados de reflexão; não demonstram fórmulas de interpolação, o método original C++ nem comportamento em execução. O encadeamento de desenrolamento associa fragmentos a uma entrada primária, mas não prova um percurso de controlo desde essa entrada até cada referência.

Os limites de instrução e de tamanho aplicam-se por intervalo .pdata; saltos indirectos e saídas do intervalo não são seguidos. As descodificações são TSV com endereço, bytes, mnemónica e operandos. O início/fim de um intervalo de desenrolamento não prova uma função C++ completa; funções folha podem faltar em .pdata. Os nomes `candidate_*` são inferenciais.

A interpretação de RUNTIME_FUNCTION, alinhamento UNWIND_INFO e UNW_FLAG_CHAININFO segue a [documentação oficial Microsoft](https://learn.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-170), consultada nesta investigação. Os dados específicos e as cadeias são extraídos do executável seleccionado.

Os nomes exactos dos analisadores foram lidos dos arquivos de código-fonte da instalação Ghidra 12.1.4, com resumos e linhas em `ghidra_analyzers`: **Disassemble Entry Points**, **Reference**, **Stack**, **Function ID**. Nenhuma opção foi alterada.

Verificação: oito casos rejeitados de PE/resumo, três regressões de descodificação e SHA-256/mtime antes/depois. A revisão independente identificou três problemas do guião inicial (cauda não descodificada omitida, limite por bloco e prefixo REX no filtro); foram corrigidos e os dois primeiros têm casos de regressão explícitos. O filtro agora pesquisa opcodes sobrepostos sem consumir REX e valida a instrução contendo o opcode.

Reprodução: `python C:/ORCA/Fantasynt/plan/rea-investigation/targeted-native/inspect_native.py`. Dependência confinada a `targeted-native/deps`: `python -m pip install --target C:/ORCA/Fantasynt/plan/rea-investigation/targeted-native/deps capstone==5.0.6`. O guião recusa qualquer ficheiro cujo SHA-256 difira do original seleccionado. Os ficheiros de sementes usam o formato público REA `address<TAB>function<TAB>name`; a aplicação e a descompilação ainda requerem confirmação pelo fornecedor.

| Entrada primária candidata | Referências observadas | Índice .pdata |
|---|---|---|
{rows}
"""
    (ROOT / "README.md").write_text(report, encoding="utf-8")
    print(json.dumps(dict(sha256=digest, strings=len(strings), data_pointers=len(pointer_refs), verified_rip_refs=len(verified_refs), candidates=len(candidates), selected=len(selected), rejection_checks=len(checks), seeds=str(ROOT / "seeds.tsv"))))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--target", type=Path, default=TARGET)
    run(parser.parse_args().target)
