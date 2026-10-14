"""Validação do caminho OpenXR através de imagens D3D11 reais num runtime simulado."""
import json
import os
import socket
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))
RUNTIME = EXE.parent / "fantasynt_mock_openxr.dll"


class VRTests(unittest.TestCase):
    def launch(self, *arguments, mode="normal"):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="xr-test-") as folder:
            folder = Path(folder)
            manifest = folder / "runtime.json"
            manifest.write_text(json.dumps({"file_format_version": "1.0.0", "runtime": {
                "library_path": str(RUNTIME), "functions": {
                    "xrNegotiateLoaderRuntimeInterface": "xrNegotiateLoaderRuntimeInterface"}}}))
            environment = dict(os.environ, XR_RUNTIME_JSON=str(manifest), FANTASYNT_MOCK_MODE=mode)
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            capture = folder / "mirror.png"
            result = subprocess.run([str(EXE), "--port", str(port), *arguments,
                *(["--capture", str(capture)] if "--vr" in arguments and mode != "no-headset" else [])],
                env=environment, cwd=folder, capture_output=True, text=True, timeout=30,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            pixels = capture.read_bytes() if capture.exists() else b""
            return result, pixels

    def test_stereo_images_reach_runtime_and_mirror(self):
        result, image = self.launch("--vr", "--hidden", "--frames", "12", "--preset", "energetic")
        self.assertEqual(result.returncode, 0, result.stderr)
        reports = [json.loads(line) for line in result.stdout.splitlines() if line.startswith("{")]
        self.assertEqual(reports[0]["mode"], "openxr")
        self.assertIn("FantasyntMock", reports[0]["runtime"])
        self.assertEqual(reports[-1]["frames"], 12)
        self.assertTrue(image.startswith(b"\x89PNG\r\n\x1a\n"))
        self.assertGreater(len(image), 2000)

    def test_absent_headset_is_error_without_desktop_fallback(self):
        result, _ = self.launch("--vr", "--hidden", "--frames", "1", mode="no-headset")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Find connected VR headset", result.stderr)
        self.assertNotIn('"type":"ready"', result.stdout)

    def test_each_scene_and_transition_reaches_both_eyes(self):
        for scene, seconds in (("night", "40"), ("landscape", "180"),
                               ("sequence", "96"), ("sequence", "228"), ("tunnel", "300")):
            with self.subTest(scene=scene, seconds=seconds):
                result, image = self.launch("--vr", "--hidden", "--frames", "6",
                    "--scene", scene, "--fixed-time", seconds)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertGreater(len(image), 2000)

    def test_vr_sequence_can_start_paused(self):
        result, _ = self.launch("--vr", "--hidden", "--frames", "6", "--start-time", "96", "--paused")
        self.assertEqual(result.returncode, 0, result.stderr)
        report = json.loads(result.stdout.splitlines()[-1])
        self.assertEqual(report["sequence_seconds"], 96)

    def test_probe_reports_runtime_and_headset(self):
        result, _ = self.launch("--xr-probe")
        self.assertEqual(result.returncode, 0, result.stderr)
        report = json.loads(result.stdout)
        self.assertTrue(report["headset_available"])
        self.assertEqual(report["runtime"], "FantasyntMock")

    def test_nonrendering_frames_and_tracking_loss_recover(self):
        result, _ = self.launch("--vr", "--hidden", "--frames", "4", mode="skip-frames")
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_shader_failure_also_releases_vr_session(self):
        result, _ = self.launch("--vr", "--hidden", "--frames", "1", "--shaders", str(ROOT / "temp/missing-xr-shaders"))
        self.assertEqual(result.returncode, 1, result.stderr)
        self.assertIn("Shader file missing", result.stderr)

    def test_runtime_loss_is_explicit_error(self):
        for mode in ("loss-session", "loss-instance"):
            with self.subTest(mode=mode):
                result, _ = self.launch("--vr", "--hidden", "--frames", "12", mode=mode)
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertIn("OpenXR runtime lost", result.stderr)


if __name__ == "__main__":
    unittest.main()
