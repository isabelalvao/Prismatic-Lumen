"""Áudio real carregado; relógio de consumo verificado com saída SDL simulada."""
import json
import os
import socket
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))


class AudioApplicationTests(unittest.TestCase):
    def launch(self, *args, capture=False):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as folder:
            with socket.socket() as s:
                s.bind(("127.0.0.1", 0))
                port = s.getsockname()[1]
            image = Path(folder) / "capture.png"
            result = subprocess.run([str(EXE), "--scene", "sequence", "--hidden", "--frames", "30", "--port", str(port), *args,
                *(["--capture", str(image)] if capture else [])],
                cwd=folder, env=dict(os.environ, SDL_AUDIODRIVER="dummy"),
                capture_output=True, text=True, timeout=15,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            if capture and result.returncode == 0:
                self.assertGreater(image.stat().st_size, 2000)
            return result

    def test_audio_drives_sequence_clock(self):
        result = self.launch("--start-time", "224")
        self.assertEqual(result.returncode, 0, result.stderr)
        final = json.loads(result.stdout.splitlines()[-1])
        self.assertTrue(final["audio_enabled"])
        self.assertGreater(final["audio_seconds"], 224.1)
        self.assertAlmostEqual(final["audio_seconds"], final["sequence_seconds"], delta=0.04)

    def test_paused_seek_does_not_consume_audio(self):
        result = self.launch("--start-time", "180", "--paused")
        self.assertEqual(result.returncode, 0, result.stderr)
        final = json.loads(result.stdout.splitlines()[-1])
        self.assertTrue(final["audio_enabled"])
        self.assertAlmostEqual(final["audio_seconds"], 180, places=4)
        self.assertAlmostEqual(final["sequence_seconds"], 180, places=4)

    def test_silent_preserves_visual_sequence(self):
        result = self.launch("--silent", "--start-time", "224")
        self.assertEqual(result.returncode, 0, result.stderr)
        final = json.loads(result.stdout.splitlines()[-1])
        self.assertFalse(final["audio_enabled"])
        self.assertGreater(final["sequence_seconds"], 224.1)

    def test_missing_audio_fails_explicitly(self):
        result = self.launch("--audio", str(ROOT / "temp/missing-audio.wav"))
        self.assertEqual(result.returncode, 1)
        self.assertIn("Audio", result.stderr)

    def test_capture_stops_audio_and_sequence_together(self):
        result = self.launch("--start-time", "224", capture=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        final = json.loads(result.stdout.splitlines()[-1])
        self.assertAlmostEqual(final["audio_seconds"], final["sequence_seconds"], places=4)
