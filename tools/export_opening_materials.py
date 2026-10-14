"""Recupera referências estáticas da abertura, sem executar o jogo original."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import zlib

from inspect_fantasynth_assets import inspect
from inspect_fantasynth_sequence import reference, resolve


ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / "plan/opening-development/material-evidence"
OUTPUT = ROOT / "assets/opening/materials.json"
TEXTURES = ROOT / "assets/opening/textures"
CLASSES = {"Material", "MaterialInstanceConstant", "Texture2D",
           "TextureRenderTarget2D", "SubstanceTexture2D", "StaticMesh",
           "BlueprintGeneratedClass"}
SEED_NAMES = {"SpawnerIntroHex", "SpawnerIntroDot", "SpawnerIntroNeon",
              "SpawnerIntroLower", "SpawnerIntroPole", "SpawnerIntroPoleDot",
              "SpawnerPyramidGroundFar", "SpawnerPyramidGroundFront",
              "SpawnerPyramidTerrainFar", "SpawnerPyramidTerrainNear",
              "SpawnerPyramidSmall", "SpawnerForestFogSheet",
              "SpawnerForestRockLight", "SpawnerTubeIntro0", "SpawnerTubeIntro1",
              "SpawnerTubeIntro2", "SpawnerTubeIntro3", "SpawnerTubeIntro4"}


def read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, data: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n",
                    encoding="utf-8")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validate_png(path: Path) -> list[int]:
    """Valida todos os blocos PNG e descomprime os dados de imagem."""
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Assinatura PNG inválida: {path}")
    offset, compressed, dimensions = 8, bytearray(), None
    ended = False
    while offset + 12 <= len(data):
        size = struct.unpack_from(">I", data, offset)[0]
        end = offset + 12 + size
        if end > len(data):
            raise ValueError(f"Bloco PNG incompleto: {path}")
        kind = data[offset + 4:offset + 8]
        payload = data[offset + 8:offset + 8 + size]
        expected = struct.unpack_from(">I", data, offset + 8 + size)[0]
        if zlib.crc32(kind + payload) != expected:
            raise ValueError(f"CRC PNG divergente: {path}")
        if kind == b"IHDR":
            dimensions = list(struct.unpack_from(">II", payload))
        elif kind == b"IDAT":
            compressed.extend(payload)
        elif kind == b"IEND":
            ended = True
            break
        offset = end
    if not ended or dimensions is None or not all(dimensions) or not compressed:
        raise ValueError(f"Estrutura PNG incompleta: {path}")
    if not zlib.decompress(compressed):
        raise ValueError(f"Dados PNG vazios: {path}")
    return dimensions


def package_path(package: str) -> str:
    return "Fantasynth/Content/" + package.removeprefix("/Game/") + ".uasset"


def import_references(asset: dict) -> list[dict]:
    result = []
    for index, item in enumerate(asset["imports"], 1):
        if item["class"] in CLASSES:
            ref = reference(asset, -index)
            if ref.startswith("/Game/"):
                result.append({"class": item["class"], "reference": ref,
                               "package": ref.split(".")[0]})
    return result


def semantic_key(name: str) -> str:
    aliases = {"M_Hex_Solid": "intro_hex_solid", "M_Hex_Hole": "intro_hex_hole"}
    if name in aliases:
        return aliases[name]
    return re.sub(r"[^a-z0-9]+", "_", name.lower()).strip("_")


def material_record(asset: dict, export: dict) -> dict:
    props = {p["name"]: p for p in export.get("properties", [])}
    scalars, vectors, textures = {}, {}, []
    for prop_name, target, field in [("ScalarParameterValues", scalars, "value"),
                                      ("VectorParameterValues", vectors, "value_rgba")]:
        for group in props.get(prop_name, {}).get("values", []):
            entries = {p["name"]: p for p in group}
            parameter = entries.get("ParameterName", {}).get("value")
            value = entries.get("ParameterValue", {}).get(field)
            if parameter is not None and value is not None:
                target[parameter] = value
    refs = import_references(asset)
    by_ref = {r["reference"]: r for r in refs}
    for group in props.get("TextureParameterValues", {}).get("values", []):
        entries = {p["name"]: p for p in group}
        ref = entries.get("ParameterValue", {}).get("object_reference")
        textures.append({"parameter": entries.get("ParameterName", {}).get("value"),
                         "reference": ref, "source_package": by_ref.get(ref, {}).get("package"),
                         "class": by_ref.get(ref, {}).get("class"), "path": None,
                         "sha256": None})
    package = asset["package"]
    defaults = []
    for expression in asset["exports"]:
        if expression.get("outer_index") != asset["exports"].index(export) + 1:
            continue
        expression_props = {p["name"]: p for p in expression.get("properties", [])}
        parameter = expression_props.get("ParameterName", {}).get("value")
        if parameter is None:
            continue
        default = expression_props.get("DefaultValue", {})
        value = default.get("value", default.get("value_rgba"))
        texture_ref = expression_props.get("Texture", {}).get("object_reference")
        defaults.append({"parameter": parameter, "expression": expression["object_reference"],
                         "class": expression["class_reference"], "value": value,
                         "texture_reference": texture_ref,
                         "status": "tagged_property" if value is not None or texture_ref else
                                   "default_value_not_serialized_unknown"})
    return {"key": semantic_key(export["name"]), "name": export["name"],
            "source_package": package, "class": export["class_reference"],
            "parent": props.get("Parent", {}).get("object_reference"),
            "scalars": scalars, "vectors": vectors, "textures": textures,
            "expression_defaults": defaults,
            "references": refs,
            "provenance": {"payload_sha1": asset["payload_sha1"],
                           "payload_hash_verified": True,
                           "metadata": "plan/opening-development/material-evidence/metadata.json"},
            "unknowns": ["Original compiled shader behaviour is not reconstructed",
                         "Runtime parameter changes are not present in these static overrides"]}


def run_umodel(tool: Path, content: Path, package: str, mode: str,
               export_dir: Path, logs_dir: Path) -> dict:
    name = package.rsplit("/", 1)[1]
    args = [str(tool), mode, "-game=ue4.13", f"-path={content}",
            f"-out={export_dir}"]
    if mode == "-export":
        args.append("-png")
    args += [package, name]
    result = subprocess.run(args, capture_output=True, check=False, timeout=120)
    log = logs_dir / (package.removeprefix("/Game/").replace("/", "__")
                      + mode + ".log")
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_bytes(result.stdout + result.stderr)
    return {"args": args, "returncode": result.returncode,
            "log": str(log.relative_to(ROOT)).replace("\\", "/"),
            "log_sha256": digest(log)}


def main() -> None:
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    TEXTURES.mkdir(parents=True, exist_ok=True)
    inventory = read_json(ROOT / "plan/fantasynth-pak-inventory.json")
    entries = {e["path"]: e for e in inventory["entries"]}
    enriched = read_json(ROOT / "plan/full-reference/enriched-map-sequence.json")
    map_asset = next(a for a in enriched["assets"] if a["path"].endswith(".umap"))
    seed_edges = []
    for export in map_asset["exports"]:
        if export["name"] in SEED_NAMES:
            for prop in export.get("properties", []):
                if prop["name"] == "Sources":
                    for ref in prop.get("object_references", []):
                        seed_edges.append({"actor": export["object_reference"],
                                           "property": "Sources", "reference": ref,
                                           "package": ref.split(".")[0]})
    pending = sorted({s["package"] for s in seed_edges})
    # Inclui as superfícies e o chão usados pelo mapa, com origem explícita.
    for export in map_asset["exports"]:
        if export.get("object_reference", "").startswith("ChezNous.PersistentLevel.ForestTerrain."):
            for prop in export.get("properties", []):
                for ref in prop.get("object_references", []):
                    if ref.startswith("/Game/"):
                        seed_edges.append({"actor": export["object_reference"],
                                           "property": prop["name"], "reference": ref,
                                           "package": ref.split(".")[0]})
                        pending.append(ref.split(".")[0])
    assets, seen = [], set()
    with Path(inventory["source"]).open("rb") as pak:
        while pending:
            package = pending.pop(0)
            if package in seen:
                continue
            seen.add(package)
            entry = entries.get(package_path(package))
            if entry is None:
                continue
            if entry["compression_method"] or entry["encrypted"]:
                raise ValueError(f"Recurso comprimido/cifrado: {package}")
            pak.seek(entry["offset"] + entry["header_size"])
            data = pak.read(entry["stored_size"])
            if hashlib.sha1(data).hexdigest() != entry["payload_sha1"]:
                raise ValueError(f"SHA1 divergente: {package}")
            asset = inspect(data)
            asset.update(package=package, path=entry["path"],
                         payload_sha1=entry["payload_sha1"])
            for index, export in enumerate(asset["exports"], 1):
                export["object_reference"] = reference(asset, index)
                export["class_reference"] = reference(asset, export["class_index"])
                resolve(export.get("properties", []), asset)
            assets.append(asset)
            pending.extend(r["package"] for r in import_references(asset)
                           if r["package"] not in seen)
    write_json(EVIDENCE / "metadata.json", {"source": inventory["source"],
                                           "seed_edges": seed_edges, "assets": assets})
    materials, texture_assets, dynamic_targets, unsupported_textures = [], [], [], []
    for asset in assets:
        for export in asset["exports"]:
            class_name = export["class_reference"].rsplit(".", 1)[-1]
            if class_name in {"Material", "MaterialInstanceConstant"}:
                materials.append(material_record(asset, export))
            elif class_name == "Texture2D":
                texture_assets.append({"source_package": asset["package"],
                                       "name": export["name"],
                                       "payload_sha1": asset["payload_sha1"]})
            elif class_name == "TextureRenderTarget2D":
                dynamic_targets.append({"source_package": asset["package"],
                                        "properties": export.get("properties", []),
                                        "path": None,
                                        "status": "runtime_generated_not_exportable_as_static_texture"})
            elif class_name == "SubstanceTexture2D":
                unsupported_textures.append({"source_package": asset["package"],
                                             "payload_sha1": asset["payload_sha1"],
                                             "class": class_name, "path": None,
                                             "status": "unsupported_by_umodel"})
    tool = ROOT / "tools/vendor/ueviewer/umodel.exe"
    content = Path(inventory["source"]).parents[1]
    export_dir = EVIDENCE / "exports"
    calls = []
    # Remove apenas os PNG esperados: uma saída antiga não prova exportação nova.
    for texture in texture_assets:
        expected = export_dir / (texture["source_package"].removeprefix("/") + ".png")
        if not expected.resolve().is_relative_to(export_dir.resolve()):
            raise ValueError("Caminho de exportação fora da pasta autorizada")
        expected.unlink(missing_ok=True)
    for package in sorted({m["source_package"] for m in materials}
                          | {t["source_package"] for t in texture_assets}
                          | {t["source_package"] for t in unsupported_textures}):
        for mode in ("-dump", "-export"):
            call = run_umodel(tool, content, package, mode, export_dir, EVIDENCE / "logs")
            calls.append(call)
            if call["returncode"]:
                raise RuntimeError(f"UModel falhou; consultar {call['log']}")
    textures = []
    for texture in texture_assets:
        package = texture["source_package"]
        source = export_dir / (package.removeprefix("/") + ".png")
        relative = Path(package.removeprefix("/Game/") + ".png")
        destination = TEXTURES / relative
        record = dict(texture, path=None, sha256=None, dimensions=None)
        if not source.exists():
            raise RuntimeError(f"UModel não exportou o Texture2D esperado: {source}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, destination)
        record.update(path=str(destination.relative_to(ROOT)).replace("\\", "/"),
                      sha256=digest(destination),
                      dimensions=validate_png(destination))
        textures.append(record)
    by_package = {t["source_package"]: t for t in textures}
    for material in materials:
        for texture in material["textures"]:
            recovered = by_package.get(texture["source_package"], {})
            texture.update(path=recovered.get("path"), sha256=recovered.get("sha256"))
    write_json(EVIDENCE / "umodel-calls.json", {"tool_sha256": digest(tool), "calls": calls})
    semantic_materials = {
        "intro_hex_hollow": "m_hex_hole_1_map",
        "intro_hex_solid_1": "m_hex_solid_1_map",
        "intro_hex_solid_2": "m_hex_solid_2_map",
        "pyramid_ground": "m_pyramid_map",
        "pyramid_terrain": "m_pyramid_theme",
        **{f"tube_intro_{i}": f"m_tubeintro{i}_map" for i in range(5)},
        **{f"intro_dot_{i}": f"m_dot{i}_map" for i in range(1, 5)},
    }
    # Os aliases são ligações explícitas; não atribuem comportamento ao nome.
    known_keys = {m["key"] for m in materials}
    if not set(semantic_materials.values()).issubset(known_keys):
        raise ValueError("Alias sem material recuperado")
    mesh_dependencies = []
    for asset in assets:
        if any(e["class_reference"] == "/Script/Engine.StaticMesh" for e in asset["exports"]):
            mesh_dependencies.append({"mesh_package": asset["package"],
                                      "material_references": [r for r in import_references(asset)
                                                              if r["class"] in {"Material", "MaterialInstanceConstant"}],
                                      "scope": "package dependencies; not decoded native material slots"})
    write_json(EVIDENCE / "mesh-material-dependencies.json", {"meshes": mesh_dependencies})
    write_json(OUTPUT, {"schema_version": 1, "scope_seconds": [0, 120],
                       "scope": "Static dependency catalogue of opening spawners; dependency presence does not prove on-screen visibility at a given second",
                       "materials": materials, "textures": textures,
                       "semantic_materials": semantic_materials,
                       "unsupported_texture_assets": unsupported_textures,
                       "dynamic_render_targets": dynamic_targets,
                       "source_edges": seed_edges,
                       "limits": ["No native shader decoding or original game execution",
                                  "Render targets contain runtime compositions, not recoverable PNG patterns",
                                  "Static textures inherited from a parent may be overridden by material instances",
                                  "No synthetic images are labelled as original assets"]})
    provenance_files = [ROOT / "plan/fantasynth-pak-inventory.json",
                        ROOT / "plan/full-reference/enriched-map-sequence.json",
                        ROOT / "tools/inspect_fantasynth_assets.py",
                        ROOT / "tools/inspect_fantasynth_sequence.py",
                        Path(__file__).resolve(), tool, OUTPUT]
    provenance_files += sorted(TEXTURES.rglob("*.png"))
    provenance_files += [EVIDENCE / "metadata.json", EVIDENCE / "umodel-calls.json",
                         EVIDENCE / "mesh-material-dependencies.json"]
    write_json(EVIDENCE / "hashes.json", {"files": [
        {"path": str(path.relative_to(ROOT)).replace("\\", "/"),
         "size": path.stat().st_size, "sha256": digest(path)} for path in provenance_files]})
    print(json.dumps({"materials": len(materials), "textures": len(textures),
                      "png_exported": sum(t["path"] is not None for t in textures),
                      "dynamic_targets": len(dynamic_targets), "umodel_calls": len(calls)}))


if __name__ == "__main__":
    main()
