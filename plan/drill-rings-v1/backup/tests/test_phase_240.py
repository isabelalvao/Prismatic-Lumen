"""Percurso de quatro minutos e contador associado ao relógio da experiência."""
import ctypes
import json
import os
import socket
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))


class PhaseTests(unittest.TestCase):
    def run_at(self, seconds):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as directory:
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            result = subprocess.run([str(EXE), "--hidden", "--fixed-time", str(seconds),
                "--frames", "8", "--port", str(port)], cwd=directory,
                env=dict(os.environ, SDL_AUDIODRIVER="dummy"), capture_output=True,
                text=True, timeout=20, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            self.assertEqual(result.returncode, 0, result.stderr)
            return json.loads(result.stdout.splitlines()[-1])

    def test_six_minutes_and_musical_transitions(self):
        phases = {seconds: self.run_at(seconds) for seconds in (120,150,180,216,224,240,272,284,360)}
        for seconds, phase in phases.items():
            self.assertEqual(phase["sequence_seconds"], seconds)
            self.assertEqual(phase["time_display"], f"{seconds // 60:02d}:{seconds % 60:02d}.0")
            self.assertTrue(phase["time_display_visible"])
        self.assertGreater(phases[150]["composition_weights"][1], 0)
        self.assertLess(phases[240]["composition_weights"][2], 1)
        self.assertEqual(phases[224]["previous_exit_progress"], 1)
        self.assertEqual(phases[240]["tunnel_light_weight"], 0)
        self.assertEqual(phases[272]["tunnel_light_weight"], 0)
        self.assertEqual(phases[284]["tunnel_light_weight"], 1)
        self.assertGreater(phases[360]["composition_weights"][2], 0.9)
        self.assertLess(phases[216]["visual_energy"], phases[180]["visual_energy"])

    def test_audio_stops_at_six_minutes(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as directory:
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            result = subprocess.run([str(EXE), "--hidden", "--start-time", "359.9",
                "--frames", "35", "--port", str(port)], cwd=directory,
                env=dict(os.environ, SDL_AUDIODRIVER="dummy"), capture_output=True,
                text=True, timeout=20, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            self.assertEqual(result.returncode, 0, result.stderr)
            final = json.loads(result.stdout.splitlines()[-1])
            self.assertEqual(final["audio_seconds"], 360)
            self.assertEqual(final["sequence_seconds"], 360)

    @unittest.skipUnless(os.name == "nt", "Teclas enviadas à janela Windows real")
    def test_counter_can_be_toggled_with_keyboard(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp") as directory:
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            process = subprocess.Popen([str(EXE), "--hidden", "--fixed-time", "85.6",
                "--frames", "90", "--port", str(port)], cwd=directory,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            try:
                ready = json.loads(process.stdout.readline())
                self.assertEqual(ready["type"], "ready")
                user32 = ctypes.WinDLL("user32", use_last_error=True)
                callback_type = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
                found = []

                def visit(window, _):
                    pid = ctypes.c_ulong()
                    user32.GetWindowThreadProcessId(ctypes.c_void_p(window), ctypes.byref(pid))
                    if pid.value == process.pid:
                        found.append(window)
                    return True

                user32.EnumWindows(callback_type(visit), None)
                self.assertTrue(found, "A janela SDL do processo deve existir")
                user32.PostMessageW(ctypes.c_void_p(found[0]), 0x0100, ord("C"), 1)
                user32.PostMessageW(ctypes.c_void_p(found[0]), 0x0101, ord("C"), 0xC0000001)
                stdout, stderr = process.communicate(timeout=20)
                self.assertEqual(process.returncode, 0, stderr)
                final = json.loads(stdout.splitlines()[-1])
                self.assertFalse(final["time_display_visible"])
                self.assertEqual(final["time_display"], "01:25.6")
            finally:
                if process.poll() is None:
                    process.kill()
                    process.communicate()


if __name__ == "__main__":
    unittest.main()
