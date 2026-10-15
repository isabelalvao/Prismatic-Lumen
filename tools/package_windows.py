"""Cria um ZIP x64 portátil a partir da compilação Release e do redistribuível MSVC."""
import argparse
import hashlib
import json
import re
import shutil
import struct
import tempfile
import zipfile
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
NAME = "Fantasynt-PCVR-Win11-x64-0.12.0"
HAND_TRACKING_FILES = {"Fantasynt.cmd", "Test-HandTracking.cmd", "Record-HandTracking.ps1",
                       "Verify-HandTracking.cmd", "Verify-HandTracking.ps1", "HAND-TRACKING.txt"}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_x64(path: Path) -> None:
    data = path.read_bytes()
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe_offset:pe_offset + 4] != b"PE\0\0" or struct.unpack_from("<H", data, pe_offset + 4)[0] != 0x8664:
        raise ValueError(f"Binário não x64: {path}")


def package(build: Path, crt: Path, name: str = NAME, keep_folder: bool = False,
            hand_tracking: bool = False, visual_config: Path | None = None) -> Path:
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]{0,100}", name):
        raise ValueError("Nome de pacote inválido")
    dist = ROOT / "dist"
    dist.mkdir(exist_ok=True)
    archive = dist / f"{name}.zip"
    portable_folder = dist / name
    if name != NAME and (archive.exists() or portable_folder.exists()):
        raise FileExistsError("O pacote já existe; usa um nome novo para o preservar")
    if keep_folder and portable_folder.exists():
        raise FileExistsError("A pasta portátil já existe; usa um nome novo")
    with tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="package-") as temporary:
        destination = Path(temporary) / name
        destination.mkdir()
        for filename in ("fantasynt.exe", "openxr_loader.dll"):
            check_x64(build / filename)
            shutil.copy2(build / filename, destination / filename)
        for folder in ("shaders", "examples", "audio", "opening"):
            shutil.copytree(build / folder, destination / folder)
        shutil.copy2(build / "visual-config.default.json", destination / "visual-config.default.json")
        # O pacote novo parte dos defaults; o JSON afinado na pasta de build fica preservado.
        shutil.copy2(visual_config or build / "visual-config.default.json", destination / "visual-config.json")
        shutil.copy2(ROOT / "assets/visual-config-guide.md", destination / "visual-config-guide.md")
        if len(list((destination / "shaders").glob("*.bin"))) != 10:
            raise ValueError("Esperados dez shaders compilados")
        required = ("msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll")
        if not all((crt / filename).is_file() for filename in required):
            raise ValueError("Pasta CRT x64 inválida")
        for dll in crt.glob("*.dll"):
            check_x64(dll)
            shutil.copy2(dll, destination / dll.name)
        for file in (ROOT / "tools/package").iterdir():
            if file.is_file() and (hand_tracking or file.name not in HAND_TRACKING_FILES):
                shutil.copy2(file, destination / file.name)
        if hand_tracking:
            (destination / "tools").mkdir()
            for script in ("inspect_session.py", "session_metrics.py"):
                shutil.copy2(ROOT / "tools" / script, destination / "tools" / script)
            # Windows PowerShell 5.1 reconhece scripts UTF-8 com BOM.
            for script in HAND_TRACKING_FILES:
                if script.endswith(".ps1"):
                    path = destination / script
                    path.write_text(path.read_text(encoding="utf-8"), encoding="utf-8-sig")
            shutil.copy2(destination / "LE-ME.txt", destination / "LE-ME-GERAL.txt")
            shutil.copy2(destination / "HAND-TRACKING.txt", destination / "LE-ME.txt")
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
        for licence_name, source in licences.items():
            shutil.copy2(source, destination / "licences" / licence_name)
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
        manifest = {"version": "0.12.0", "platform": "Windows 11 x64", "vr": "OpenXR D3D11",
                    "target": "Quest 3 / Virtual Desktop / VDXR", "physical_headset_validated": False,
                    "vr_0_2_user_confirmed": True,
                    "vr_0_3_user_confirmed": True,
                    "scenes": ["opening", "sequence", "night", "landscape", "tunnel", "field"],
                    "default_scene": "opening", "opening_seconds": 424.1821315192744, "default_vr_scale": 1.0,
                    "final_fade_seconds": 16, "final_fade_scope": "entire composite including timer",
                    "ring_pulse_start_seconds": 306, "ring_pulse_groups": "successive bass attacks alternate even cyan / odd pink",
                    "artifact_density_multiplier": 0.3, "artifact_motion": "individual rigid rotation and three-axis drift",
                    "ground_rock_height_m": [0.25,0.7], "ground_rock_scaling": "uniform, normalized mesh variants",
                    "cone_count": 224, "cone_density_multiplier": 1.4,
                    "cone_colour_pairs": ["cyan/magenta","blue/white","violet/gold"],
                    "visual_profile": "editable-director", "camera_start_seconds": 0,
                    "opening_camera_cm_per_second": 150, "pyramids_start_seconds": 80,
                    "pulse_source": "bass onsets detected from selected PCM soundtrack",
                    "visual_config": "visual-config.json", "hot_reload": True, "auto_play": True,
                    "time_display": "MM:SS.d", "time_display_key": "C", "time_display_default": False,
                    "time_display_vr": "shared head-relative plane at 2m, per-eye homography",
                    "pillars_motion": "independent vertical oscillation", "music_phase_anchors": [144,176,208,224],
                    "mini_pillars": "base removed; rigid vertical motion with static buried shaft tail",
                    "coloured_geometry_density_multiplier": 0.25, "timer_y": 0.65, "timer_scale": 1.5,
                    "cone_pieces": 17, "cone_entry": "8 individual ribs, then 9 base-to-tip rings; bass-onset aligned",
                    "cone_rotation_degrees_s": 2, "crystal_variants": 6, "crystal_pieces": 9,
                    "temporal_geometry_scaling": "tunnel rings only",
                    "previous_exit_seconds": [208,224], "tunnel_build_seconds": [224,272], "tunnel_glow_seconds": [294,306],
                    "stochastic_transition_coverage": False,
                    "tunnel_departure_seconds": [290,306], "tunnel_remaining": "64 individually aligned axial rings",
                    "ring_alignment_seconds": [290,306], "ring_inner_radius_m": 5, "ring_spacing_m": 8,
                    "retain_behind_m": 60, "behind_fade_m": 30,
                    "dark_triangles_removed": True, "platforms_removed": True,
                    "sequence_seconds": 424.1821315192744, "audio_playback": True,
                    "audio_clock": "SDL input sample consumption; physical output latency not measured",
                    "sequence_camera_curve": True, "fog_curve_modes": ["Linear", "Constant", "Cubic"],
                    "created_utc": datetime.now(timezone.utc).isoformat(), "files_sha256": files}
        manifest["package_name"] = name
        if hand_tracking:
            manifest["hand_tracking"] = {"extension": "XR_EXT_hand_tracking", "joints_per_hand": 26,
                                         "acquisition_only": True, "hands_rendered": False,
                                         "physical_headset_validated": False,
                                         "record_launcher": "Test-HandTracking.cmd",
                                         "verify_launcher": "Verify-HandTracking.cmd",
                                         "full_inspector": "tools/inspect_session.py",
                                         "full_inspector_python_minimum": "3.10",
                                         "inspector_version": "p0-inspector-2",
                                         "headset_evidence": "User reported VDXR acquisition in initial samples; full session analysis pending",
                                         "python_required": False}
        (destination / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
        if keep_folder:
            shutil.copytree(destination, portable_folder)
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
    parser.add_argument("--name", default=NAME)
    parser.add_argument("--keep-folder", action="store_true")
    parser.add_argument("--hand-tracking", action="store_true")
    parser.add_argument("--visual-config", type=Path)
    args = parser.parse_args()
    print(package(args.build.resolve(), args.crt.resolve(), args.name, args.keep_folder,
                  args.hand_tracking, args.visual_config.resolve() if args.visual_config else None))
