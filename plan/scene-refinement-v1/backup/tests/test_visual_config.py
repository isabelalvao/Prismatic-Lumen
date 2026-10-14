"""Afinação através de ficheiro: reload real, erros atómicos e arranque imediato."""
import json
import os
import socket
import subprocess
import tempfile
import time
import unittest
import wave
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))


class VisualConfigTests(unittest.TestCase):
    def test_short_soundtrack_uses_configured_duration(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as directory:
            folder = Path(directory)
            soundtrack = folder / "short.wav"
            with wave.open(str(soundtrack), "wb") as output:
                output.setnchannels(2)
                output.setsampwidth(2)
                output.setframerate(44100)
                output.writeframes(bytes(44100 * 4 * 3))
            config = folder / "visual.json"
            config.write_text(json.dumps({"schema_version": 1, "timeline": {"opening_duration_seconds": 2},
                "startup": {"soundtrack": str(soundtrack)}}), encoding="utf-8")
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            result = subprocess.run([str(EXE), "--config", str(config), "--hidden", "--frames", "12",
                "--port", str(port)], cwd=folder, env=dict(os.environ, SDL_AUDIODRIVER="dummy"),
                capture_output=True, text=True, timeout=15,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            self.assertEqual(result.returncode, 0, result.stderr)
            ready, final = [json.loads(line) for line in result.stdout.splitlines() if line.startswith("{")]
            self.assertTrue(ready["audio_enabled"])
            self.assertEqual(ready["detected_bass_beats"], 0)
            self.assertGreater(final["audio_seconds"], 0)

    def test_configuration_changes_without_restarting_and_rejects_invalid_file(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as directory:
            folder = Path(directory)
            config = folder / "configuração visual João.json"
            document = {"schema_version": 1, "camera": {"speed_m_s": 0.5},
                        "parameters": {"light_intensity": 0.2}}
            config.write_text(json.dumps(document), encoding="utf-8")
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            process = subprocess.Popen([str(EXE), "--config", str(config), "--hidden",
                "--frames", "210", "--port", str(port)], cwd=folder,
                env=dict(os.environ, SDL_AUDIODRIVER="dummy"), stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, text=True, encoding="utf-8",
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            try:
                ready_line = process.stdout.readline()
                self.assertTrue(ready_line, process.stderr.read() if process.poll() is not None else "No ready")
                ready = json.loads(ready_line)
                self.assertEqual(ready["type"], "ready")
                self.assertFalse(ready["paused"])
                time.sleep(0.3)
                document["camera"]["speed_m_s"] = 1
                document["parameters"]["light_intensity"] = 0.4
                document["startup"] = {"window_width": 900}
                config.write_text(json.dumps(document), encoding="utf-8")
                time.sleep(0.7)
                config.write_text('{"schema_version":1,"camera":{"speed_m_s":-10}}', encoding="utf-8")
                time.sleep(0.5)
                document["traces"] = {"speed_m_s": 0.25}
                config.write_text(json.dumps(document), encoding="utf-8")
                stdout, stderr = process.communicate(timeout=20)
                self.assertEqual(process.returncode, 0, stderr)
                events = [json.loads(line) for line in stdout.splitlines() if line.startswith("{")]
                reloads = [event for event in events if event["type"] == "config_reloaded"]
                self.assertGreaterEqual(len(reloads), 2)
                self.assertTrue(all(event["restart_required"] for event in reloads))
                self.assertIn("config_rejected", stderr)
                final = next(event for event in reversed(events) if event["type"] == "performance")
                self.assertGreater(final["camera_position"], -6400 + 80)
                self.assertEqual(final["config_revision"], len(reloads))
                self.assertEqual(final["camera_speed_m_s"], 1)
                self.assertEqual(final["trace_speed_m_s"], 0.25)
            finally:
                if process.poll() is None:
                    process.kill()
                    process.communicate()

    def test_default_movement_has_no_initial_wait(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as directory:
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            result = subprocess.run([str(EXE), "--hidden", "--frames", "8", "--fixed-time", "1",
                "--port", str(port)], cwd=directory, capture_output=True, text=True, timeout=15,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            self.assertEqual(result.returncode, 0, result.stderr)
            final = json.loads(result.stdout.splitlines()[-1])
            self.assertAlmostEqual(final["camera_position"], -6250.00048828125, delta=0.001)

    def test_shortening_duration_preserves_camera_with_audio_and_silent(self):
        for silent in (False, True):
            with self.subTest(silent=silent), tempfile.TemporaryDirectory(dir=ROOT / "temp") as directory:
                folder = Path(directory)
                config = folder / "visual.json"
                config.write_text('{"schema_version":1}', encoding="utf-8")
                with socket.socket() as reservation:
                    reservation.bind(("127.0.0.1", 0))
                    port = reservation.getsockname()[1]
                process = subprocess.Popen([str(EXE), "--config", str(config), "--hidden", "--frames", "100",
                    "--start-time", "115", "--port", str(port), *(["--silent"] if silent else [])],
                    cwd=folder, env=dict(os.environ, SDL_AUDIODRIVER="dummy"), stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE, text=True, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
                try:
                    self.assertTrue(process.stdout.readline())
                    time.sleep(0.25)
                    config.write_text('{"schema_version":1,"timeline":{"opening_duration_seconds":20},"pulse":{"relative_threshold":0.06}}', encoding="utf-8")
                    stdout, stderr = process.communicate(timeout=20)
                    self.assertEqual(process.returncode, 0, stderr)
                    events = [json.loads(line) for line in stdout.splitlines() if line.startswith("{")]
                    final = events[-1]
                    self.assertEqual(final["sequence_seconds"], 20)
                    self.assertGreaterEqual(final["camera_position"], -6400.00048828125+115*150)
                    if not silent:
                        self.assertEqual(final["audio_seconds"], 20)
                finally:
                    if process.poll() is None:
                        process.kill()
                        process.communicate()

    def test_duration_and_detector_reload_do_not_rewind_audio(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as directory:
            folder = Path(directory)
            config = folder / "visual.json"
            config.write_text('{"schema_version":1}', encoding="utf-8")
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            process = subprocess.Popen([str(EXE), "--config", str(config), "--hidden", "--frames", "100",
                "--start-time", "100", "--port", str(port)], cwd=folder,
                env=dict(os.environ, SDL_AUDIODRIVER="dummy"), stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, text=True, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            try:
                self.assertTrue(process.stdout.readline())
                time.sleep(0.25)
                config.write_text('{"schema_version":1,"timeline":{"opening_duration_seconds":150},"pulse":{"low_frequency_hz":40}}', encoding="utf-8")
                stdout, stderr = process.communicate(timeout=20)
                self.assertEqual(process.returncode, 0, stderr)
                events = [json.loads(line) for line in stdout.splitlines() if line.startswith("{")]
                reload = next(event for event in events if event["type"] == "config_reloaded")
                self.assertAlmostEqual(reload["audio_seconds_before"], reload["audio_seconds_after"], delta=0.001)
                self.assertGreater(events[-1]["audio_seconds"], 100)
            finally:
                if process.poll() is None:
                    process.kill()
                    process.communicate()
