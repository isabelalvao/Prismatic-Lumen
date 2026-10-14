"""Integração M1: executável real, mensagens reais e captura gráfica."""

import json
import os
import socket
import subprocess
import time
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build" / "Release" / "fantasynt.exe"))


class ApplicationTests(unittest.TestCase):
    def test_missing_shader_is_explicit_startup_error(self):
        result = subprocess.run(
            [str(EXE), "--hidden", "--frames", "1", "--shaders", str(ROOT / "temp" / "missing-shaders")],
            cwd=ROOT, capture_output=True, text=True, timeout=10,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Shader file missing", result.stderr)

    def test_occupied_port_is_explicit_startup_error(self):
        with socket.socket() as listener:
            listener.bind(("127.0.0.1", 0))
            listener.listen()
            result = subprocess.run(
                [str(EXE), "--hidden", "--frames", "1", "--port", str(listener.getsockname()[1])],
                cwd=ROOT, capture_output=True, text=True, timeout=10,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
            )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Error:", result.stderr)
        self.assertNotIn('"type":"ready"', result.stdout)

    def test_invalid_utf8_does_not_terminate_application(self):
        with socket.socket() as reservation:
            reservation.bind(("127.0.0.1", 0))
            port = reservation.getsockname()[1]
        process = subprocess.Popen(
            [str(EXE), "--hidden", "--frames", "120", "--port", str(port)],
            cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
        try:
            connection = None
            for _ in range(200):
                if process.poll() is not None:
                    break
                try:
                    connection = socket.create_connection(("127.0.0.1", port), timeout=0.25)
                    break
                except OSError:
                    time.sleep(0.025)
            self.assertIsNotNone(connection)
            with connection, connection.makefile("rb") as reader:
                connection.settimeout(2)
                connection.sendall(b'{"id":"\xff"}\n')
                rejected = json.loads(reader.readline())
                self.assertEqual(rejected["status"], "rejected")
                self.assertEqual(rejected["revision"], 0)
                command = {
                    "schema_version": 1, "id": "after-utf8", "command": "apply_parameters",
                    "transition_seconds": 0.1, "parameters": {"light_intensity": 0.2},
                }
                connection.sendall(json.dumps(command).encode() + b"\n")
                accepted = json.loads(reader.readline())
                self.assertEqual(accepted["status"], "accepted")
                self.assertEqual(accepted["revision"], 1)
            stdout, stderr = process.communicate(timeout=10)
            self.assertEqual(process.returncode, 0, stderr)
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()

    def test_rejects_invalid_patch_without_changing_state(self):
        self.assertTrue(EXE.is_file(), "A aplicação ainda não foi compilada")
        with socket.socket() as reservation:
            reservation.bind(("127.0.0.1", 0))
            port = reservation.getsockname()[1]
        capture = ROOT / "plan" / "m1-integration.png"
        process = subprocess.Popen(
            [str(EXE), "--hidden", "--frames", "360", "--port", str(port), "--capture", str(capture)],
            cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
        try:
            connection = None
            for _ in range(200):
                if process.poll() is not None:
                    break
                try:
                    connection = socket.create_connection(("127.0.0.1", port), timeout=0.25)
                    break
                except OSError:
                    time.sleep(0.025)
            self.assertIsNotNone(connection, "A aplicação não abriu o servidor local")
            with connection, connection.makefile("rb") as reader:
                connection.settimeout(2)
                command = {
                    "schema_version": 1, "id": "integration-1", "command": "apply_parameters",
                    "transition_seconds": 0.2, "parameters": {"light_intensity": 0.2},
                }

                def send(document):
                    connection.sendall(json.dumps(document).encode() + b"\n")
                    return json.loads(reader.readline())

                accepted = send(command)
                self.assertEqual(accepted["status"], "accepted")
                self.assertEqual(accepted["revision"], 1)
                self.assertAlmostEqual(accepted["target"]["light_intensity"], 0.2, places=5)
                command["id"] = "invalid-1"
                command["parameters"] = {"light_intensity": 0.9, "fog_density": -1}
                rejected = send(command)
                self.assertEqual(rejected["status"], "rejected")
                self.assertEqual(rejected["revision"], 1)
                self.assertEqual(rejected["target"], accepted["target"])
                command["id"] = "integration-2"
                command["parameters"] = {"bloom": 0.8}
                later = send(command)
                self.assertEqual(later["status"], "accepted")
                self.assertEqual(later["revision"], 2)
                self.assertAlmostEqual(later["target"]["light_intensity"], 0.2, places=5)
            self.assertIsNone(process.poll(), "O cliente desligado terminou a aplicação")
            stdout, stderr = process.communicate(timeout=20)
            self.assertEqual(process.returncode, 0, stderr)
            self.assertTrue(capture.is_file())
            self.assertGreater(capture.stat().st_size, 1000)
            self.assertEqual(capture.read_bytes()[:8], b"\x89PNG\r\n\x1a\n")
            self.assertIn('"type":"performance"', stdout)
        finally:
            if process.poll() is None:
                process.kill()
                process.communicate()


if __name__ == "__main__":
    unittest.main()
