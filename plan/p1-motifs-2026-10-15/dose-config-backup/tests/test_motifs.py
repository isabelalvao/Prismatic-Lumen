"""P1: guião determinístico, imagens D3D11 e runtime XR simulado; sem medição física."""
import ctypes
import json
import os
import queue
import socket
import subprocess
import tempfile
import threading
import time
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))


def key(process_id, code):
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    callback_type = ctypes.WINFUNCTYPE(ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p)
    user32.EnumWindows.argtypes = [callback_type, ctypes.c_void_p]
    user32.GetWindowThreadProcessId.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ulong)]
    user32.PostMessageW.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_size_t, ctypes.c_ssize_t]
    windows = []

    def visit(window, _):
        owner = ctypes.c_ulong()
        user32.GetWindowThreadProcessId(window, ctypes.byref(owner))
        if owner.value == process_id:
            windows.append(window)
        return 1

    user32.EnumWindows(callback_type(visit), None)
    if not windows:
        raise RuntimeError("Janela de ensaio não encontrada")
    for window in windows:
        user32.PostMessageW(window, 0x100, code, 0)
        user32.PostMessageW(window, 0x101, code, 0xC0000000)


class MotifTests(unittest.TestCase):
    def folder(self):
        (ROOT / "temp").mkdir(exist_ok=True)
        return tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="p1-test-")

    def command(self, folder, *arguments, enabled=True, mode=None, capture=False):
        folder = Path(folder)
        with socket.socket() as reservation:
            reservation.bind(("127.0.0.1", 0))
            port = reservation.getsockname()[1]
        command = [str(EXE), "--hidden", "--width", "640", "--height", "360",
                   "--config", str(ROOT / "visual-config.json"), "--port", str(port),
                   "--record-session", str(folder / "session.jsonl"), *arguments]
        if enabled:
            command += ["--motifs-scheduled"]
        if capture:
            command += ["--capture", str(folder / "mirror.png")]
        env = dict(os.environ, SDL_AUDIODRIVER="dummy")
        if mode:
            manifest = folder / "runtime.json"
            manifest.write_text(json.dumps({"file_format_version": "1.0.0", "runtime": {
                "library_path": str(EXE.parent / "fantasynt_mock_openxr.dll"), "functions": {
                    "xrNegotiateLoaderRuntimeInterface": "xrNegotiateLoaderRuntimeInterface"}}}))
            env.update(XR_RUNTIME_JSON=str(manifest), FANTASYNT_MOCK_MODE=mode,
                       DISABLE_XR_APILAYER_VIRTUALDESKTOP_OCULUS_COMPATIBILITY="1")
            command += ["--vr"]
        return command, env, port

    def launch(self, folder, *arguments, **kwargs):
        command, env, _ = self.command(folder, *arguments, **kwargs)
        result = subprocess.run(command, env=env, cwd=folder, capture_output=True,
                                text=True, timeout=40, creationflags=subprocess.CREATE_NO_WINDOW)
        rows = [json.loads(line) for line in (Path(folder) / "session.jsonl").read_text().splitlines()]
        return result, rows

    def test_effect_in_four_phases_and_other_channels_preserved(self):
        for seconds in (54, 156, 192, 338):
            with self.subTest(seconds=seconds), self.folder() as on, self.folder() as off:
                args = ("--silent", "--fixed-time", str(seconds), "--frames", "8")
                result, rows = self.launch(on, *args, capture=True)
                baseline, originals = self.launch(off, *args, enabled=False, capture=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(baseline.returncode, 0, baseline.stderr)
                self.assertNotEqual((Path(on) / "mirror.png").read_bytes(), (Path(off) / "mirror.png").read_bytes())
                sample = next(r["environment"] for r in rows if r["type"] == "observation")
                original = next(r["environment"] for r in originals if r["type"] == "observation")
                self.assertEqual(rows[0]["condition"], "p1_scheduled")
                self.assertEqual(sample["base"], original["base"])
                self.assertEqual(sample["directed"], original["directed"])
                self.assertAlmostEqual(sample["master_gain"], .88)
                self.assertAlmostEqual(sample["effective"]["light_intensity"], .54, places=6)
                expected = dict(sample["directed"], light_intensity=sample["effective"]["light_intensity"])
                self.assertEqual(sample["effective"], expected)
                for field in ("camera_virtual_z_cm", "composition_weights", "exposure", "bass_pulse", "ring_pulse_group"):
                    self.assertEqual(sample[field], original[field])

    def test_neutral_windows_and_final_fade_match_disabled(self):
        for seconds in (50, 100, 216, 296, 392, 416, 424.1821315192744):
            with self.subTest(seconds=seconds), self.folder() as on, self.folder() as off:
                args = ("--silent", "--fixed-time", str(seconds), "--frames", "6")
                result, rows = self.launch(on, *args, capture=True)
                baseline, _ = self.launch(off, *args, enabled=False, capture=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(baseline.returncode, 0, baseline.stderr)
                self.assertEqual((Path(on) / "mirror.png").read_bytes(), (Path(off) / "mirror.png").read_bytes())
                for row in rows:
                    if row["type"] == "observation":
                        self.assertEqual(row["environment"]["master_gain"], 1)

    def test_sdl_clock_crosses_first_opportunity(self):
        with self.folder() as folder:
            result, rows = self.launch(folder, "--start-time", "49.8", "--frames", "100", "--play")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue(any(r["type"] == "motif_event" and r["event"] == "started" and r["plan_id"] == "M1" for r in rows))
            samples = [r["environment"] for r in rows if r["type"] == "observation"]
            self.assertTrue(all(r["audio_enabled"] for r in samples))
            self.assertTrue(any(r["master_gain"] < .99 for r in samples))
            for sample in samples:
                self.assertLess(abs(sample["audio_seconds"] - sample["scene_seconds"]), .1)
            self.assertTrue(rows[-1]["complete"])

    def test_xr_loss_of_hands_does_not_cancel_but_invalid_head_holds(self):
        for mode, suspended in (("hands-lost", False), ("head-position-only", True), ("skip-frames", False), ("recenter", False)):
            with self.subTest(mode=mode), self.folder() as folder:
                result, rows = self.launch(folder, "--silent", "--fixed-time", "54", "--frames", "12", mode=mode)
                self.assertEqual(result.returncode, 0, result.stderr)
                samples = [r["environment"] for r in rows if r["type"] == "observation"]
                self.assertEqual(samples[-1]["technical_paused"], suspended)
                started = [r for r in rows if r["type"] == "motif_event" and r["event"] == "started"]
                self.assertEqual(bool(started), not suspended)
                if mode == "skip-frames":
                    self.assertTrue(any(r["technical_paused"] for r in samples))
                if mode == "recenter":
                    self.assertTrue(any(r["type"] == "motif_event" and r["event"] == "suspended" for r in rows))

    def test_operator_pause_restart_and_frozen_controls(self):
        with self.folder() as folder:
            command, env, port = self.command(folder, "--start-time", "54", "--paused", "--frames", "300")
            process = subprocess.Popen(command, env=env, cwd=folder, stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE, text=True, creationflags=subprocess.CREATE_NO_WINDOW)
            ready = queue.Queue()
            reader = threading.Thread(target=lambda: ready.put(process.stdout.readline()), daemon=True)
            reader.start()
            try:
                self.assertEqual(json.loads(ready.get(timeout=30))["type"], "ready")
                key(process.pid, 0x20)  # retoma
                time.sleep(.25)
                key(process.pid, 0x20)  # pausa no motivo
                time.sleep(.25)
                key(process.pid, 0x74)  # F5: rejeitado
                with socket.create_connection(("127.0.0.1", port), timeout=5) as client:
                    client.sendall(b'{"schema_version":1,"id":"freeze","command":"apply_parameters","transition_seconds":1,"parameters":{"light_intensity":0.9}}\n')
                    receipt = json.loads(client.makefile("rb").readline())
                    self.assertEqual(receipt["status"], "rejected")
                    self.assertEqual(receipt["error_code"], "p1_configuration_frozen")
                key(process.pid, 0x20)  # retoma e reagenda numa epoch nova
                time.sleep(.15)
                key(process.pid, 0x51)  # Q: reinício com o mesmo relógio SDL
                time.sleep(.2)
                key(process.pid, 0x1B)
                _, stderr = process.communicate(timeout=15)
                self.assertEqual(process.returncode, 0, stderr)
            finally:
                if process.poll() is None:
                    process.kill()
                    process.communicate()
                reader.join(timeout=1)
            rows = [json.loads(line) for line in (Path(folder) / "session.jsonl").read_text().splitlines()]
            events = [r for r in rows if r["type"] == "motif_event"]
            self.assertTrue(any(r["event"] == "suspended" for r in events))
            self.assertTrue(any(r["event"] == "cancelled" and r["reason"] == "operator_restart" for r in events))
            self.assertTrue(any(r["type"] == "control_rejected" for r in rows))
            self.assertTrue(any(r["event"] == "scheduled" and r["reason"] == "resume_new_epoch" for r in events))
            samples = [r["environment"] for r in rows if r["type"] == "observation"]
            self.assertLess(samples[-1]["scene_seconds"], 1)
            self.assertEqual(samples[-1]["master_gain"], 1)
            self.assertTrue(all(s["config_revision"] == 0 and s["controller_revision"] == 0 for s in samples))

    def test_runtime_exception_cancels_and_marks_incomplete(self):
        with self.folder() as folder:
            result, rows = self.launch(folder, "--silent", "--fixed-time", "54", "--frames", "12", mode="loss-session")
            self.assertNotEqual(result.returncode, 0)
            self.assertTrue(any(r["type"] == "motif_event" and r["event"] == "cancelled" and r["reason"] == "exception_or_scope_exit" for r in rows))
            self.assertFalse(rows[-1]["complete"])

    def test_explicit_enable_creates_new_record_without_path_and_rejects_other_scene(self):
        with self.folder() as folder:
            command, env, _ = self.command(folder, "--silent", "--fixed-time", "54", "--frames", "6")
            index = command.index("--record-session")
            del command[index:index + 2]
            result = subprocess.run(command, env=env, cwd=folder, capture_output=True,
                                    text=True, timeout=30, creationflags=subprocess.CREATE_NO_WINDOW)
            self.assertEqual(result.returncode, 0, result.stderr)
            records = list((Path(folder) / "logs").glob("p1-*.jsonl"))
            self.assertEqual(len(records), 1)
            self.assertTrue(json.loads(records[0].read_text().splitlines()[-1])["complete"])
            invalid = subprocess.run(command + ["--scene", "sequence"], env=env, cwd=folder,
                                     capture_output=True, text=True, timeout=30, creationflags=subprocess.CREATE_NO_WINDOW)
            self.assertNotEqual(invalid.returncode, 0)
            self.assertIn("Scheduled P1 requires", invalid.stderr)


if __name__ == "__main__":
    unittest.main()
