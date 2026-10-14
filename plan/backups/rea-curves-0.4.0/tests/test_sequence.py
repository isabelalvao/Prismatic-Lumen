"""Reprodução real e manutenção dos comandos durante transições de composição."""
import json
import os
import socket
import subprocess
import tempfile
import time
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))


class SequenceApplicationTests(unittest.TestCase):
    def test_fixed_time_selects_each_phase_and_overlap(self):
        for seconds, weights in ((40, [1, 0, 0]), (96, [0.5, 0.5, 0]),
                                 (180, [0, 1, 0]), (228, [0, 0.5, 0.5]), (300, [0, 0, 1])):
            with self.subTest(seconds=seconds), tempfile.TemporaryDirectory(dir=ROOT / "temp") as folder:
                with socket.socket() as reservation:
                    reservation.bind(("127.0.0.1", 0))
                    port = reservation.getsockname()[1]
                image = Path(folder) / "capture.png"
                result = subprocess.run([str(EXE), "--hidden", "--frames", "8", "--scene", "sequence",
                    "--fixed-time", str(seconds), "--port", str(port), "--capture", str(image)],
                    cwd=folder, capture_output=True, text=True, timeout=20,
                    creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
                self.assertEqual(result.returncode, 0, result.stderr)
                report = json.loads(result.stdout.splitlines()[-1])
                self.assertEqual(report["sequence_seconds"], seconds)
                self.assertEqual(report["composition_weights"], weights)
                self.assertGreater(image.stat().st_size, 2000)

    def test_commands_survive_crossing_into_tunnel(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as folder:
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            process = subprocess.Popen([str(EXE), "--hidden", "--frames", "150", "--scene", "sequence",
                "--start-time", "223.5", "--port", str(port)], cwd=folder,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            try:
                for _ in range(200):
                    try:
                        connection = socket.create_connection(("127.0.0.1", port), timeout=1)
                        break
                    except OSError:
                        if process.poll() is not None:
                            self.fail(process.communicate()[1])
                        time.sleep(0.025)
                else:
                    self.fail("Servidor indisponível")
                with connection, connection.makefile("rb") as reader:
                    def send(identifier, parameters):
                        connection.sendall(json.dumps({"schema_version": 1, "id": identifier,
                            "command": "apply_parameters", "transition_seconds": 0.1,
                            "parameters": parameters}).encode() + b"\n")
                        return json.loads(reader.readline())
                    first = send("sequence-control", {"light_intensity": 0.17, "motion_speed": 0})
                    self.assertEqual(first["status"], "accepted")
                    time.sleep(1.0)
                    second = send("sequence-read", {"bloom": 0.23})
                    self.assertEqual(second["revision"], first["revision"] + 1)
                    self.assertAlmostEqual(second["current"]["light_intensity"], 0.17, places=5)
                    self.assertEqual(second["target"]["motion_speed"], 0)
                stdout, stderr = process.communicate(timeout=15)
                self.assertEqual(process.returncode, 0, stderr)
                final = json.loads(stdout.splitlines()[-1])
                self.assertGreater(final["composition_weights"][2], 0)
                self.assertEqual(final["accepted_commands"], 2)
            finally:
                if process.poll() is None:
                    process.kill()
                    process.communicate()

    def test_invalid_time_is_rejected_before_startup(self):
        for value in ("nan", "inf", "-1", "425", "abc"):
            result = subprocess.run([str(EXE), "--fixed-time", value], capture_output=True,
                text=True, timeout=5, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            self.assertEqual(result.returncode, 1)
            self.assertIn("Time must be", result.stderr)
