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
NAME = "Fantasynt-PCVR-Win11-x64-0.9.0"


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
        for folder in ("shaders", "examples", "audio", "opening"):
            shutil.copytree(build / folder, destination / folder)
        shutil.copy2(build / "visual-config.default.json", destination / "visual-config.default.json")
        # O pacote novo parte dos defaults; o JSON afinado na pasta de build fica preservado.
        shutil.copy2(build / "visual-config.default.json", destination / "visual-config.json")
        shutil.copy2(ROOT / "assets/visual-config-guide.md", destination / "visual-config-guide.md")
        if len(list((destination / "shaders").glob("*.bin"))) != 10:
            raise ValueError("Esperados dez shaders compilados")
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
        manifest = {"version": "0.9.0", "platform": "Windows 11 x64", "vr": "OpenXR D3D11",
                    "target": "Quest 3 / Virtual Desktop / VDXR", "physical_headset_validated": False,
                    "vr_0_2_user_confirmed": True,
                    "vr_0_3_user_confirmed": True,
                    "scenes": ["opening", "sequence", "night", "landscape", "tunnel", "field"],
                    "default_scene": "opening", "opening_seconds": 360, "default_vr_scale": 1.0,
                    "visual_profile": "editable-director", "camera_start_seconds": 0,
                    "opening_camera_cm_per_second": 150, "pyramids_start_seconds": 80,
                    "pulse_source": "bass onsets detected from selected PCM soundtrack",
                    "visual_config": "visual-config.json", "hot_reload": True, "auto_play": True,
                    "time_display": "MM:SS.d", "time_display_key": "C", "time_display_default": True,
                    "pillars_motion": "independent vertical oscillation", "music_phase_anchors": [144,176,208,224],
                    "mini_pillars": "base removed; shaft changes height from fixed floor contact",
                    "coloured_geometry_density_multiplier": 0.25, "timer_y": 0.65, "timer_scale": 1.5,
                    "cone_segments": 7, "cone_segment_interval_seconds": 2, "cone_rotation_degrees_s": 2,
                    "previous_exit_seconds": [208,224], "tunnel_build_seconds": [224,272], "tunnel_glow_seconds": [272,284],
                    "stochastic_transition_coverage": False,
                    "sequence_seconds": 424, "audio_playback": True,
                    "audio_clock": "SDL input sample consumption; physical output latency not measured",
                    "sequence_camera_curve": True, "fog_curve_modes": ["Linear", "Constant", "Cubic"],
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
