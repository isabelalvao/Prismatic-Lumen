"""Converte geometria e tabelas recuperadas para a cena inicial, sem alterar originais."""
import hashlib
import json
import math
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "plan/opening-development/source-data"
OUTPUT = ROOT / "assets/opening"


def read_json(path):
    return json.loads(path.read_text(encoding="utf-8"))


def convert_mesh(path):
    data = read_json(path)
    buffers = []
    for record in data["buffers"]:
        uri = record["uri"]
        if Path(uri).name != uri:
            raise ValueError("URI de buffer não local")
        raw = (path.parent / uri).read_bytes()
        if len(raw) != record["byteLength"]:
            raise ValueError("buffer truncado")
        buffers.append(raw)

    def accessor(index):
        a = data["accessors"][index]
        v = data["bufferViews"][a["bufferView"]]
        dimensions = {"SCALAR": 1, "VEC2": 2, "VEC3": 3}[a["type"]]
        fmt = {5123: "H", 5125: "I", 5126: "f"}[a["componentType"]]
        size = struct.calcsize("<"+fmt*dimensions)
        stride = v.get("byteStride", size)
        offset = v.get("byteOffset", 0)+a.get("byteOffset", 0)
        raw = buffers[v["buffer"]]
        end = offset+(a["count"]-1)*stride+size
        if not a["count"] or stride < size or end > len(raw) or end > v.get("byteOffset", 0)+v["byteLength"]:
            raise ValueError("accessor fora do buffer")
        return [struct.unpack_from("<"+fmt*dimensions, raw, offset+i*stride) for i in range(a["count"])]

    vertices, indices = [], []
    for mesh in data["meshes"]:
        for primitive in mesh["primitives"]:
            attr = primitive["attributes"]
            pos, normals, uv = (accessor(attr[k]) for k in ("POSITION", "NORMAL", "TEXCOORD_0"))
            if len(pos) != len(normals) or len(pos) != len(uv):
                raise ValueError("Contagens de atributos distintas")
            offset = len(vertices)
            for p, n, t in zip(pos, normals, uv):
                row = (*[x*100 for x in p], *n, *t)
                if not all(math.isfinite(x) for x in row):
                    raise ValueError("Vértice não finito")
                vertices.append(row)
            source_indices = accessor(primitive["indices"])
            if len(source_indices) % 3:
                raise ValueError("Índices não triangulares")
            for (index,) in source_indices:
                if index >= len(pos):
                    raise ValueError("índice fora da primitiva")
                indices.append(index+offset)
    if len(vertices) > 65535:
        raise ValueError("Malha excede índice uint16")
    result = bytearray(struct.pack("<8sII", b"FNTMESH1", len(vertices), len(indices)))
    for vertex in vertices:
        result.extend(struct.pack("<8f", *vertex))
    result.extend(struct.pack("<"+"H"*len(indices), *indices))
    return bytes(result)


def scatter_matrix(row, anchor):
    def swap(v):
        return [v[0], v[2], v[1]]
    scale = row["Scale"]
    x = swap([a*scale[0] for a in row["RotationX"]])
    y = swap([a*scale[2] for a in row["RotationZ"]])
    z = swap([a*scale[1] for a in row["RotationY"]])
    p = swap([a+b for a, b in zip(row["Location"], anchor)])
    matrix = [*x, 0.0, *y, 0.0, *z, 0.0, *p, 1.0]
    if not all(math.isfinite(v) for v in matrix):
        raise ValueError("Transformação não finita")
    return matrix


def material_id(name):
    if "Hex_Hole" in name:
        return 1
    if "Hex_Solid" in name:
        return 2
    if "Tube_Intro" in name:
        return 3+int(name.rsplit("_", 1)[1])
    if "RockLight" in name or "Dot" in name:
        return 8
    if "Tree" in name:
        return 9
    if "Pole" in name:
        return 11
    if "Pyramid_Ground" in name or "Pyramid_Terrain" in name or name in ("alpha32", "green32", "forest_content_green_fixed"):
        return 10
    if "Neon" in name:
        return 3
    return 0


def generate():
    bank = ROOT / "plan/visual-resource-bank"
    resources = read_json(bank/"manifest.json")["resources"]
    known = {x["qualified_package"]: dict(x, base=bank) for x in resources}
    for extra in read_json(DATA/"extra-meshes.json"):
        known[extra["qualified_package"]] = dict(extra, base=ROOT)
    packets = {p["source_class"]: p for p in read_json(DATA/"blueprint-source-packets.json")}
    scatters = {s["id"]: s for s in read_json(DATA/"selected-scatters.json")}
    groups, meshes, gaps = {}, {}, []
    table_counts = {}
    for label in ("Intro", "Forest"):
        table = read_json(DATA/("HelloScatterTable_"+label+".json"))
        if table["export_end"] != table["consumed_end"] or len(table["rows"]) != table["row_count"]:
            raise ValueError("Tabela não consumida integralmente")
        table_counts[label] = table["row_count"]
        for row in table["rows"]:
            scatter_id = row["row_name"].rsplit("_", 1)[0]
            s = scatters.get(scatter_id)
            if not s or not s.get("sources") or row["Index"] >= len(s["sources"]):
                gaps.append({"row": row["row_name"], "reason": "source/scatter não resolvido"})
                continue
            source = s["sources"][row["Index"]]
            packet = packets.get(source)
            if not packet:
                gaps.append({"row": row["row_name"], "reason": "Blueprint source ausente"})
                continue
            # Preview explicita a variante; componentes herdados podem guardar outra mesh.
            candidates = [m for m in packet["meshes"] if m["name"] == "StaticMeshComponent0"]
            if not candidates:
                candidates = [m for m in packet["meshes"] if "StaticMeshComponent" in m["class"] and "Hierarchical" not in m["class"]]
            if not candidates:
                candidates = [m for m in packet["meshes"] if "HierarchicalInstancedStaticMeshComponent" in m["class"]]
            if not candidates:
                gaps.append({"row": row["row_name"], "reason": "mesh source não resolvida"})
                continue
            component = candidates[-1]
            package = component["mesh_reference"].split(".", 1)[0]
            resource = known.get(package)
            if not resource:
                gaps.append({"row": row["row_name"], "reason": "geometria não recuperada", "package": package})
                continue
            rid = resource["resource_id"]
            meshes[rid] = resource
            transform = s.get("transform_component") or {}
            anchor = transform.get("RelativeLocation", [0, 0, 0])
            parent = transform.get("AttachParent", "")
            stage = "forest" if label == "Forest" else ("dome" if "IntroDome" in parent else "intro")
            params = s.get("source_params") or []
            cull = 16000
            if row["Index"] < len(params):
                cull = next((v for k, v in params[row["Index"]].items() if k.startswith("EndCullDistance")), cull)
            key = (label, scatter_id, source, rid)
            if key not in groups:
                groups[key] = {"name": label+":"+scatter_id+":"+resource["name"], "mesh": rid,
                    "stage": stage, "material": material_id(resource["name"]), "cull_distance": cull,
                    "start_time": 112 if "ForestGround" in parent else (80 if stage == "forest" else 0),
                    "instances": [], "source_class": source, "source_component": component["name"],
                    "table_sha1": table["payload_sha1"], "placement": "serialized row + scatter RelativeLocation; parent anchor inherited as identity"}
            groups[key]["instances"].append(scatter_matrix(row, anchor))
    dest = OUTPUT/"meshes"
    dest.mkdir(parents=True, exist_ok=True)
    hashes = {}
    for rid, resource in meshes.items():
        path = resource["base"]/resource["gltf_path"]
        binary = convert_mesh(path)
        (dest/(rid+".fmesh")).write_bytes(binary)
        hashes[rid] = hashlib.sha256(binary).hexdigest()
    scene = {"schema_version": 1, "groups": list(groups.values()),
        "scope_seconds": [0, 120], "table_counts": table_counts, "unresolved_rows": gaps,
        "mesh_hashes": hashes, "limitations": ["Parent container transforms inherited as identity; runtime local/world consumer not fully traced",
        "Preview variant selected where inherited HISM differs; source-component runtime routing unverified",
        "Material roles reconstructed from source identity; dynamic Mapping textures not original shader output",
        "Intro Lower/Grass have no rows in selected table and are not fabricated here"]}
    OUTPUT.mkdir(exist_ok=True)
    (OUTPUT/"scene.json").write_text(json.dumps(scene, separators=(",", ":"), ensure_ascii=False), encoding="utf-8")
    print(json.dumps({"groups": len(groups), "meshes": len(meshes), "instances": sum(len(g["instances"]) for g in groups.values()), "unresolved_rows": len(gaps)}))


if __name__ == "__main__":
    generate()
