"""Guião de deslocamento dos anéis; runtime simulado, sem avaliação musical no Quest."""
import json
import unittest
import test_motifs as support


class RingTravelTests(unittest.TestCase):
    def test_crescendo_and_coda_are_visible_in_session_state_and_pause(self):
        values = []
        for time in (306, 334, 400, 420):
            with self.subTest(time=time), support.MotifTests().folder() as folder:
                result, rows = support.MotifTests().launch(folder, "--silent", "--fixed-time", str(time),
                    "--paused", "--frames", "6", enabled=False)
                self.assertEqual(result.returncode, 0, result.stderr)
                observations = [r["environment"] for r in rows if r["type"] == "observation"]
                self.assertTrue(observations)
                multiplier = observations[-1]["ring_travel_speed_multiplier"]
                self.assertTrue(all(r["ring_travel_speed_multiplier"] == multiplier for r in observations))
                self.assertTrue(all(r["scene_seconds"] == time and r["paused"] for r in observations))
                self.assertEqual(rows[0]["ring_travel_profile"], "master-crescendo-coda-1")
                values.append(multiplier)
        self.assertEqual(values[:2], [1, 3])
        self.assertGreater(values[1], values[2])
        self.assertGreater(values[2], values[3])
        self.assertEqual(values[3], 0.25)

    def test_sequence_keeps_original_movement(self):
        with support.MotifTests().folder() as folder:
            result, rows = support.MotifTests().launch(folder, "--silent", "--scene", "sequence",
                "--fixed-time", "334", "--paused", "--frames", "4", enabled=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            observations = [r["environment"] for r in rows if r["type"] == "observation"]
            self.assertTrue(observations)
            self.assertTrue(all(r["ring_travel_speed_multiplier"] == 1 for r in observations))
            performance = next(json.loads(line) for line in result.stdout.splitlines()
                               if line.startswith('{') and json.loads(line).get("type") == "performance")
            self.assertEqual(performance["ring_travel_speed_multiplier"], 1)


if __name__ == "__main__":
    unittest.main()
