"""Cria um ZIP x64 portátil a partir da compilação Release e do redistribuível MSVC."""
import argparse
import hashlib
import json
import shutil
import struct
import tempfile
import zipfile
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
NAME = "Fantasynt-PCVR-Win11-x64-0.3.0"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_x64(path: Path) -> None:
    data = path.read_bytes()
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe_offset:pe_offset + 4] != b"PE\0\0" or struct.unpack_from("<H", data, pe_offset + 4)[0] != 0x8664:
        raise ValueError(f"Binário não x64: {path}")


def package(build: Path, crt: Path) -> Path:
    dist = ROOT / "dist"
    dist.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="package-") as temporary:
        destination = Path(temporary) / NAME
        destination.mkdir()
        for name in ("fantasynt.exe", "openxr_loader.dll"):
            check_x64(build / name)
            shutil.copy2(build / name, destination / name)
        for folder in ("shaders", "examples"):
            shutil.copytree(build / folder, destination / folder)
        if len(list((destination / "shaders").glob("*.bin"))) != 8:
            raise ValueError("Esperados oito shaders compilados")
        required = ("msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll")
        if not all((crt / name).is_file() for name in required):
            raise ValueError("Pasta CRT x64 inválida")
        for dll in crt.glob("*.dll"):
            check_x64(dll)
            shutil.copy2(dll, destination / dll.name)
        for file in (ROOT / "tools/package").iterdir():
            if file.is_file():
                shutil.copy2(file, destination / file.name)
        licences = {
            "SDL3.txt": ROOT / "build/_deps/sdl3-src/LICENSE.txt",
            "bgfx.txt": ROOT / "build/_deps/bgfx_cmake-src/bgfx/LICENSE",
            "bx.txt": ROOT / "build/_deps/bgfx_cmake-src/bx/LICENSE",
            "bimg.txt": ROOT / "build/_deps/bgfx_cmake-src/bimg/LICENSE",
            "nlohmann-json.txt": ROOT / "build/_deps/nlohmann_json-src/LICENSE.MIT",
            "OpenXR.txt": ROOT / "build/_deps/openxr-src/LICENSE",
            "OpenXR-COPYING.adoc": ROOT / "build/_deps/openxr-src/COPYING.adoc",
            "OpenXR-jsoncpp.txt": ROOT / "build/_deps/openxr-src/src/external/jsoncpp/LICENSE",
        }
        (destination / "licences").mkdir()
        for name, source in licences.items():
            shutil.copy2(source, destination / "licences" / name)
        # Conserva também os avisos dos componentes de terceiros presentes nas bibliotecas.
        for library in ("bgfx", "bx", "bimg"):
            third_party = ROOT / "build/_deps/bgfx_cmake-src" / library / "3rdparty"
            for source in third_party.rglob("*"):
                if source.is_file() and source.name.lower().startswith(("license", "copying")):
                    target = destination / "licences" / library / source.relative_to(third_party)
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(source, target)
        (destination / "licences/Microsoft-CRT.txt").write_text(
            "Microsoft Visual C++ Runtime DLLs, copyright Microsoft Corporation.\n"
            "Redistributed from the installed Visual Studio Build Tools x64 CRT folder.\n"
            "https://learn.microsoft.com/cpp/windows/redistributing-visual-cpp-files\n", encoding="utf-8")
        files = {str(file.relative_to(destination)).replace("\\", "/"): digest(file)
                 for file in sorted(destination.rglob("*")) if file.is_file()}
        manifest = {"version": "0.3.0", "platform": "Windows 11 x64", "vr": "OpenXR D3D11",
                    "target": "Quest 3 / Virtual Desktop / VDXR", "physical_headset_validated": False,
                    "vr_0_2_user_confirmed": True,
                    "created_utc": datetime.now(timezone.utc).isoformat(), "files_sha256": files}
        (destination / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
        archive = dist / f"{NAME}.zip"
        with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as output:
            for file in sorted(destination.rglob("*")):
                if file.is_file():
                    output.write(file, file.relative_to(destination.parent))
        archive.with_suffix(".zip.sha256").write_text(f"{digest(archive)}  {archive.name}\n", encoding="ascii")
    return archive


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", type=Path, default=ROOT / "build/Release")
    parser.add_argument("--crt", type=Path, required=True)
    args = parser.parse_args()
    print(package(args.build.resolve(), args.crt.resolve()))
