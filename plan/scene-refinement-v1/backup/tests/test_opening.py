"""Cenas de referência nos primeiros120s e seleção de resolução VR."""
import json
import os
import socket
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))


class OpeningTests(unittest.TestCase):
    def launch(self, seconds, *extra):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as folder:
            with socket.socket() as s:
                s.bind(("127.0.0.1", 0))
                port = s.getsockname()[1]
            result = subprocess.run([str(EXE), "--scene", "opening", "--hidden", "--frames", "8",
                "--fixed-time", str(seconds), "--port", str(port), *extra], cwd=folder,
                env=dict(os.environ, SDL_AUDIODRIVER="dummy"), capture_output=True, text=True,
                timeout=25, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            self.assertEqual(result.returncode, 0, result.stderr)
            return [json.loads(x) for x in result.stdout.splitlines() if x.startswith("{")]

    def test_stages_use_recovered_meshes(self):
        for seconds in (0, 40, 80, 96, 112, 120):
            with self.subTest(seconds=seconds):
                ready, final = self.launch(seconds)
                self.assertGreater(ready["opening_meshes"], 0)
                self.assertEqual(final["scene"], "opening")
                self.assertEqual(final["sequence_seconds"], seconds)
                self.assertTrue(final["opening_active"])

    def test_camera_starts_immediately_and_uses_clear_route(self):
        ready, initial = self.launch(0)
        _, advanced = self.launch(20)
        _, later = self.launch(21)
        self.assertAlmostEqual(initial["camera_position"], -6400.00048828125, delta=0.001)
        self.assertGreater(advanced["camera_position"], initial["camera_position"])
        self.assertAlmostEqual(later["camera_position"]-advanced["camera_position"], 150, delta=0.001)
        self.assertGreater(ready["route_adjustments"], 0)
        self.assertGreater(ready["detected_bass_beats"], 0)

    def test_opening_rejects_time_beyond_six_minutes(self):
        result = subprocess.run([str(EXE), "--scene", "opening", "--start-time", "361"],
            capture_output=True, text=True, timeout=5,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        self.assertEqual(result.returncode, 1)
        self.assertIn("Opening time", result.stderr)

    def test_audio_stops_at_six_minutes(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as folder:
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            result = subprocess.run([str(EXE), "--scene", "opening", "--hidden", "--frames", "35",
                "--start-time", "359.9", "--port", str(port)], cwd=folder,
                env=dict(os.environ, SDL_AUDIODRIVER="dummy"), capture_output=True, text=True,
                timeout=15, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            self.assertEqual(result.returncode, 0, result.stderr)
            final = json.loads(result.stdout.splitlines()[-1])
            self.assertEqual(final["sequence_seconds"], 360)
            self.assertEqual(final["audio_seconds"], 360)

    def test_vr_resolution_defaults_to_one(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as folder:
            manifest = Path(folder) / "runtime.json"
            manifest.write_text(json.dumps({"file_format_version": "1.0.0", "runtime": {
                "library_path": str(EXE.parent / "fantasynt_mock_openxr.dll")}}))
            with socket.socket() as s:
                s.bind(("127.0.0.1", 0))
                port = s.getsockname()[1]
            result = subprocess.run([str(EXE), "--vr", "--hidden", "--frames", "3", "--port", str(port)],
                cwd=folder, env=dict(os.environ, XR_RUNTIME_JSON=str(manifest),
                    FANTASYNT_MOCK_MODE="native-resolution", SDL_AUDIODRIVER="dummy"), capture_output=True,
                text=True, timeout=20, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            self.assertEqual(result.returncode, 0, result.stderr)
            ready = json.loads(result.stdout.splitlines()[0])
            self.assertEqual(ready["vr_scale"], 1.0)
