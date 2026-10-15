"""Gesto sintético e efeito D3D11; não prova deteção física de estalos no Quest."""
import json
import subprocess
import unittest
from pathlib import Path
import test_motifs as support

class HandSnapTests(unittest.TestCase):
    def launch(self, folder, mode="snap-gesture", enabled=True, frames=16):
        return support.MotifTests().launch(folder, "--silent", "--fixed-time", "54", "--paused", "--frames", str(frames),
                                  *(["--hand-snap"] if enabled else []), enabled=False, mode=mode, capture=True)

    def test_release_once_per_hand_while_music_paused_and_image_changes(self):
        with support.MotifTests().folder() as on, support.MotifTests().folder() as off:
            active, rows = self.launch(on)
            inactive, original = self.launch(off, enabled=False)
            self.assertEqual(active.returncode, 0, active.stderr)
            self.assertEqual(inactive.returncode, 0, inactive.stderr)
            events = [r for r in rows if r["type"] == "hand_snap"]
            self.assertEqual(len(events), 2)
            self.assertEqual({r["hand"] for r in events}, {"left", "right"})
            self.assertEqual(rows[0]["condition"], "hand_snap")
            self.assertTrue(rows[0]["hand_snap"]["enabled"])
            self.assertFalse(any(r["type"] == "hand_snap" for r in original))
            self.assertNotEqual((Path(on) / "mirror.png").read_bytes(), (Path(off) / "mirror.png").read_bytes())
            for row in rows:
                if row["type"] == "observation":
                    self.assertEqual(row["environment"]["scene_seconds"], 54)
            self.assertTrue(rows[-1]["complete"])

    def test_hold_lost_tracking_and_no_extension_do_not_trigger(self):
        for mode in ("snap-hold", "snap-lost", "no-hands", "hand-skeleton", "hands-inactive"):
            with self.subTest(mode=mode), support.MotifTests().folder() as folder:
                result, rows = self.launch(folder, mode=mode)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertFalse(any(r["type"] == "hand_snap" for r in rows))

    def test_particles_expire_without_repeating_and_disabled_matches_after_expiry(self):
        with support.MotifTests().folder() as on, support.MotifTests().folder() as off:
            active, rows = self.launch(on, frames=65)
            inactive, _ = self.launch(off, enabled=False, frames=65)
            self.assertEqual(active.returncode, 0, active.stderr)
            self.assertEqual(inactive.returncode, 0, inactive.stderr)
            self.assertEqual(len([r for r in rows if r["type"] == "hand_snap"]), 2)
            self.assertEqual((Path(on) / "mirror.png").read_bytes(), (Path(off) / "mirror.png").read_bytes())

    def test_option_requires_vr(self):
        result = subprocess.run([str(support.EXE), "--hand-snap"], cwd=support.ROOT, capture_output=True, text=True, timeout=10,
                                creationflags=subprocess.CREATE_NO_WINDOW)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("requires --vr", result.stderr)

if __name__ == "__main__":
    unittest.main()
