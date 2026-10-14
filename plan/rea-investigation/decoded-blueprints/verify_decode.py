"""Descodificação local limitada de Blueprint; não é REA Evidence ou execução.

Uso: python -B plan/rea-investigation/decoded-blueprints/verify_decode.py
Lê o PAK original, valida os hashes e escreve apenas neste directório.
"""

from __future__ import annotations

import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys
from typing import Any


DIRECTORY = Path(__file__).resolve().parent
ROOT = DIRECTORY.parents[2]
SOURCE_LABEL = "local format-aware Blueprint decoding; not REA Evidence or runtime"
FUNCTIONS = {
    "BeatDirector": {
        "ExecuteUbergraph_BeatDirector", "ReceiveTick", "Start", "Stop", "Reset"
    },
    "ActorHiddenSequencer": {
        "ExecuteUbergraph_ActorHiddenSequencer", "OnActorAction",
        "UserConstructionScript"
    },
}
TOKENS = {
    0x00: "EX_LocalVariable", 0x01: "EX_InstanceVariable",
    0x04: "EX_Return", 0x06: "EX_Jump", 0x07: "EX_JumpIfNot",
    0x0B: "EX_Nothing", 0x0F: "EX_Let", 0x14: "EX_LetBool",
    0x16: "EX_EndFunctionParms", 0x30: "EX_EndStructConst",
    0x17: "EX_Self", 0x19: "EX_Context", 0x1B: "EX_VirtualFunction",
    0x1C: "EX_FinalFunction", 0x1D: "EX_IntConst", 0x1E: "EX_FloatConst",
    0x1F: "EX_StringConst", 0x20: "EX_ObjectConst", 0x24: "EX_ByteConst",
    0x25: "EX_IntZero", 0x27: "EX_True", 0x28: "EX_False",
    0x2F: "EX_StructConst", 0x42: "EX_StructMemberContext",
    0x4C: "EX_PushExecutionFlow", 0x4D: "EX_PopExecutionFlow",
    0x4E: "EX_ComputedJump", 0x53: "EX_EndOfScript",
    0x63: "EX_CallMulticastDelegate", 0x64: "EX_LetValueOnPersistentFrame",
    0x68: "EX_CallMath", 0x69: "EX_SwitchValue",
}


def require(condition: bool, message: str) -> None:
    """Falha explicitamente, também quando Python usa a opção -O."""
    if not condition:
        raise ValueError(message)


class Decoder:
    """Perfil dos scripts seleccionados: ponteiros 4→8, nomes 8→12 bytes."""

    def __init__(self, script: bytes, metadata: dict[str, Any]) -> None:
        self.script = script
        self.metadata = metadata
        self.storage = 0
        self.memory = 0
        self.instructions: list[dict[str, Any]] = []
        self.top_level: list[int] = []
        self.depth = 0

    def take(self, count: int, memory_count: int | None = None) -> bytes:
        value = self.script[self.storage:self.storage + count]
        require(len(value) == count, "Leitura fora dos limites do script")
        self.storage += count
        self.memory += count if memory_count is None else memory_count
        return value

    def integer(self) -> int:
        return struct.unpack("<i", self.take(4))[0]

    def pointer(self, role: str, allow_null: bool = False) -> dict[str, Any]:
        index = struct.unpack("<i", self.take(4, 8))[0]
        table = self.metadata["exports" if index > 0 else "imports"]
        if index == 0:
            require(allow_null, f"Referência null não permitida: {role}")
            return {"package_index": 0, "name": "null"}
        slot = index - 1 if index > 0 else -index - 1
        require(0 <= slot < len(table), f"Referência inválida: {index}")
        entry = table[slot]
        if index < 0:
            kind = entry["class"]
        else:
            class_index = entry["class_index"]
            require(class_index != 0, "Classe da referência não resolvida neste perfil")
            class_table = self.metadata["imports" if class_index < 0 else "exports"]
            class_slot = -class_index - 1 if class_index < 0 else class_index - 1
            require(0 <= class_slot < len(class_table), "Índice de classe inválido")
            kind = class_table[class_slot]["name"]
        valid = {"property": kind.endswith("Property"), "function": kind == "Function",
                 "struct": kind == "ScriptStruct",
                 "object": kind != "Function" and not kind.endswith("Property")}
        require(valid[role], f"Tipo {kind} incompatível com referência {role}")
        return {"package_index": index, "name": entry["name"], "class": kind}

    def arguments(self) -> list[dict[str, Any]]:
        values = []
        while True:
            require(self.storage < len(self.script), "Argumentos sem terminador")
            if self.script[self.storage] == 0x16:
                self.expression(allowed_terminal=0x16)
                return values
            values.append(self.expression())

    def expression(self, allowed_terminal: int | None = None) -> dict[str, Any]:
        start, memory_start = self.storage, self.memory
        token = self.take(1)[0]
        require(token in TOKENS, f"Token desconhecido {token:02x} em {start}")
        if token in (0x16, 0x30, 0x53):
            require(token == allowed_terminal, f"Terminador fora de contexto em {start}")
        if token == 0x53:
            require(self.depth == 0 and self.storage == len(self.script),
                    "EndOfScript deve ser o último token ao nível superior")
        node: dict[str, Any] = {
            "storage_offset": start, "memory_offset": memory_start,
            "token": f"0x{token:02x}", "instruction": TOKENS[token]
        }
        self.depth += 1
        try:
            self.operands(token, node)
        finally:
            self.depth -= 1
        node.update(storage_end=self.storage, memory_end=self.memory)
        self.instructions.append(node)
        return node

    def operands(self, token: int, node: dict[str, Any]) -> None:
        if token in (0x00, 0x01, 0x20):
            node["reference"] = self.pointer("object" if token == 0x20 else "property")
        elif token in (0x1C, 0x68):
            node["function"] = self.pointer("function")
            node["arguments"] = self.arguments()
        elif token == 0x1B:
            index, number = struct.unpack("<ii", self.take(8, 12))
            require(0 <= index < len(self.metadata["names"]), "FName inválido")
            require(number == 0, "FName numerado não suportado neste perfil limitado")
            node["function_name"] = self.metadata["names"][index]
            node["fname"] = {"index": index, "number": number}
            node["arguments"] = self.arguments()
        elif token == 0x19:
            node["object"] = self.expression()
            node["skip_memory_bytes"] = self.integer()
            node["rvalue_property"] = self.pointer("property", allow_null=True)
            node["expression"] = self.expression()
        elif token in (0x0F, 0x14):
            if token == 0x0F:
                node["property"] = self.pointer("property")
            node["target"] = self.expression()
            node["value"] = self.expression()
        elif token == 0x63:
            node["signature"] = self.pointer("function")
            node["delegate"] = self.expression()
            node["arguments"] = self.arguments()
        elif token == 0x64:
            node["property"] = self.pointer("property")
            node["value"] = self.expression()
        elif token in (0x04, 0x4E):
            node["expression"] = self.expression()
        elif token in (0x06, 0x07, 0x4C):
            node["target_memory_offset"] = self.integer()
            if token == 0x07:
                node["condition"] = self.expression()
        elif token == 0x1D:
            node["value"] = self.integer()
        elif token == 0x1E:
            node["value"] = struct.unpack("<f", self.take(4))[0]
        elif token == 0x24:
            node["value"] = self.take(1)[0]
        elif token == 0x1F:
            value = bytearray()
            while True:
                character = self.take(1)[0]
                if character == 0:
                    break
                value.append(character)
            node["value"] = value.decode("utf-8")
        elif token == 0x2F:
            node["struct"] = self.pointer("struct")
            node["struct_memory_size"] = self.integer()
            values = []
            while self.storage < len(self.script) and self.script[self.storage] != 0x30:
                values.append(self.expression())
            require(self.storage < len(self.script) and self.script[self.storage] == 0x30,
                    "Struct sem terminador")
            self.expression(allowed_terminal=0x30)
            node["values"] = values
        elif token == 0x42:
            node["member"] = self.pointer("property")
            node["struct_expression"] = self.expression()
        elif token == 0x69:
            count = struct.unpack("<H", self.take(2))[0]
            node["end_memory_offset"] = self.integer()
            node["selector"] = self.expression()
            cases = []
            for _ in range(count):
                cases.append({
                    "match": self.expression(),
                    "next_memory_offset": self.integer(),
                    "result": self.expression(),
                })
            node["cases"] = cases
            node["default"] = self.expression()
        elif token in (0x25, 0x27, 0x28):
            node["value"] = {0x25: 0, 0x27: True, 0x28: False}[token]

    def decode(self) -> list[dict[str, Any]]:
        require(bool(self.script), "Script vazio")
        ended = False
        while self.storage < len(self.script):
            self.top_level.append(self.storage)
            terminal = 0x53 if self.script[self.storage] == 0x53 else None
            node = self.expression(allowed_terminal=terminal)
            ended = node["instruction"] == "EX_EndOfScript"
        require(ended, "Script sem token EndOfScript final descodificado")
        self.instructions.sort(key=lambda item: item["storage_offset"])
        return self.instructions


def check_targets(decoder: Decoder) -> list[dict[str, Any]]:
    """Valida saltos, selecções e spans de contexto no perfil em memória."""
    boundaries = {node["memory_offset"] for node in decoder.instructions}
    top_boundaries = {
        node["memory_offset"] for node in decoder.instructions
        if node["storage_offset"] in decoder.top_level
    }
    checks = []
    for node in decoder.instructions:
        targets = []
        if "target_memory_offset" in node:
            targets.append(("jump", node["target_memory_offset"], top_boundaries))
        if "end_memory_offset" in node:
            require(node["end_memory_offset"] == node["default"]["memory_end"]
                    == node["memory_end"], "Fim da selecção divergente do fim do ramo default")
            targets.append(("switch_end", node["end_memory_offset"], boundaries | {decoder.memory}))
            for case in node["cases"]:
                require(case["next_memory_offset"] == case["result"]["memory_end"],
                        "Alvo do caso divergente do fim do resultado")
                targets.append(("switch_next", case["next_memory_offset"], boundaries))
        if "skip_memory_bytes" in node:
            expression = node["expression"]
            expected = expression["memory_end"] - expression["memory_offset"]
            require(node["skip_memory_bytes"] == expected, "Span de contexto divergente")
        for kind, target, allowed in targets:
            valid = target in allowed
            checks.append({"kind": kind, "source_storage_offset": node["storage_offset"],
                           "target_memory_offset": target, "instruction_boundary": valid})
            require(valid, f"Alvo {target} fora de um limite de instrução")
    return checks


def load_inspector() -> Any:
    """Importa o parser existente sem o executar como programa."""
    path = ROOT / "tools/inspect_fantasynth_assets.py"
    spec = importlib.util.spec_from_file_location("asset_inspector", path)
    require(spec is not None and spec.loader is not None, "Parser indisponível")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def decode_asset(asset: dict[str, Any], entry: dict[str, Any], payload: bytes,
                 inspector: Any) -> dict[str, Any]:
    digest = hashlib.sha1(payload).hexdigest()
    require(digest == entry["payload_sha1"] == asset["asset_sha1"], "SHA1 divergente")
    metadata = inspector.inspect(payload)
    name = Path(asset["path"]).stem
    results = []
    for function in asset["functions"]:
        if function["function"] not in FUNCTIONS[name]:
            continue
        start, length = function["asset_script_offset"], function["storage_size"]
        export = metadata["exports"][function["export_index"] - 1]
        require(export["name"] == function["function"], "Export de função divergente")
        require(start == export["tagged_properties_end"] + 24,
                "Cabeçalho nativo fora do perfil seleccionado de 24 bytes")
        memory_size, storage_size = struct.unpack_from("<ii", payload, start - 8)
        require(memory_size == function["memory_size"] and storage_size == length,
                "Tamanhos do cabeçalho nativo divergentes")
        require(start + length + 12 == export["serial_offset"] + export["serial_size"],
                "Fim do script e cauda de 12 bytes fora dos limites do export")
        script = payload[start:start + length]
        require(script == bytes.fromhex(function["script_hex"]), "Script diverge do JSON")
        sha256 = hashlib.sha256(script).hexdigest()
        require(sha256 == function["script_sha256"], "SHA256 do script divergente")
        require(function["pak_script_offset"] == entry["offset"] + entry["header_size"] + start,
                "Offset no PAK divergente")
        decoder = Decoder(script, metadata)
        instructions = decoder.decode()
        require(decoder.storage == length, "Tamanho serializado divergente")
        require(decoder.memory == function["memory_size"], "Tamanho em memória divergente")
        results.append({
            "function": function["function"], "export_index": function["export_index"],
            "asset_script_offset": start, "pak_script_offset": function["pak_script_offset"],
            "script_sha256": sha256, "storage_size": decoder.storage,
            "memory_size": decoder.memory, "expected_storage_size": length,
            "expected_memory_size": function["memory_size"], "sizes_match": True,
            "native_header_bounds_match": True,
            "top_level_storage_offsets": decoder.top_level,
            "target_checks": check_targets(decoder), "instructions": instructions,
        })
    require({f["function"] for f in results} == FUNCTIONS[name], "Funções em falta")
    return {"source": SOURCE_LABEL, "asset_path": asset["path"], "asset_sha1": digest,
            "hash_verified": True, "functions": results}


def check_entrypoints(result: dict[str, Any]) -> list[dict[str, Any]]:
    """Valida as chamadas dos eventos para o grafo principal seleccionado."""
    functions = {f["function"]: f for f in result["functions"]}
    graph = next(f for name, f in functions.items() if name.startswith("ExecuteUbergraph_"))
    boundaries = {n["memory_offset"] for n in graph["instructions"]
                  if n["storage_offset"] in graph["top_level_storage_offsets"]}
    checks = []
    for function in result["functions"]:
        for node in function["instructions"]:
            if node.get("function_name") != graph["function"]:
                continue
            args = node["arguments"]
            require(len(args) == 1 and args[0]["instruction"] == "EX_IntConst",
                    "Ponto de entrada não constante")
            target = args[0]["value"]
            require(target in boundaries, "Ponto de entrada fora dos limites")
            checks.append({"caller": function["function"],
                           "call_storage_offset": node["storage_offset"],
                           "target_memory_offset": target, "instruction_boundary": True})
    return checks


def write_json(path: Path, value: dict[str, Any]) -> None:
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def check_rejections() -> list[str]:
    """Verifica que entradas inválidas não passam silenciosamente."""
    metadata = {
        "imports": [{"name": "NotFunction", "class": "Class"},
                    {"name": "Function", "class": "Class"},
                    {"name": "FloatProperty", "class": "Class"}],
        "exports": [{"name": "Value", "class_index": -3},
                    {"name": "FunctionExport", "class_index": -2}],
        "names": ["Call"],
    }
    cases = [
        ("unknown_token", b"\xff\x53"),
        ("truncated_operand", b"\x1d\x00\x53"),
        ("interior_jump_target", b"\x06\x02\x00\x00\x00\x53"),
        ("empty_script", b""), ("missing_end_script", b"\x04\x0b"),
        ("early_end_script", b"\x53\x25\x53"),
        ("trailing_bytes_after_end", b"\x53\x25"),
        ("fake_end_in_int_operand", bytes.fromhex("1d00000053")),
        ("fake_end_in_return_operand", bytes.fromhex("041d00000053")),
        ("fake_end_in_byte_operand", bytes.fromhex("2453")),
        ("loose_argument_terminator", b"\x16\x53"),
        ("loose_struct_terminator", b"\x30\x53"),
        ("nested_end_script", b"\x04\x53"),
        ("null_variable", b"\x00" + struct.pack("<i", 0) + b"\x53"),
        ("function_used_as_variable", b"\x00" + struct.pack("<i", 2) + b"\x53"),
        ("null_function", b"\x1c" + struct.pack("<i", 0) + b"\x16\x53"),
        ("class_used_as_function", b"\x1c" + struct.pack("<i", -1) + b"\x16\x53"),
        ("negative_fname_number", b"\x1b" + struct.pack("<ii", 0, -1) + b"\x16\x53"),
        ("numbered_fname_outside_profile", b"\x1b" + struct.pack("<ii", 0, 1) + b"\x16\x53"),
        ("switch_end_not_default_end", b"\x69" + struct.pack("<HI", 1, 0)
         + b"\x24\x00\x24\x00" + struct.pack("<I", 16) + b"\x27\x28\x53"),
        ("switch_next_not_case_end", b"\x69" + struct.pack("<HI", 1, 17)
         + b"\x24\x00\x24\x00" + struct.pack("<I", 0) + b"\x27\x28\x53"),
    ]
    passed = []
    for label, script in cases:
        try:
            decoder = Decoder(script, metadata)
            decoder.decode()
            check_targets(decoder)
        except ValueError:
            passed.append(label)
        else:
            raise ValueError(f"Entrada inválida aceite: {label}")
    return passed


def main() -> None:
    """Verifica todos os resultados antes de escrever os artefactos finais."""
    sys.dont_write_bytecode = True
    inventory_path = ROOT / "plan/fantasynth-pak-inventory.json"
    regions_path = ROOT / "plan/rea-investigation/blueprint-script-regions.json"
    inventory = json.loads(inventory_path.read_text(encoding="utf-8"))
    regions = json.loads(regions_path.read_text(encoding="utf-8"))
    negative_checks = check_rejections()
    inspector = load_inspector()
    decoded = []
    with Path(inventory["source"]).open("rb") as pak:
        for asset in regions["assets"]:
            if Path(asset["path"]).stem not in FUNCTIONS:
                continue
            entry = next(e for e in inventory["entries"] if e["path"] == asset["path"])
            require(not entry["compression_method"] and not entry["encrypted"],
                    "Recurso comprimido ou cifrado não suportado")
            pak.seek(entry["offset"] + entry["header_size"])
            result = decode_asset(asset, entry, pak.read(entry["stored_size"]), inspector)
            result["entrypoint_checks"] = check_entrypoints(result)
            decoded.append(result)
    require(len(decoded) == 2, "Recursos seleccionados em falta")
    report = {
        "source": SOURCE_LABEL, "status": "passed", "pak_path": inventory["source"],
        "inventory_sha256": hashlib.sha256(inventory_path.read_bytes()).hexdigest(),
        "regions_sha256": hashlib.sha256(regions_path.read_bytes()).hexdigest(),
        "decoder_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "negative_checks_passed": negative_checks,
        "limits": ["Perfil limitado aos scripts seleccionados, não um parser UE universal.",
                   "Offsets de memória usam ponteiros de 8 e FNames de 12 bytes.",
                   "FNames numerados são rejeitados; os scripts seleccionados usam number=0.",
                   "Validação estrutural, não prova de execução ou subscritores activos.",
                   "Sem invocação REA, análise nativa ou REA Evidence."],
        "references": [
            "https://github.com/atenfyr/UAssetAPI/tree/master/UAssetAPI/Kismet/Bytecode/Expressions",
            "https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/EExprToken"
        ],
        "assets": [],
    }
    for result in decoded:
        filename = Path(result["asset_path"]).stem + ".instructions.json"
        write_json(DIRECTORY / filename, result)
        report["assets"].append({
            "asset_path": result["asset_path"], "asset_sha1": result["asset_sha1"],
            "artifact": filename, "entrypoint_checks": result["entrypoint_checks"],
            "functions": [{key: f[key] for key in (
                "function", "script_sha256", "storage_size", "memory_size", "sizes_match",
                "native_header_bounds_match",
                "asset_script_offset", "pak_script_offset", "target_checks")}
                for f in result["functions"]],
        })
    write_json(DIRECTORY / "verification-report.json", report)
    print(json.dumps({"status": "passed", "assets": len(decoded),
                      "functions": sum(len(a["functions"]) for a in decoded),
                      "output_directory": str(DIRECTORY)}))


if __name__ == "__main__":
    main()
