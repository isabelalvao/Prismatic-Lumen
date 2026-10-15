"""Verifica o ZIP portátil P0 e os lançadores Windows PowerShell sem Quest real."""
import argparse
import hashlib
import json
import os
import socket
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def check(archive_path: Path) -> dict:
    results = {}
    with tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="portable Q3 João ") as temporary, \
            tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="xr-hand-package-") as runtime_folder:
        temporary = Path(temporary)
        with zipfile.ZipFile(archive_path) as archive:
            if archive.testzip():
                raise ValueError("ZIP corrompido")
            archive.extractall(temporary)
        folder = temporary / archive_path.stem
        manifest = json.loads((folder / "manifest.json").read_text(encoding="utf-8"))
        if manifest["package_name"] != folder.name or not manifest["hand_tracking"]["acquisition_only"]:
            raise ValueError("Identidade/capacidade incorreta no manifesto")
        for name, expected in manifest["files_sha256"].items():
            if hashlib.sha256((folder / name).read_bytes()).hexdigest() != expected:
                raise ValueError(f"Hash incorreto: {name}")
        if (folder / "fantasynt_mock_openxr.dll").exists():
            raise ValueError("Runtime simulado não pode ser distribuído")
        for script in ("Record-HandTracking.ps1", "Verify-HandTracking.ps1"):
            if not (folder / script).read_bytes().startswith(b"\xef\xbb\xbf"):
                raise ValueError("Script PowerShell5.1 sem BOM UTF-8")
        required = ("fantasynt.exe", "openxr_loader.dll", "msvcp140.dll",
                    "vcruntime140.dll", "vcruntime140_1.dll", "Test-HandTracking.cmd",
                    "Verify-HandTracking.cmd", "Diagnose-VR.cmd", "LE-ME.txt", "Fantasynt.cmd",
                    "tools/inspect_session.py", "tools/session_metrics.py")
        if not all((folder / name).is_file() for name in required):
            raise ValueError("Ficheiro obrigatório ausente")
        # O loader OpenXR lê o manifesto externo por uma interface de caminho estreita.
        runtime = Path(runtime_folder) / "runtime.json"
        runtime.write_text(json.dumps({"file_format_version": "1.0.0", "runtime": {
            "library_path": str(ROOT / "build/Release/fantasynt_mock_openxr.dll"),
            "functions": {"xrNegotiateLoaderRuntimeInterface": "xrNegotiateLoaderRuntimeInterface"}}}))
        environment = dict(os.environ, XR_RUNTIME_JSON=str(runtime), SDL_AUDIODRIVER="dummy")
        probe = subprocess.run([str(folder / "fantasynt.exe"), "--xr-probe"],
                               cwd=folder, env=environment, capture_output=True,
                               text=True, timeout=15, creationflags=subprocess.CREATE_NO_WINDOW)
        if probe.returncode:
            raise RuntimeError(probe.stderr)
        results["probe"] = json.loads(probe.stdout)
        for mode, expected in (("normal", 0), ("no-hands", 2), ("loss-session", 1)):
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            environment["FANTASYNT_MOCK_MODE"] = mode
            run = subprocess.run(["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass",
                                  "-File", str(folder / "Record-HandTracking.ps1"),
                                  "--hidden", "--silent", "--fixed-time", "152",
                                  "--frames", "8", "--port", str(port)],
                                 cwd=folder, env=environment, capture_output=True,
                                 text=True, errors="replace", timeout=45,
                                 creationflags=subprocess.CREATE_NO_WINDOW)
            if run.returncode != expected:
                raise RuntimeError(f"{mode}: código {run.returncode}; stdout={run.stdout}; stderr={run.stderr}")
            report_file = max((folder / "logs").glob("hands-*.inspection.json"),
                              key=lambda path: path.stat().st_mtime_ns)
            report = json.loads(report_file.read_text(encoding="utf-8"))
            if mode == "normal" and not report["hand_acquisition_observed"]:
                raise ValueError("Aquisição não observada no cenário válido")
            if mode == "no-hands" and (report["hand_trackers_ready"] or report["hand_acquisition_observed"]):
                raise ValueError("Aquisição falsamente anunciada sem extensão")
            if mode == "loss-session" and report["complete"]:
                raise ValueError("Falha XR anunciada como sessão completa")
            results[mode] = {"exit_code": run.returncode, "inspection": report}
            recording = Path(report["session_file"])
            inspection = subprocess.run([sys.executable, str(folder / "tools/inspect_session.py"),
                                         str(recording), "--output", str(recording.with_suffix(".analysis.json")),
                                         "--expect-executable-sha256", manifest["files_sha256"]["fantasynt.exe"]],
                                        cwd=folder, capture_output=True, text=True, timeout=15)
            if inspection.returncode != (1 if mode == "loss-session" else 0):
                raise RuntimeError(f"Inspetor portátil {mode}: {inspection.stdout}; {inspection.stderr}")
            results[mode]["full_inspector"] = json.loads(inspection.stdout)
        if len(list((folder / "logs").glob("hands-*.jsonl"))) != 3:
            raise ValueError("Gravações repetidas não foram preservadas")
        results["independent_extraction"] = True
        results["hashes_valid"] = True
        results["python_required_on_destination"] = False
        results["physical_headset_validated"] = False
        results["archive"] = str(archive_path)
    return results


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--output", type=Path, default=ROOT / "plan/hand-observation-p0/package-check.json")
    arguments = parser.parse_args()
    result = check(arguments.archive.resolve())
    arguments.output.write_text(
        json.dumps(result, indent=2, ensure_ascii=False), encoding="utf-8")
    print("Portable P0: hashes, external extraction, PS5.1, tracking, absence and runtime failure passed")


if __name__ == "__main__":
    main()
