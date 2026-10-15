"""Cria atualização de ferramentas P0 sem executável, configurações ou registos."""
import hashlib
import json
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
NAME = "Prismatic-Lumen-Q3-P0-Inspection-Update-2026-10-14"


def package() -> Path:
    archive = ROOT / "dist" / f"{NAME}.zip"
    if archive.exists():
        raise FileExistsError("Atualização já existe; preservar o pacote anterior.")
    sources = {"tools/inspect_session.py": ROOT / "tools/inspect_session.py",
               "tools/session_metrics.py": ROOT / "tools/session_metrics.py",
               "Fantasynt.cmd": ROOT / "tools/package/Fantasynt.cmd",
               "Verify-HandTracking.ps1": ROOT / "tools/package/Verify-HandTracking.ps1",
               "HAND-TRACKING.txt": ROOT / "tools/package/HAND-TRACKING.txt",
               "LE-ME.txt": ROOT / "tools/package/HAND-TRACKING.txt"}
    files = {name: path.read_bytes() for name, path in sources.items()}
    # PowerShell 5.1 exige BOM para reconhecer UTF-8.
    files["Verify-HandTracking.ps1"] = sources["Verify-HandTracking.ps1"].read_text(
        encoding="utf-8-sig").encode("utf-8-sig")
    files["LE-ME-INSPECAO.txt"] = (
        "Extrai este ZIP para a pasta que contém fantasynt.exe e logs.\n"
        "Aceita substituir apenas as ferramentas/instruções incluídas.\n"
        "O ZIP não contém executável, visual-config.json ou registos.\n"
        "O manifest.json original descreve os ficheiros anteriores; os hashes\n"
        "desta atualização constam de inspection-update.json.\n\n"
        "Na mesma pasta, em PowerShell:\n"
        "python .\\tools\\inspect_session.py .\\logs\\hand-tracking-01.jsonl "
        "--output .\\logs\\hand-tracking-01.analysis.json\n\n"
        "Python 3.10+; sem bibliotecas adicionais. Conserva o JSONL completo\n"
        "e envia-o em ZIP para analisar toda a sessão.\n"
    ).encode("utf-8")
    executable_hash = hashlib.sha256((ROOT / "build/Release/fantasynt.exe").read_bytes()).hexdigest()
    manifest = {"inspector_version": "p0-inspector-2", "schema_version_supported": 1,
                "target_executable_sha256": executable_hash,
                "base_commit": "bc86e22", "includes_executable": False,
                "files_sha256": {name: hashlib.sha256(data).hexdigest() for name, data in files.items()}}
    files["inspection-update.json"] = json.dumps(manifest, indent=2).encode("utf-8")
    archive.parent.mkdir(exist_ok=True)
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as output:
        for name, data in files.items():
            output.writestr(name, data)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    archive.with_suffix(".zip.sha256").write_text(f"{digest}  {archive.name}\n", encoding="ascii")
    return archive


if __name__ == "__main__":
    print(package())
