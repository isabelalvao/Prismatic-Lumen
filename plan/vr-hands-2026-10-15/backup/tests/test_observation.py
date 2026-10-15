"""Aquisição e JSONL no runtime simulado; não constitui prova no Quest real."""
import hashlib
import json
import os
import socket
import subprocess
import tempfile
import unittest
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from inspect_session import inspect
EXE = Path(os.environ.get("FANTASYNT_EXE", ROOT / "build/Release/fantasynt.exe"))
RUNTIME = EXE.parent / "fantasynt_mock_openxr.dll"


class ObservationTests(unittest.TestCase):
    def launch(self, folder, mode="normal", record=True, vr=True, capture=False):
        folder = Path(folder)
        runtime = folder / "runtime.json"
        runtime.write_text(json.dumps({"file_format_version": "1.0.0", "runtime": {
            "library_path": str(RUNTIME), "functions": {
                "xrNegotiateLoaderRuntimeInterface": "xrNegotiateLoaderRuntimeInterface"}}}))
        # O runtime simulado não deve atravessar a camada global do Virtual Desktop.
        environment = dict(os.environ, XR_RUNTIME_JSON=str(runtime),
                           DISABLE_XR_APILAYER_VIRTUALDESKTOP_OCULUS_COMPATIBILITY="1",
                           FANTASYNT_MOCK_MODE=mode, SDL_AUDIODRIVER="dummy")
        with socket.socket() as reservation:
            reservation.bind(("127.0.0.1", 0))
            port = reservation.getsockname()[1]
        path = folder / "session.jsonl"
        command = [str(EXE), "--hidden", "--silent", "--fixed-time", "152",
                   "--frames", "12", "--port", str(port),
                   "--config", str(ROOT / "visual-config.json")]
        if vr:
            command += ["--vr"]
        if record:
            command += ["--record-session", str(path)]
        if capture:
            command += ["--capture", str(folder / "mirror.png")]
        result = subprocess.run(command, env=environment, cwd=folder,
                                capture_output=True, text=True, timeout=45,
                                creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        rows = [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines()] if path.exists() else []
        reports = [json.loads(line) for line in result.stdout.splitlines() if line.startswith("{")]
        return result, rows, reports

    def folder(self):
        (ROOT / "temp").mkdir(exist_ok=True)
        return tempfile.TemporaryDirectory(dir=ROOT / "temp", prefix="p0-test-")

    def test_manifest_poses_and_effective_state(self):
        with self.folder() as folder:
            result, rows, reports = self.launch(folder)
            self.assertEqual(result.returncode, 0, result.stderr)
            header = rows[0]
            self.assertEqual(header["condition"], "observe_only")
            self.assertTrue(header["tracking"]["hand_trackers_ready"])
            self.assertFalse(header["hands_rendered"])
            self.assertEqual(header["configuration_sha256"],
                             hashlib.sha256((ROOT / "visual-config.json").read_bytes()).hexdigest())
            samples = [r for r in rows if r["type"] == "observation"]
            self.assertGreaterEqual(len(samples), 1)
            for row in samples:
                xr = row["xr"]
                self.assertEqual(xr["reference_space"], "LOCAL")
                self.assertGreater(xr["xr_time_ns"], 0)
                self.assertGreater(xr["received_qpc"], 0)
                self.assertGreater(xr["qpc_frequency"], 0)
                self.assertEqual(len(xr["hands"][0]["joints"]), 26)
                self.assertEqual(xr["head"]["position_m"], [0.0, 0.0, 0.0])
                state = row["environment"]
                self.assertEqual(state["scene_seconds"], 152)
                self.assertGreater(abs(state["camera_virtual_z_cm"]), 1000)
                self.assertNotEqual(state["base"]["palette"], state["directed"]["palette"])
                self.assertEqual(state["master_gain"], 1)
                self.assertIsNone(state["semantic_modulation"])
                self.assertTrue(state["render_submitted"])
            end = rows[-1]
            self.assertTrue(end["complete"])
            self.assertEqual(end["accepted"], len(rows) - 2)
            self.assertEqual(end["dropped"], 0)
            self.assertTrue(next(r for r in reports if r["type"] == "recording_result")["complete"])
            inspected = inspect(Path(folder) / "session.jsonl")
            self.assertTrue(inspected["valid_record"], inspected["errors"])

    def test_hand_loss_is_logged_even_between_publish_ticks(self):
        with self.folder() as folder:
            result, rows, _ = self.launch(folder, mode="hands-lost")
            self.assertEqual(result.returncode, 0, result.stderr)
            samples = [r for r in rows if r["type"] == "observation"]
            lost = [r for r in samples if r["xr"]["hands"][0]["status"] == "inactive"]
            self.assertTrue(lost)
            for row in lost:
                self.assertIsNone(row["motion"]["hand_linear_m_s"][0])
                self.assertIsNone(row["xr"]["hands"][0]["joints"][0]["position_m"])
            self.assertTrue(any(r["xr"]["hands"][0]["active"] for r in samples if r["xr"]["sequence"] > 3))

    def test_xr_time_can_be_mapped_to_qpc_when_available(self):
        with self.folder() as folder:
            result, rows, _ = self.launch(folder, mode="time-conversion")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue(rows[0]["tracking"]["xr_to_qpc_available"])
            for row in rows:
                if row["type"] == "observation":
                    xr = row["xr"]
                    self.assertEqual(xr["clock_mapping"], "xr_to_qpc")
                    expected = int(xr["xr_time_ns"] / 1e9 * xr["qpc_frequency"])
                    self.assertEqual(xr["predicted_qpc"], expected)

    def test_unavailable_hands_have_explicit_diagnostics(self):
        for mode, reason in (("no-hands", "extension_unavailable"),
                             ("no-hand-support", "system_not_supported"),
                             ("hand-create-failure", "tracker_creation_failed")):
            with self.subTest(mode=mode), self.folder() as folder:
                result, rows, _ = self.launch(folder, mode=mode)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(rows[0]["tracking"]["hand_tracking_reason"], reason)
                self.assertFalse(rows[0]["tracking"]["hand_trackers_ready"])
                for row in rows:
                    if row["type"] == "observation":
                        self.assertEqual(row["xr"]["hands"][0]["status"], "unavailable")

    def test_hand_locate_errors_do_not_become_immobility(self):
        with self.folder() as folder:
            result, rows, _ = self.launch(folder, mode="hand-locate-failure")
            self.assertEqual(result.returncode, 0, result.stderr)
            for row in rows:
                if row["type"] == "observation":
                    self.assertEqual(row["xr"]["hands"][0]["status"], "error")
                    self.assertLess(row["xr"]["hands"][0]["locate_result"], 0)
                    self.assertIsNone(row["motion"]["hand_linear_m_s"][0])

    def test_partial_head_validity_is_independent(self):
        for mode, present, missing in (("head-position-only", "position_m", "orientation_xyzw"),
                                       ("head-orientation-only", "orientation_xyzw", "position_m")):
            with self.subTest(mode=mode), self.folder() as folder:
                result, rows, _ = self.launch(folder, mode=mode)
                self.assertEqual(result.returncode, 0, result.stderr)
                for row in rows:
                    if row["type"] == "observation":
                        self.assertIsNotNone(row["xr"]["head"][present])
                        self.assertIsNone(row["xr"]["head"][missing])

    def test_recenter_takes_effect_at_requested_xr_time(self):
        with self.folder() as folder:
            result, rows, _ = self.launch(folder, mode="recenter")
            self.assertEqual(result.returncode, 0, result.stderr)
            samples = [r for r in rows if r["type"] == "observation"]
            newer = [r for r in samples if r["xr"]["origin_epoch"] == 1]
            self.assertTrue(newer)
            self.assertGreaterEqual(newer[0]["xr"]["xr_time_ns"], 4 * 11111111)
            self.assertEqual(newer[0]["xr"]["reference_change_time_xr"], 4 * 11111111)
            self.assertIsNone(newer[0]["motion"]["head_linear_m_s"])
            inspected = inspect(Path(folder) / "session.jsonl")
            self.assertTrue(inspected["valid_record"], inspected["errors"])

    def test_nonrendering_and_invalid_frames_are_retained(self):
        with self.folder() as folder:
            result, rows, _ = self.launch(folder, mode="skip-frames")
            self.assertEqual(result.returncode, 0, result.stderr)
            statuses = {r["xr"]["frame_status"] for r in rows if r["type"] == "observation"}
            self.assertIn("not_rendering", statuses)
            self.assertIn("tracking_invalid", statuses)
            self.assertIn("located", statuses)

    def test_aborted_runtime_produces_incomplete_record(self):
        with self.folder() as folder:
            result, rows, _ = self.launch(folder, mode="loss-session")
            self.assertNotEqual(result.returncode, 0)
            self.assertFalse(rows[-1]["complete"])
            self.assertFalse(rows[-1]["normal_exit"])

    def test_recording_does_not_change_fixed_scene_image(self):
        with self.folder() as first, self.folder() as second:
            baseline, _, _ = self.launch(first, record=False, capture=True)
            observed, _, _ = self.launch(second, capture=True)
            self.assertEqual(baseline.returncode, 0, baseline.stderr)
            self.assertEqual(observed.returncode, 0, observed.stderr)
            self.assertEqual((Path(first) / "mirror.png").read_bytes(),
                             (Path(second) / "mirror.png").read_bytes())

    def test_desktop_recording_does_not_claim_xr_data(self):
        with self.folder() as folder:
            result, rows, _ = self.launch(folder, vr=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(rows[0]["mode"], "desktop")
            self.assertIsNone(rows[0]["tracking"])
            self.assertTrue(all(r["xr"] is None for r in rows if r["type"] == "observation"))

    def test_existing_recording_is_preserved(self):
        with self.folder() as folder:
            path = Path(folder) / "session.jsonl"
            path.write_text('{"original":true}\n')
            result, _, _ = self.launch(folder)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(path.read_text(), '{"original":true}\n')

    def test_inspector_rejects_truncated_or_malformed_input(self):
        for content in ('[]\n', '{"type":"session_manifest","schema_version":1}\n', '{'):
            with self.subTest(content=content), self.folder() as folder:
                path = Path(folder) / "bad.jsonl"
                path.write_text(content)
                result = subprocess.run([sys.executable, str(ROOT / "tools/inspect_session.py"), str(path)],
                                        capture_output=True, text=True, timeout=10)
                self.assertEqual(result.returncode, 1)
                self.assertFalse(json.loads(result.stdout)["valid_record"])


if __name__ == "__main__":
    unittest.main()
