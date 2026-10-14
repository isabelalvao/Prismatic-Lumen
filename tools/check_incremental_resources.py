"""Verifica recursos incrementais; altera fontes temporariamente e repõe-as no fim."""

import argparse
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cmake", required=True)
    args = parser.parse_args()
    shader = ROOT / "shaders" / "fs_extract.sc"
    example = ROOT / "examples" / "balanced.json"
    originals = {path: path.read_bytes() for path in (shader, example)}

    def build() -> None:
        subprocess.run(
            [args.cmake, "--build", str(ROOT / "build"), "--config", "Release",
             "--parallel", "4", "--target", "fantasynt"],
            cwd=ROOT, check=True, stdout=subprocess.DEVNULL,
        )

    try:
        source = originals[shader].decode("utf-8")
        if "brightness-0.85" not in source:
            raise RuntimeError("O teste requer o limiar original de extração")
        shader.write_text(source.replace("brightness-0.85", "brightness-0.851"), encoding="utf-8")
        build()
        generated = ROOT / "build" / "shaders" / "fs_extract.bin"
        deployed = ROOT / "build" / "Release" / "shaders" / "fs_extract.bin"
        if generated.read_bytes() != deployed.read_bytes():
            raise AssertionError("Shader junto do executável ficou desactualizado")
        shader.write_bytes(originals[shader])
        build()
        document = json.loads(originals[example])
        document["id"] = "incremental-resource-test"
        example.write_text(json.dumps(document, indent=2), encoding="utf-8")
        build()
        if example.read_bytes() != (ROOT / "build" / "Release" / "examples" / example.name).read_bytes():
            raise AssertionError("Exemplo junto do executável ficou desactualizado")
        print("PASS: shader e exemplo actualizados sem alteração do código C++")
        return 0
    finally:
        for path, content in originals.items():
            path.write_bytes(content)
        build()


if __name__ == "__main__":
    raise SystemExit(main())
