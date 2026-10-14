"""Inspecção offline limitada de pacotes UE4 cooked, sem extrair recursos."""

import argparse
import hashlib
import json
from pathlib import Path
import struct


class Reader:
    def __init__(self, data, pos=0, limit=None):
        self.data, self.pos = data, pos
        self.limit = len(data) if limit is None else min(limit, len(data))

    def take(self, count):
        if count < 0 or self.pos < 0 or self.pos + count > self.limit:
            raise ValueError("Leitura fora dos limites")
        result = self.data[self.pos:self.pos + count]
        self.pos += count
        return result

    def i32(self):
        return struct.unpack("<i", self.take(4))[0]

    def string(self):
        length = self.i32()
        if abs(length) > 100000:
            raise ValueError("FString demasiado extensa")
        return self.take(length)[:-1].decode("utf-8") if length >= 0 else self.take(-length * 2)[:-2].decode("utf-16-le")

    def name(self, names):
        index, number = self.i32(), self.i32()
        if not 0 <= index < len(names):
            raise ValueError(f"Índice FName inválido: {index}")
        return names[index] + (f"_{number - 1}" if number else "")


def properties(reader, names, limit=None):
    previous_limit = reader.limit
    if limit is not None:
        reader.limit = min(limit, previous_limit)
    try:
        return _properties(reader, names)
    finally:
        reader.limit = previous_limit


def _properties(reader, names):
    result = []
    while reader.pos < reader.limit:
        offset = reader.pos
        name = reader.name(names)
        if name == "None":
            return result
        kind = reader.name(names)
        size, index = reader.i32(), reader.i32()
        item = {"offset": offset, "name": name, "type": kind, "size": size, "array_index": index}
        if kind == "StructProperty":
            item["struct"] = reader.name(names)
            reader.take(16)
        elif kind == "BoolProperty":
            item["value"] = bool(reader.take(1)[0])
        elif kind in ("ByteProperty", "EnumProperty"):
            item["enum"] = reader.name(names)
        elif kind == "ArrayProperty":
            item["inner"] = reader.name(names)
        elif kind in ("SetProperty", "MapProperty"):
            raise ValueError("Tipo de propriedade não suportado")
        has_guid = reader.take(1)[0]
        if has_guid:
            reader.take(16)
        start, end = reader.pos, reader.pos + size
        if end > reader.limit or size < 0:
            raise ValueError("Tamanho de propriedade inválido")
        expected = {"FloatProperty": 4, "IntProperty": 4, "NameProperty": 8, "ObjectProperty": 4, "EnumProperty": 8, "BoolProperty": 0}
        if kind in expected and size != expected[kind]:
            raise ValueError(f"Tamanho inválido de {kind}")
        struct_size = {"LinearColor": 16, "Vector": 12, "Rotator": 12}
        if kind == "StructProperty" and item["struct"] in struct_size and size != struct_size[item["struct"]]:
            raise ValueError("Tamanho de struct inválido")
        payload = Reader(reader.data, start, end)
        value = payload.data[start:end]
        if kind == "FloatProperty" and size == 4:
            item["value"] = struct.unpack("<f", value)[0]
        elif kind == "IntProperty" and size == 4:
            item["value"] = struct.unpack("<i", value)[0]
        elif kind == "NameProperty":
            item["value"] = payload.name(names)
        elif kind in ("ByteProperty", "EnumProperty") and size == 8:
            item["value"] = payload.name(names)
        elif kind == "ObjectProperty" and size == 4:
            item["object_index"] = struct.unpack("<i", value)[0]
        elif kind == "StructProperty" and item["struct"] == "LinearColor" and size == 16:
            item["value_rgba"] = list(struct.unpack("<4f", value))
        elif kind == "StructProperty" and item["struct"] in ("Vector", "Rotator") and size == 12:
            item["value_xyz"] = list(struct.unpack("<3f", value))
        elif kind == "ArrayProperty" and item["inner"] == "StructProperty":
            count = payload.i32()
            item["count"] = count
            # Os pacotes seleccionados usam um cabeçalho de elemento comum.
            element_name = payload.name(names)
            element_kind = payload.name(names)
            element_size, element_index = payload.i32(), payload.i32()
            element_struct = payload.name(names)
            payload.take(16)
            if payload.take(1)[0]:
                payload.take(16)
            item["element"] = {"name": element_name, "type": element_kind, "struct": element_struct, "size": element_size, "index": element_index}
            if not 0 <= count <= 10000:
                raise ValueError("Array demasiado extenso")
            if element_struct in ("ScalarParameterValue", "VectorParameterValue", "TextureParameterValue"):
                item["values"] = [properties(payload, names, end) for _ in range(count)]
            elif element_struct == "RichCurveKey" and end - payload.pos == count * 27:
                item["keys"] = []
                for _ in range(count):
                    key_offset = payload.pos
                    modes = list(payload.take(3))
                    time, value, arrive, arrive_weight, leave, leave_weight = struct.unpack("<6f", payload.take(24))
                    item["keys"].append({"offset": key_offset, "mode_bytes": modes, "time": time, "value": value, "arrive_tangent": arrive, "arrive_weight": arrive_weight, "leave_tangent": leave, "leave_weight": leave_weight})
            else:
                item["raw_hex_prefix"] = payload.data[payload.pos:min(end, payload.pos + 64)].hex()
        elif kind == "ArrayProperty" and item["inner"] == "ObjectProperty":
            count = payload.i32()
            if not 0 <= count <= 10000 or size != 4 + count * 4:
                raise ValueError("Tamanho de array de referências inválido")
            item["object_indices"] = [payload.i32() for _ in range(count)]
        elif kind == "StructProperty" and item["struct"] in ("BoxSphereBounds", "RichCurve", "ScatterScale", "ScatterRotation"):
            item["properties"] = properties(payload, names, end)
        elif kind == "StructProperty" and item["struct"] not in ("Guid",):
            item["raw_hex_prefix"] = value[:64].hex()
        reader.pos = end
        result.append(item)
    return result


def inspect(data):
    r = Reader(data)
    magic, legacy = r.i32(), r.i32()
    if magic != -1641380927 or legacy != -6:
        raise ValueError("Perfil de pacote não suportado (esperado UE4 legacy -6)")
    ue3, ue4, licensee, custom_count = [r.i32() for _ in range(4)]
    if custom_count:
        raise ValueError("Custom versions não suportadas neste inspector limitado")
    header_size, folder = r.i32(), r.string()
    flags, name_count, name_offset, text_count, text_offset, export_count, export_offset, import_count, import_offset, depends_offset = [r.i32() for _ in range(10)]
    r.pos = name_offset
    names = []
    for _ in range(name_count):
        names.append(r.string())
        r.take(4)
    if r.pos != import_offset:
        raise ValueError("O fim da tabela de nomes não coincide com a tabela de imports")
    imports = []
    for _ in range(import_count):
        imports.append({"class_package": r.name(names), "class": r.name(names), "outer_index": r.i32(), "name": r.name(names)})
    if r.pos != export_offset:
        raise ValueError("O fim de imports não coincide com exports")
    exports = []
    r.pos = export_offset
    for _ in range(export_count):
        start = r.pos
        entry = {"class_index": r.i32(), "super_index": r.i32(), "outer_index": r.i32(), "name": r.name(names), "flags": r.i32(), "serial_size": r.i32(), "serial_offset": r.i32()}
        r.pos = start + 72
        offset, size = entry["serial_offset"], entry["serial_size"]
        if offset < header_size or offset + size > len(data):
            raise ValueError("Export fora dos limites do pacote")
        payload = Reader(data, offset)
        try:
            entry["properties"] = properties(payload, names, offset + size)
            entry["tagged_properties_end"] = payload.pos
            entry["remaining_serial_bytes"] = offset + size - payload.pos
        except (ValueError, UnicodeError, struct.error) as exc:
            entry["property_parse_error"] = str(exc)
        exports.append(entry)
    return {"profile": "limited cooked UE4 legacy -6 / 72-byte exports", "legacy": legacy, "ue4_version": ue4, "licensee_version": licensee, "custom_count": custom_count, "header_size": header_size, "name_offset": name_offset, "import_offset": import_offset, "export_offset": export_offset, "names": names, "imports": imports, "exports": exports}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--inventory", default="plan/fantasynth-pak-inventory.json")
    parser.add_argument("--output", default="plan/fidelity-assets/selected-assets.json")
    parser.add_argument("--match", action="append", required=True)
    args = parser.parse_args()
    inventory = json.loads(Path(args.inventory).read_text(encoding="utf-8"))
    selected = [e for e in inventory["entries"] if any(e["path"].endswith(m) for m in args.match)]
    results = []
    with open(inventory["source"], "rb") as pak:
        for entry in selected:
            if entry["compression_method"] or entry["encrypted"]:
                raise ValueError("Recursos comprimidos ou cifrados não suportados")
            pak.seek(entry["offset"] + entry["header_size"])
            data = pak.read(entry["stored_size"])
            digest = hashlib.sha1(data).hexdigest()
            if digest != entry["payload_sha1"]:
                raise ValueError("SHA1 do recurso diverge do índice validado")
            result = {"path": entry["path"], "payload_sha1": digest, "payload_hash_verified": True, "raw_size": len(data), "pak_payload_offset": entry["offset"] + entry["header_size"]}
            try:
                result.update(inspect(data))
            except (ValueError, UnicodeError, struct.error) as exc:
                result["parse_error"] = str(exc)
            results.append(result)
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps({"source": inventory["source"], "scope": "Static package metadata and tagged properties only; no runtime observation or shader/native payload decoding", "assets": results}, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({"output": str(output), "assets": len(results), "parse_errors": sum("parse_error" in r for r in results) + sum("property_parse_error" in e for r in results for e in r.get("exports", []))}))


if __name__ == "__main__":
    main()
