"""Gera canais C++ a partir dos metadados recuperados, sem incluir recursos UE4."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "plan/full-reference/timeline.json"
OUTPUT = ROOT / "src/render/sequence_curves.hpp"
ASSET_SHA1 = "e34b3d02621b8a31185f4cb5b43bc6d6515daf9a"


def generate() -> str:
    data = json.loads(SOURCE.read_text(encoding="utf-8"))
    if data["sha1"] != ASSET_SHA1:
        raise ValueError("Recurso inesperado")
    camera = [t for t in data["tracks"] if t["binding_name"] == "CameraRig"]
    fog = [t for t in data["tracks"] if t["binding_name"] == "HeightFogComponent0"
           and t["track"].get("PropertyName") == "FogInscatteringColor"]
    if len(camera) != 1 or len(fog) != 1:
        raise ValueError("Canais ambíguos")
    channels = [("camera_y", camera[0]["sections"][0]["properties"]["Translation[1]"])]
    channels += [("fog_" + c.lower(), fog[0]["sections"][0]["properties"][c + "Curve"])
                 for c in ("Red", "Green", "Blue")]
    lines = ["// Gerado por tools/generate_sequence_curves.py; não editar à mão.",
             f"// ChezNousSequence SHA1: {ASSET_SHA1}",
             f"// timeline.json SHA256: {hashlib.sha256(SOURCE.read_bytes()).hexdigest()}",
             "// Regras nativas: plan/rea-investigation/native-richcurve-eval.json.",
             "#pragma once", '#include "sequence.hpp"', "#include <array>",
             "namespace fantasynt::sequence_curves {"]
    for name, channel in channels:
        keys = channel["Keys"]
        if any(k in channel for k in ("PreInfinityExtrap", "PostInfinityExtrap")):
            raise ValueError("Override de extrapolação exige análise própria")
        if any(k["mode_bytes"][0] not in (0, 1, 2) for k in keys):
            raise ValueError("Modo não suportado no conjunto selecionado")
        if any(a["time"] >= b["time"] for a, b in zip(keys, keys[1:])):
            raise ValueError("Tempos fora de ordem")
        lines.append(f"inline constexpr std::array<CurveKey,{len(keys)}> {name}{{{{")
        for key in keys:
            values = [key[v] for v in ("time", "value", "arrive_tangent", "leave_tangent")]
            fields = [repr(float(value)) + "f" for value in values]
            fields.append(str(key["mode_bytes"][0]))
            lines.append("    {" + ",".join(fields) + "},")
        lines.append("}};")
    return "\n".join(lines + ["}", ""])


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    content = generate()
    if args.check:
        if OUTPUT.read_text(encoding="utf-8") != content:
            raise SystemExit("Canais gerados desatualizados")
        print("Canais gerados coincidem com a evidência")
    else:
        OUTPUT.write_text(content, encoding="utf-8")
