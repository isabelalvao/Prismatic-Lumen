"""Verifica capturas reais da montagem, atmosfera e transformação dos anéis."""
import json
import os
from pathlib import Path
import socket
import subprocess

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "plan/scene-refinement-v1/captures"
EXE = ROOT / "build/Release/fantasynt.exe"


def capture(seconds: float, label: str, overrides: dict | None = None, backward_vr: bool = False) -> Path:
    config = json.loads((ROOT / "visual-config.json").read_text(encoding="utf-8"))
    for section, values in (overrides or {}).items():
        config[section].update(values)
    config_path = OUTPUT / f"{label}.json"
    config_path.write_text(json.dumps(config), encoding="utf-8")
    path = OUTPUT / f"{label}.png"
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    environment = dict(os.environ, SDL_AUDIODRIVER="dummy")
    if backward_vr:
        runtime = OUTPUT / "runtime-backward.json"
        runtime.write_text(json.dumps({"file_format_version": "1.0.0", "runtime": {
            "library_path": str(EXE.parent / "fantasynt_mock_openxr.dll")}}), encoding="utf-8")
        environment.update(XR_RUNTIME_JSON=str(runtime), FANTASYNT_MOCK_MODE="backward")
    result = subprocess.run(
        [str(EXE), "--config", str(config_path), "--hidden", "--silent",
         "--fixed-time", str(seconds), "--frames", "8", "--port", str(port),
         "--width", "1280", "--height", "720", "--capture", str(path),
         *(["--vr"] if backward_vr else [])],
        cwd=ROOT, env=environment,
        capture_output=True, text=True, timeout=40,
        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
    )
    (OUTPUT / f"{label}.log").write_text(result.stdout + result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError(result.stderr)
    final = json.loads(result.stdout.splitlines()[-1])
    assert final["sequence_seconds"] == seconds
    assert not final["time_display_visible"]
    with Image.open(path) as rendered:
        assert rendered.size == (1280, 720)
    return path


def same(a: Path, b: Path) -> bool:
    with Image.open(a) as first, Image.open(b) as second:
        return ImageChops.difference(first.convert("RGB"), second.convert("RGB")).getbbox() is None


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    phases = (136, 140, 148, 154, 160, 168, 174, 180, 207, 240, 284, 290, 298, 306, 330, 360)
    images = {time: capture(time, f"phase-{time}") for time in phases}
    assert not same(images[207], capture(207, "red-saturated", {"red_phase": {"saturation": 1}}))
    assert not same(images[180], capture(180, "dark-triangles-enabled", {"layout": {"remove_dark_triangles": False}}))
    assert not same(images[180], capture(180, "platforms-enabled", {"layout": {"remove_platforms": False}}))
    assert not same(images[174], capture(174, "reflections-disabled", {"floor": {"reflections_enabled": False}}))
    assert same(images[290], capture(290, "departure-start-far", {"extension": {"tunnel_departure_distance_m": 500}}))
    assert not same(images[298], capture(298, "departure-mid-far", {"extension": {"tunnel_departure_distance_m": 500}}))
    assert same(images[306], capture(306, "departure-finished-far", {"extension": {"tunnel_departure_distance_m": 500}}))
    assert not same(images[306], capture(306, "departure-disabled", {"extension": {"tunnel_departure_start_seconds": 424}}))
    assert not same(images[306], capture(306, "alignment-disabled", {"extension": {"ring_alignment_start_seconds": 424}}))
    assert not same(images[306], capture(306, "glow-disabled", {"extension": {"tunnel_glow_emission": 0}}))
    assert not same(images[306], capture(306, "ring-spacing-wide", {"extension": {"ring_spacing_m": 16}}))
    assert not same(images[306], capture(306, "ring-radius-wide", {"extension": {"ring_inner_radius_m": 9}}))
    assert not same(capture(140, "cone-beat-sync"), capture(140, "cone-no-beat-sync", {"cones": {"beat_sync": False}}))
    assert not same(capture(174, "vr-backward-retained", backward_vr=True),
                    capture(174, "vr-backward-immediate", {"layout": {"retain_behind_m": 0, "behind_fade_m": 0.1}}, backward_vr=True))
    (OUTPUT.parent / "gpu-validation.json").write_text(
        json.dumps({"phases": list(phases), "gpu_checks": 14, "backward_vr": "simulated", "status": "passed"}, indent=2), encoding="utf-8"
    )
    print("GPU: construção, cor quente, reflexos, remoção de obstáculos, dispersão e anéis validados.")


if __name__ == "__main__":
    main()
