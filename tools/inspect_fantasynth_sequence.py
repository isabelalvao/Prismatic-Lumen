"""Descodificação estática adicional de propriedades da sequência UE4 observada."""

import argparse
import hashlib
import json
from pathlib import Path
import struct

from inspect_fantasynth_assets import Reader, inspect, properties


TAGGED_STRUCTS = {
    "MovieScenePossessable", "MovieSceneBinding", "FloatRange",
    "FloatRangeBound", "SpawnerSourceParam", "BeatAction", "ComposerAction",
    "ScatterSourceParam", "ScatterScale", "ScatterRotation", "ScatterActor",
    "RichCurve", "NameCurve", "HelloComposition", "HelloCompositionParam",
    "RandomStream", "Parameter",
    "HelloComposerAction", "ScatterParameter", "DataTableRowHandle",
    "ScatterRandomSeeds", "TransformCombineMode",
}


def payload_reader(data, prop, names):
    """Relê apenas o cabeçalho validado para delimitar o conteúdo."""
    r = Reader(data, prop["offset"])
    if r.name(names) != prop["name"] or r.name(names) != prop["type"]:
        raise ValueError("Cabeçalho da propriedade diverge")
    size, index = r.i32(), r.i32()
    kind = prop["type"]
    if kind == "StructProperty":
        r.name(names)
        r.take(16)
    elif kind == "BoolProperty":
        r.take(1)
    elif kind in ("ByteProperty", "EnumProperty", "ArrayProperty"):
        r.name(names)
    if r.take(1)[0]:
        r.take(16)
    if size != prop["size"] or index != prop["array_index"]:
        raise ValueError("Metadados da propriedade divergem")
    if size < 0 or r.pos + size > len(data):
        raise ValueError("Conteúdo da propriedade fora dos limites")
    r.limit = r.pos + size
    return r


def tagged(r, names, depth):
    result = properties(r, names, r.limit)
    enrich(result, r.data, names, depth + 1)
    return result


def enrich(props, data, names, depth=0):
    if depth > 16:
        raise ValueError("Recursão excessiva")
    for p in props:
        if "properties" in p:
            enrich(p["properties"], data, names, depth + 1)
        if "values" in p:
            for values in p["values"]:
                enrich(values, data, names, depth + 1)
        r = payload_reader(data, p, names)
        kind = p["type"]
        try:
            if kind == "StrProperty":
                p["value"] = r.string()
            elif kind == "ByteProperty" and p["size"] == 1:
                p["value"] = r.take(1)[0]
            elif kind == "StructProperty" and p["struct"] == "Guid":
                if p["size"] != 16:
                    raise ValueError("Guid com tamanho inválido")
                p["guid_bytes"] = r.take(16).hex()
            elif kind == "StructProperty" and p["struct"] == "RandomStream" and p["size"] == 8:
                p["initial_seed"], p["seed"] = r.i32(), r.i32()
            elif kind == "StructProperty" and p["struct"] in TAGGED_STRUCTS:
                if "properties" not in p:
                    p["properties"] = tagged(r, names, depth)
                else:
                    continue
            elif kind == "StructProperty" and p["struct"] == "LevelSequenceObjectReferenceMap":
                count = r.i32()
                if not 0 <= count <= 10000:
                    raise ValueError("Mapa de referências demasiado extenso")
                p["references"] = [
                    {"binding_guid_bytes": r.take(16).hex(),
                     "object_guid_bytes": r.take(16).hex(), "object_path": r.string()}
                    for _ in range(count)
                ]
            elif kind == "ArrayProperty" and p.get("inner") != "ObjectProperty":
                count = r.i32()
                if not 0 <= count <= 10000:
                    raise ValueError("Array demasiado extenso")
                inner = p["inner"]
                if inner == "StructProperty":
                    r.name(names)
                    if r.name(names) != "StructProperty":
                        raise ValueError("Cabeçalho de elemento inválido")
                    element_size, element_index = r.i32(), r.i32()
                    element_struct = r.name(names)
                    r.take(16)
                    if r.take(1)[0]:
                        r.take(16)
                    if element_size != r.limit - r.pos or element_index != 0:
                        raise ValueError("Tamanho de elementos incoerente")
                    if element_struct in TAGGED_STRUCTS:
                        p["values"] = [tagged(r, names, depth) for _ in range(count)]
                    elif element_struct == "NameCurveKey":
                        if r.limit - r.pos != count * 12:
                            raise ValueError("NameCurveKey com tamanho inválido")
                        p["keys"] = [
                            {"offset": r.pos, "time": struct.unpack("<f", r.take(4))[0],
                             "value": r.name(names)} for _ in range(count)
                        ]
                    else:
                        continue
                elif inner in ("NameProperty", "StrProperty", "FloatProperty", "IntProperty", "AssetObjectProperty"):
                    readers = {"NameProperty": lambda: r.name(names),
                               "StrProperty": r.string, "AssetObjectProperty": r.string, "IntProperty": r.i32,
                               "FloatProperty": lambda: struct.unpack("<f", r.take(4))[0]}
                    p["values"] = [readers[inner]() for _ in range(count)]
                elif inner == "TextProperty":
                    texts = []
                    for _ in range(count):
                        flags = r.i32()
                        history = r.take(1)[0]
                        if history != 0:
                            raise ValueError("História FText não suportada")
                        texts.append({"flags": flags, "namespace": r.string(),
                                      "key": r.string(), "source": r.string()})
                    p["values"] = texts
                else:
                    continue
            else:
                continue
            if r.pos != r.limit:
                raise ValueError("Conteúdo descodificado não consumido exactamente")
        except (ValueError, UnicodeError, struct.error) as exc:
            p["additional_parse_error"] = str(exc)


def reference(asset, index, visited=None):
    if index == 0:
        return "None"
    visited = set() if visited is None else visited
    if index in visited:
        raise ValueError("Ciclo de referências")
    visited.add(index)
    table = asset["exports"] if index > 0 else asset["imports"]
    position = index - 1 if index > 0 else -index - 1
    if not 0 <= position < len(table):
        raise ValueError("Referência fora dos limites")
    entry = table[position]
    parent = entry["outer_index"]
    return (reference(asset, parent, visited) + "." if parent else "") + entry["name"]


def resolve(props, asset):
    for p in props:
        if "object_index" in p:
            p["object_reference"] = reference(asset, p["object_index"])
        if "object_indices" in p:
            p["object_references"] = [reference(asset, i) for i in p["object_indices"]]
        if "properties" in p:
            resolve(p["properties"], asset)
        if "values" in p and p.get("inner") == "StructProperty":
            for values in p["values"]:
                resolve(values, asset)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--inventory", default="plan/fantasynth-pak-inventory.json")
    parser.add_argument("--output", required=True)
    parser.add_argument("--match", action="append", required=True)
    args = parser.parse_args()
    inventory = json.loads(Path(args.inventory).read_text(encoding="utf-8"))
    entries = [e for e in inventory["entries"] if any(e["path"].endswith(m) for m in args.match)]
    if not entries:
        raise ValueError("Nenhum recurso seleccionado")
    assets = []
    with open(inventory["source"], "rb") as pak:
        for entry in entries:
            if entry["compression_method"] or entry["encrypted"]:
                raise ValueError("Recurso comprimido ou cifrado não suportado")
            pak.seek(entry["offset"] + entry["header_size"])
            data = pak.read(entry["stored_size"])
            if hashlib.sha1(data).hexdigest() != entry["payload_sha1"]:
                raise ValueError("SHA1 diverge do índice")
            asset = inspect(data)
            asset.update({"path": entry["path"], "payload_sha1": entry["payload_sha1"],
                          "payload_hash_verified": True,
                          "pak_payload_offset": entry["offset"] + entry["header_size"]})
            for index, export in enumerate(asset["exports"], 1):
                export["object_reference"] = reference(asset, index)
                export["class_reference"] = reference(asset, export["class_index"])
                enrich(export.get("properties", []), data, asset["names"])
                resolve(export.get("properties", []), asset)
                if export["class_reference"] == "/Script/Engine.UserDefinedEnum":
                    end = export["serial_offset"] + export["serial_size"]
                    reader = Reader(data, export["tagged_properties_end"], end)
                    try:
                        if reader.i32() != 0:
                            raise ValueError("GUID de objecto nativo não suportado")
                        cpp_type, count = reader.string(), reader.i32()
                        if not 0 <= count <= 10000:
                            raise ValueError("Enum demasiado extenso")
                        # Este perfil cooked guarda valores de enum em um byte.
                        if reader.limit - reader.pos != count * 9 + 1:
                            raise ValueError("Layout nativo de enum não suportado")
                        entries_enum = [{"name": reader.name(asset["names"]),
                                         "value": reader.take(1)[0]}
                                        for _ in range(count)]
                        cpp_form = reader.take(1)[0]
                        if reader.pos != end:
                            raise ValueError("Enum não consumido exactamente")
                        export["enum_entries"] = entries_enum
                        export["cpp_type"], export["cpp_form"] = cpp_type, cpp_form
                    except (ValueError, UnicodeError, struct.error) as exc:
                        export["enum_parse_error"] = str(exc)
                if export["class_reference"] == "/Script/Engine.DataTable":
                    end = export["serial_offset"] + export["serial_size"]
                    reader = Reader(data, export["tagged_properties_end"], end)
                    try:
                        if reader.i32() != 0:
                            raise ValueError("GUID de objecto nativo não suportado")
                        count = reader.i32()
                        if not 0 <= count <= 10000:
                            raise ValueError("DataTable demasiado extensa")
                        rows = []
                        for _ in range(count):
                            row_name = reader.name(asset["names"])
                            row_props = tagged(reader, asset["names"], 0)
                            resolve(row_props, asset)
                            rows.append({"row_name": row_name, "properties": row_props})
                        if reader.pos != end:
                            raise ValueError("DataTable não consumida exactamente")
                        export["datatable_rows"] = rows
                    except (ValueError, UnicodeError, struct.error) as exc:
                        export["datatable_parse_error"] = str(exc)
            assets.append(asset)
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps({"source": inventory["source"],
                                 "scope": "Static tagged properties and limited native structures; no execution",
                                 "assets": assets}, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({"output": str(output), "assets": len(assets)}))


if __name__ == "__main__":
    main()
