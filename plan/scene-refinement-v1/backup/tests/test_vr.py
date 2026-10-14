"""Validação do caminho OpenXR através de imagens D3D11 reais num runtime simulado."""
import ctypes
import json
import os
import queue
import socket
import subprocess
import tempfile
import threading
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))
RUNTIME = EXE.parent / "fantasynt_mock_openxr.dll"


class VRTests(unittest.TestCase):
    def launch(self, *arguments, mode="normal", toggle_timer=False):
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
            command = [str(EXE), "--port", str(port), *arguments,
                *(["--capture", str(capture)] if "--vr" in arguments and mode != "no-headset" else [])]
            if toggle_timer:
                process = subprocess.Popen(command, env=environment, cwd=folder,
                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
                    creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
                ready_messages = queue.Queue()
                reader = threading.Thread(target=lambda: ready_messages.put(process.stdout.readline()), daemon=True)
                reader.start()
                try:
                    try:
                        ready_line = ready_messages.get(timeout=30)
                    except queue.Empty:
                        self.fail("O processo VR não anunciou ready em 30 segundos")
                    reader.join(timeout=1)
                    self.assertEqual(json.loads(ready_line)["type"], "ready")
                    self.post_timer_key(process.pid)
                    stdout, stderr = process.communicate(timeout=30)
                    result = subprocess.CompletedProcess(command, process.returncode,
                                                         ready_line + stdout, stderr)
                finally:
                    if process.poll() is None:
                        process.kill()
                        reader.join(timeout=2)
                        process.communicate()
            else:
                result = subprocess.run(command, env=environment, cwd=folder,
                    capture_output=True, text=True, timeout=30,
                    creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            pixels = capture.read_bytes() if capture.exists() else b""
            return result, pixels

    def post_timer_key(self, process_id):
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
                title = ctypes.create_unicode_buffer(256)
                user32.GetWindowTextW(ctypes.c_void_p(window), title, len(title))
                if title.value.startswith("Fantasynt "):
                    windows.append(window)
            return True

        self.assertTrue(user32.EnumWindows(callback_type(visit), None))
        self.assertEqual(len(windows), 1, "A janela SDL deve pertencer apenas ao processo do teste")
        # O código virtual permite ao SDL receber C sem alterar o foco nem o teclado físico.
        self.assertTrue(user32.PostMessageW(windows[0], 0x0100, ord("C"), 1))
        self.assertTrue(user32.PostMessageW(windows[0], 0x0101, ord("C"), 0xC0000001))

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

    def test_timer_starts_hidden_even_with_legacy_visible_config(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="xr-timer-") as folder:
            config = Path(folder) / "visual-config.json"
            config.write_text(json.dumps({"schema_version": 1, "timer": {"visible": True}}))
            result, _ = self.launch("--vr", "--hidden", "--silent", "--frames", "6",
                                    "--fixed-time", "15", "--config", str(config))
            self.assertEqual(result.returncode, 0, result.stderr)
            report = json.loads(result.stdout.splitlines()[-1])
            self.assertFalse(report["time_display_visible"])

    @unittest.skipUnless(os.name == "nt", "Tecla C enviada à janela SDL Windows real")
    def test_vr_timer_key_draws_plate_in_mirror(self):
        with tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="xr-timer-key-") as folder:
            config = Path(folder) / "visual-config.json"
            config.write_text(json.dumps({"schema_version": 1, "timer": {"visible": True}}))
            arguments = ("--vr", "--hidden", "--silent", "--frames", "90",
                         "--fixed-time", "15", "--config", str(config))
            hidden, hidden_image = self.launch(*arguments)
            self.assertEqual(hidden.returncode, 0, hidden.stderr)
            self.assertFalse(json.loads(hidden.stdout.splitlines()[-1])["time_display_visible"])
            visible, visible_image = self.launch(*arguments, toggle_timer=True)
            self.assertEqual(visible.returncode, 0, visible.stderr)
            report = json.loads(visible.stdout.splitlines()[-1])
            self.assertTrue(report["time_display_visible"])
            self.assertEqual(report["time_display"], "00:15.0")
            self.assertTrue(hidden_image.startswith(b"\x89PNG\r\n\x1a\n"))
            self.assertTrue(visible_image.startswith(b"\x89PNG\r\n\x1a\n"))
            # Tempo e número de frames idênticos fixam a cena e a pose do runtime simulado.
            self.assertNotEqual(visible_image, hidden_image,
                                "C deve desenhar a placa no shader VR e alterar a captura")

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
