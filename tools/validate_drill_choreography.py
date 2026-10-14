"""Capturas e regressões gráficas da emergência e dispersão dirigida."""
import json
import os
from pathlib import Path
import socket
import subprocess

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "plan/drill-rings-v1/captures"
EXE = ROOT / "build/Release/fantasynt.exe"


def capture(seconds: float, label: str, extension=None) -> Path:
    config = OUTPUT / f"{label}.json"
    config.write_text(json.dumps({"schema_version": 1, "extension": extension or {}}), encoding="utf-8")
    image = OUTPUT / f"{label}.png"
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    result = subprocess.run([str(EXE), "--config", str(config), "--hidden", "--silent",
        "--fixed-time", str(seconds), "--frames", "8", "--port", str(port),
        "--width", "1280", "--height", "720", "--capture", str(image)],
        cwd=ROOT, env=dict(os.environ, SDL_AUDIODRIVER="dummy"), capture_output=True,
        text=True, timeout=30, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    (OUTPUT / f"{label}.log").write_text(result.stdout + result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError(result.stderr)
    report = json.loads(result.stdout.splitlines()[-1])
    assert not report["time_display_visible"]
    assert report["sequence_seconds"] == seconds
    with Image.open(image) as rendered:
        assert rendered.size == (1280, 720)
    return image


def same(first: Path, second: Path) -> bool:
    with Image.open(first) as a, Image.open(second) as b:
        return ImageChops.difference(a.convert("RGB"), b.convert("RGB")).getbbox() is None


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    images = {seconds: capture(seconds, f"phase-{seconds}") for seconds in
        (136, 140, 143, 148, 150, 153, 156, 224, 240, 270, 284, 300, 330, 360)}
    # Completada a dispersão, a distância de saída já não altera a imagem: só ficam anéis.
    assert same(images[300], capture(300, "departure-far", {"tunnel_departure_distance_m": 500}))
    # Antes da saída, a mesma distância também não pode alterar nenhum objeto.
    assert same(images[270], capture(270, "departure-start-far", {"tunnel_departure_distance_m": 500}))
    assert not same(images[284], capture(284, "departure-disabled", {"tunnel_departure_start_seconds": 424}))
    assert not same(images[284], capture(284, "departure-mid-far", {"tunnel_departure_distance_m": 500}))
    assert not same(images[300], capture(300, "departure-disabled-end", {"tunnel_departure_start_seconds": 424}))
    print("GPU captures: hidden timer, departure onset, moving artifacts and rings-only finish passed")


if __name__ == "__main__":
    main()
