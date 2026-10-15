"""Lápis de luz com pose XR sintética; sem prova percetiva/física no Quest."""
import subprocess
import unittest
from pathlib import Path
import test_motifs as support

class LightPencilTests(unittest.TestCase):
    def launch(self, folder, mode="pencil-write", enabled=True, frames=40):
        return support.MotifTests().launch(folder, "--silent", "--fixed-time", "54", "--paused", "--frames", str(frames),
            *(["--hand-light-pencil"] if enabled else []), enabled=False, mode=mode, capture=True)

    def test_index_only_writes_per_hand_in_pause_and_changes_image(self):
        with support.MotifTests().folder() as on, support.MotifTests().folder() as off:
            result, rows = self.launch(on)
            baseline, original = self.launch(off, enabled=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(baseline.returncode, 0, baseline.stderr)
            events = [r for r in rows if r["type"] == "hand_light_pencil"]
            self.assertEqual(len(events), 2)
            self.assertTrue(all(r["event"] == "started" for r in events))
            self.assertEqual({r["hand"] for r in events}, {"left", "right"})
            self.assertFalse(any(r["type"] == "hand_light_pencil" for r in original))
            self.assertTrue(rows[0]["hand_light_pencil"]["enabled"])
            self.assertEqual(rows[0]["condition"], "hand_effects")
            self.assertNotEqual((Path(on) / "mirror.png").read_bytes(), (Path(off) / "mirror.png").read_bytes())
            self.assertTrue(all(r["environment"]["scene_seconds"] == 54 for r in rows if r["type"] == "observation"))
            self.assertTrue(rows[-1]["complete"])

    def test_two_extended_fingers_open_hand_and_missing_tracking_do_not_write(self):
        for mode in ("pencil-two-fingers", "hand-skeleton", "no-hands", "hands-inactive"):
            with self.subTest(mode=mode), support.MotifTests().folder() as folder:
                result, rows = self.launch(folder, mode=mode)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertFalse(any(r["type"] == "hand_light_pencil" and r["event"] == "started" for r in rows))

    def test_pose_exit_fades_to_baseline_and_tracking_loss_clears_immediately(self):
        for mode, frames, event in (("pencil-stop", 100, "ended"), ("pencil-lost", 40, "reset")):
            with self.subTest(mode=mode), support.MotifTests().folder() as on, support.MotifTests().folder() as off:
                result, rows = self.launch(on, mode=mode, frames=frames)
                baseline, _ = self.launch(off, mode=mode, enabled=False, frames=frames)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(baseline.returncode, 0, baseline.stderr)
                self.assertTrue(any(r["type"] == "hand_light_pencil" and r["event"] == event for r in rows))
                self.assertEqual((Path(on) / "mirror.png").read_bytes(), (Path(off) / "mirror.png").read_bytes())

    def test_requires_vr(self):
        result = subprocess.run([str(support.EXE), "--hand-light-pencil"], cwd=support.ROOT, capture_output=True,
            text=True, timeout=10, creationflags=subprocess.CREATE_NO_WINDOW)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("requires --vr", result.stderr)

    def test_natural_pose_and_brief_auxiliary_occlusion_keep_one_stroke(self):
        for mode in ("pencil-natural", "pencil-chatter"):
            with self.subTest(mode=mode), support.MotifTests().folder() as folder:
                result, rows = self.launch(folder, mode=mode, frames=70)
                self.assertEqual(result.returncode, 0, result.stderr)
                events = [r for r in rows if r["type"] == "hand_light_pencil"]
                self.assertEqual(len(events), 2)
                self.assertTrue(all(r["event"] == "started" for r in events))
                self.assertEqual(rows[0]["hand_light_pencil"]["version"], "index-only-trail-2")

    def test_snap_and_pencil_coexist_without_triggering_each_other(self):
        for mode, expected, absent in (("pencil-write", "hand_light_pencil", "hand_snap"),
                                       ("snap-gesture", "hand_snap", "hand_light_pencil")):
            with self.subTest(mode=mode), support.MotifTests().folder() as folder:
                result, rows = support.MotifTests().launch(folder, "--silent", "--fixed-time", "54", "--paused",
                    "--frames", "24", "--hand-snap", "--hand-light-pencil", enabled=False, mode=mode)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertTrue(rows[0]["hand_snap"]["enabled"])
                self.assertTrue(rows[0]["hand_light_pencil"]["enabled"])
                self.assertTrue(any(r["type"] == expected for r in rows))
                self.assertFalse(any(r["type"] == absent for r in rows))

if __name__ == "__main__":
    unittest.main()
