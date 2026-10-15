"""Valida atualização sobre o dist anterior, preservando binário e JSONL."""
import argparse
import hashlib
import json
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def check(update: Path, original: Path) -> dict:
    with tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="inspection João ") as temporary:
        folder = Path(temporary)
        with zipfile.ZipFile(original) as archive:
            archive.extractall(folder)
        folder /= original.stem
        before = {str(path.relative_to(folder)).replace("\\", "/"):
                  hashlib.sha256(path.read_bytes()).hexdigest()
                  for path in folder.rglob("*") if path.is_file()}
        (folder / "logs").mkdir()
        recording = folder / "logs/hand-tracking-01.jsonl"
        shutil.copy2(ROOT / "plan/hand-observation-p0/example-session.jsonl", recording)
        original_record = recording.read_bytes()
        with zipfile.ZipFile(update) as archive:
            if archive.testzip():
                raise ValueError("Atualização ZIP corrompida")
            archive.extractall(folder)
        manifest = json.loads((folder / "inspection-update.json").read_text())
        allowed = {"tools/inspect_session.py", "tools/session_metrics.py", "Fantasynt.cmd",
                   "Verify-HandTracking.ps1", "HAND-TRACKING.txt", "LE-ME.txt", "LE-ME-INSPECAO.txt"}
        if set(manifest["files_sha256"]) - allowed:
            raise ValueError("Atualização contém ficheiros fora das ferramentas/instruções previstas")
        for name, digest in before.items():
            actual = hashlib.sha256((folder / name).read_bytes()).hexdigest()
            if name not in allowed and actual != digest:
                raise ValueError(f"Ficheiro fora da atualização alterado: {name}")
        for name, digest in manifest["files_sha256"].items():
            if hashlib.sha256((folder / name).read_bytes()).hexdigest() != digest:
                raise ValueError(f"Hash de atualização incompatível: {name}")
        if before["fantasynt.exe"] != manifest["target_executable_sha256"]:
            raise ValueError("Executável original não corresponde ao alvo")
        command = [sys.executable, str(folder / "tools/inspect_session.py"), str(recording),
                   "--output", str(recording.with_suffix(".analysis.json")),
                   "--expect-executable-sha256", manifest["target_executable_sha256"]]
        inspected = subprocess.run(command, cwd=folder, capture_output=True, text=True, timeout=15)
        if inspected.returncode != 0:
            raise RuntimeError(inspected.stdout + inspected.stderr)
        verified = subprocess.run(["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass",
                                   "-File", str(folder / "Verify-HandTracking.ps1")],
                                  cwd=folder, capture_output=True, text=True, errors="replace", timeout=15,
                                  creationflags=subprocess.CREATE_NO_WINDOW)
        if verified.returncode != 0:
            raise RuntimeError(verified.stdout + verified.stderr)
        if original_record != recording.read_bytes():
            raise ValueError("JSONL de origem alterado")
        result = {"archive": str(update), "target_executable_sha256": before["fantasynt.exe"],
                  "hashes_valid": True, "previous_assets_preserved": True, "source_record_preserved": True,
                  "standalone_python_inspector_passed": True, "ps51_custom_filename_passed": True,
                  "fixture": "Synthetic runtime recording; not the user's Quest session"}
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("update", type=Path)
    parser.add_argument("original", type=Path)
    arguments = parser.parse_args()
    result = check(arguments.update.resolve(), arguments.original.resolve())
    (ROOT / "plan/hand-observation-p0/inspection-update-check.json").write_text(
        json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps(result, indent=2))
