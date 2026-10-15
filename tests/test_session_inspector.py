"""Casos de cobertura e integridade com dados controlados, sem headset."""
import copy
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from inspect_session import angular_speed, derive, inspect
from session_metrics import summary


class InspectorTests(unittest.TestCase):
    def setUp(self):
        fixture = [json.loads(line) for line in
                   (ROOT / "plan/hand-observation-p0/example-session.jsonl").read_text().splitlines()]
        self.manifest = fixture[0]
        self.template = next(row for row in fixture if row["type"] == "observation")
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.path = Path(self.folder.name) / "hand-tracking-01.jsonl"

    def samples(self, times=(0, .05, .1, .15, .2)):
        rows = []
        for index, time in enumerate(times):
            event = copy.deepcopy(self.template)
            event["received_monotonic_ns"] = int((10 + time) * 1e9)
            xr = event["xr"]
            xr["received_monotonic_ns"] = event["received_monotonic_ns"]
            xr["xr_time_ns"] = int((1 + time) * 1e9)
            xr["sequence"] = index + 1
            xr["head"]["position_m"] = [time * 2, 0, 0]
            for hand in xr["hands"]:
                for joint in hand["joints"]:
                    joint["position_m"] = [time, 0, 0]
            rows.append(event)
        return rows

    def write(self, observations, replay=True):
        previous = None
        for row in observations:
            if replay:
                row["motion"] = derive(previous, row["xr"])
            previous = row["xr"]
        footer = {"type": "session_end", "accepted": len(observations),
                  "complete": True, "normal_exit": True, "dropped": 0, "write_failed": False}
        self.path.write_text("\n".join(json.dumps(row) for row in
                            [self.manifest, *observations, footer]) + "\n", encoding="utf-8")
        return inspect(self.path)

    def test_known_speed_and_initial_null(self):
        report = self.write(self.samples())
        self.assertTrue(report["valid_record"], report["errors"])
        metrics = report["tracking_analysis"]
        self.assertAlmostEqual(metrics["effective_publish_hz"], 20)
        left = metrics["sources"]["left"]
        self.assertEqual(left["usable_position_sample_percent"], 100)
        self.assertAlmostEqual(left["estimated_usable_seconds"], .2)
        self.assertAlmostEqual(left["linear_speed_m_s"]["median"], 1)
        self.assertEqual(left["null_speed_samples"], 1)
        self.assertAlmostEqual(metrics["sources"]["head"]["linear_speed_m_s"]["max"], 2)

    def test_loss_recovery_and_distinct_hands(self):
        rows = self.samples()
        for row in rows[1:3]:
            hand = row["xr"]["hands"][0]
            hand["active"] = False
            hand["status"] = "inactive"
            for joint in hand["joints"]:
                joint["position_valid"] = joint["position_tracked"] = False
                joint["position_m"] = None
        report = self.write(rows)
        self.assertTrue(report["valid_record"])
        left = report["tracking_analysis"]["sources"]["left"]
        self.assertEqual(left["active_sample_percent"], 60)
        self.assertEqual(left["recoveries"], 1)
        self.assertAlmostEqual(left["estimated_usable_percent"], 50)
        interval = left["unusable_position_intervals"][0]
        self.assertAlmostEqual(interval["start_seconds"], .05)
        self.assertAlmostEqual(interval["end_seconds"], .15)
        self.assertEqual(report["tracking_analysis"]["sources"]["right"]["usable_position_sample_percent"], 100)

    def test_gaps_are_unknown_and_reset_motion(self):
        report = self.write(self.samples((0, .05, 1.05, 1.1)))
        left = report["tracking_analysis"]["sources"]["left"]
        self.assertAlmostEqual(left["known_interval_seconds"], .1)
        self.assertAlmostEqual(left["unobserved_gap_seconds"], 1)
        self.assertEqual(left["null_speed_samples"], 2)

    def test_recenter_does_not_become_velocity_spike(self):
        rows = self.samples()
        for row in rows[2:]:
            row["xr"]["origin_epoch"] = 1
            row["xr"]["head"]["position_m"][0] += 100
        report = self.write(rows)
        self.assertEqual(report["counts"]["origin_changes"], 1)
        self.assertAlmostEqual(report["tracking_analysis"]["sources"]["head"]["linear_speed_m_s"]["max"], 2)

    def test_unknown_gap_does_not_claim_continuous_loss_or_recovery(self):
        rows = self.samples((0, 10))
        rows[0]["xr"]["hands"][0]["active"] = False
        rows[0]["xr"]["hands"][0]["status"] = "inactive"
        source = self.write(rows)["tracking_analysis"]["sources"]["left"]
        self.assertEqual(source["recoveries"], 0)
        self.assertEqual(source["unobserved_gap_seconds"], 10)
        self.assertEqual(source["unusable_position_intervals"],
                         [{"start_seconds": 0, "end_seconds": 0,
                           "recovered": False, "ended_by_gap": True}])

    def test_joint_loss_and_flags_are_independent(self):
        rows = self.samples()
        for row in rows:
            row["xr"]["hands"][0]["joints"][5]["position_tracked"] = False
            row["xr"]["head"]["orientation_tracked"] = False
        report = self.write(rows)
        left = report["tracking_analysis"]["sources"]["left"]
        self.assertEqual(left["joint_position_valid_sample_percentages"][5], 100)
        self.assertEqual(left["joint_position_tracked_sample_percentages"][5], 0)
        self.assertEqual(left["usable_position_sample_percent"], 100)
        head = report["tracking_analysis"]["sources"]["head"]
        self.assertEqual(head["flag_sample_percentages"]["orientation_tracked"], 0)
        self.assertEqual(head["usable_position_sample_percent"], 100)

    def test_qpc_statistics_are_not_physical_latency(self):
        rows = self.samples()
        for row in rows:
            row["xr"].update(predicted_qpc=1200, received_qpc=1000, qpc_frequency=10000)
        metrics = self.write(rows)["tracking_analysis"]
        self.assertEqual(metrics["xr_to_qpc_sample_percent"], 100)
        self.assertEqual(metrics["predicted_time_minus_receipt_ms"]["median"], 20)
        self.assertIn("not physical", metrics["limitations"][-1])

    def test_quaternion_normalization_avoids_overflow_and_sign_artifact(self):
        pose = copy.deepcopy(self.template["xr"]["head"])
        pose["orientation_xyzw"] = [0, 0, 0, 1e200]
        other = copy.deepcopy(pose)
        other["orientation_xyzw"] = [0, 0, 0, -1e200]
        self.assertEqual(angular_speed(pose, other, .05), 0)

    def test_summary_interpolation_avoids_overflow(self):
        self.assertEqual(summary([-1e308, 1e308])["median"], 0)
        self.assertLess(summary([-1e308, 1e308])["p95"], 1e308)

    def test_numeric_corruption_is_rejected(self):
        rows = self.samples()
        rows[0]["xr"]["hands"][1]["joints"][2]["position_m"] = None
        rows[1]["xr"]["head"]["orientation_xyzw"] = [0, 0, 0, 0]
        report = self.write(rows)
        self.assertFalse(report["valid_record"])
        self.assertEqual(len(report["errors"]), 2)

    def test_motion_tampering_is_rejected(self):
        rows = self.samples()
        self.write(rows)
        rows[2]["motion"]["hand_linear_m_s"][0] = 99
        self.assertFalse(self.write(rows, replay=False)["valid_record"])

    def test_nested_acquisition_clock_regression_is_rejected(self):
        rows = self.samples()
        rows[2]["xr"]["received_monotonic_ns"] = 1
        self.assertFalse(self.write(rows)["valid_record"])

    def test_invalid_clock_fields_and_nonfinite_json_are_rejected(self):
        for field, value in (("query_duration_ns", -1), ("qpc_frequency", 0),
                             ("received_qpc", True), ("query_duration_ns", float("inf"))):
            with self.subTest(field=field, value=value):
                rows = self.samples()
                rows[0]["xr"][field] = value
                with self.assertRaises(ValueError):
                    self.write(rows)
        self.write(self.samples())
        content = self.path.read_text()
        self.path.write_text(content.replace('"scene_seconds": 152.0', '"scene_seconds": 1e999'))
        with self.assertRaises(ValueError):
            inspect(self.path)

    def test_identity_and_truncated_record(self):
        self.write(self.samples())
        self.assertTrue(inspect(self.path, self.manifest["executable_sha256"])["valid_record"])
        self.assertFalse(inspect(self.path, "wrong")["valid_record"])
        self.path.write_text("\n".join(self.path.read_text().splitlines()[:-1]) + "\n")
        self.assertFalse(inspect(self.path)["valid_record"])

    def test_cli_output_and_source_preservation(self):
        self.write(self.samples())
        original = self.path.read_bytes()
        output = self.path.with_suffix(".analysis.json")
        command = [sys.executable, str(ROOT / "tools/inspect_session.py"), str(self.path), "--output"]
        result = subprocess.run([*command, str(output)], capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertTrue(json.loads(output.read_text())["valid_record"])
        result = subprocess.run([*command, str(self.path)], capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(self.path.read_bytes(), original)


if __name__ == "__main__":
    unittest.main()
