"""Extrai e testa o pacote portátil fora da árvore de compilação."""
import hashlib
import json
import os
import socket
import subprocess
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ARCHIVE = ROOT / "dist/Fantasynt-PCVR-Win11-x64-0.10.0.zip"


def run_scene(folder: Path, environment: dict, vr: bool) -> dict:
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    process = subprocess.Popen([str(folder / "fantasynt.exe"), "--hidden", "--frames", "180",
        "--scene", "opening", "--start-time", "79.5", "--play",
        "--port", str(port), "--preset", "energetic", "--capture", str(folder / "check.png"),
        *( ["--vr"] if vr else [])], cwd=folder, env=environment,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    try:
        client = subprocess.run(["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass",
            "-File", str(folder / "Send-Parameters.ps1"), "-File", "examples/calm.json", "-Port", str(port)],
            cwd=folder, capture_output=True, text=True, timeout=10,
            errors="replace",
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        stdout, stderr = process.communicate(timeout=30)
        if process.returncode or client.returncode:
            raise RuntimeError(f"Scene={process.returncode}: {stderr}; Client={client.returncode}: {client.stderr}")
        reports = [json.loads(line) for line in stdout.splitlines() if line.startswith("{")]
        assert reports[0]["mode"] == ("openxr" if vr else "desktop")
        assert reports[-1]["accepted_commands"] == 1
        assert reports[-1]["sequence"]
        assert reports[-1]["audio_enabled"]
        assert abs(reports[-1]["sequence_seconds"]-reports[-1]["audio_seconds"])<0.04
        assert reports[-1]["opening_active"]
        assert reports[0]["opening_meshes"] > 0
        assert reports[0]["vr_scale"] == 1
        assert reports[0]["detected_bass_beats"] > 0
        assert reports[0]["route_adjustments"] > 0
        assert not reports[0]["paused"]
        assert not reports[-1]["time_display_visible"]
        assert (folder / "check.png").read_bytes().startswith(b"\x89PNG\r\n\x1a\n")
        return {"ready": reports[0], "performance": reports[-1], "client_receipt": json.loads(client.stdout)}
    finally:
        if process.poll() is None:
            process.kill()
            process.communicate()


def run_extension(folder: Path, environment: dict, seconds: int, vr: bool) -> dict:
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    capture = folder / f"phase-{seconds}-{'vr' if vr else 'desktop'}.png"
    result = subprocess.run([str(folder / "fantasynt.exe"), "--hidden", "--frames", "8",
        "--fixed-time", str(seconds), "--port", str(port), "--capture", str(capture),
        *(["--vr"] if vr else [])], cwd=folder, env=environment,
        capture_output=True, text=True, timeout=30,
        creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    assert result.returncode == 0, result.stderr
    reports = [json.loads(line) for line in result.stdout.splitlines() if line.startswith("{")]
    final = reports[-1]
    assert final["sequence_seconds"] == seconds
    assert not final["time_display_visible"]
    assert final["time_display"] == f"{seconds // 60:02d}:{seconds % 60:02d}.0"
    if seconds >= 272:
        assert final["composition_weights"][2] == 1
        assert final["geometry_density_multiplier"] == 1
    elif seconds == 150:
        assert 0.24 <= final["geometry_density_multiplier"] <= 0.26
    if 224 <= seconds <= 272:
        assert final["previous_exit_progress"] == 1
        assert final["tunnel_light_weight"] == 0
    if seconds >= 284:
        assert final["tunnel_light_weight"] == 1
    assert capture.read_bytes().startswith(b"\x89PNG\r\n\x1a\n")
    return final


def main() -> None:
    with tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="portable João check ") as temporary, \
            tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="xr-manifest-") as runtime_folder:
        with zipfile.ZipFile(ARCHIVE) as archive:
            archive.extractall(temporary)
        folder = next(Path(temporary).iterdir())
        manifest = json.loads((folder / "manifest.json").read_text())
        assert manifest["version"] == "0.10.0"
        assert manifest["opening_seconds"] == 360
        config = json.loads((folder / "visual-config.json").read_text(encoding="utf-8"))
        assert config["timer"]["y"] == 0.65 and config["timer"]["scale"] == 1.5
        assert config["extension"]["geometry_density_multiplier"] == 0.25
        assert config["extension"]["tunnel_transition_seconds"] == 48
        assert config["timer"]["visible"] is False
        assert config["cones"]["segment_interval_seconds"] == 2
        assert config["crystals"]["variants"] == 6
        assert manifest["temporal_geometry_scaling"] is False
        assert config["extension"]["tunnel_departure_start_seconds"] == 270
        for name, expected in manifest["files_sha256"].items():
            assert hashlib.sha256((folder / name).read_bytes()).hexdigest() == expected, name
        assert not (folder / "fantasynt_mock_openxr.dll").exists()
        environment = dict(os.environ, SDL_AUDIODRIVER="dummy")
        desktop = run_scene(folder, environment, False)
        # O loader lê o manifesto do runtime separado da pasta da aplicação.
        runtime = Path(runtime_folder) / "mock-runtime.json"
        runtime.write_text(json.dumps({"file_format_version": "1.0.0", "runtime": {
            "library_path": str(ROOT / "build/Release/fantasynt_mock_openxr.dll")}}))
        environment.update(XR_RUNTIME_JSON=str(runtime), FANTASYNT_MOCK_MODE="normal")
        vr = run_scene(folder, environment, True)
        extension = {str(seconds): run_extension(folder, environment, seconds, True) for seconds in (140,156,180,224,240,270,272,284,300,360)}
        report = {"archive": str(ARCHIVE), "manifest_hashes_valid": True, "desktop": desktop,
                  "openxr_simulated": vr, "extension_openxr_simulated": extension, "physical_headset_validated": False}
        (ROOT / "plan/spectral-v1/package-check.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
        print("Portable package: desktop + stereo OpenXR + PowerShell JSON client passed")


if __name__ == "__main__":
    main()
