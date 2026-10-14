"""Compara capturas da afinação 0.12.0 produzidas pela aplicação real."""
import json

from PIL import Image, ImageStat

import validate_scene_refinement as capture_tools

OUTPUT = capture_tools.ROOT / "plan/scene-refinement-v2/captures"


def main() -> None:
    capture_tools.OUTPUT = OUTPUT
    OUTPUT.mkdir(parents=True, exist_ok=True)
    phases = (174, 180, 250, 284, 290, 298, 306, 330, 360, 408, 412, 418, 424.1821315192744)
    images = {seconds: capture_tools.capture(seconds, f"phase-{seconds}") for seconds in phases}
    comparisons = (
        (174, "cones-old-density", {"cones": {"density_multiplier": 1}}),
        (174, "cones-one-palette", {"cones": {"palette_variants": 1}}),
        (180, "rocks-larger", {"crystals": {"minimum_height_m": 1.5, "maximum_height_m": 2}}),
        (250, "artifacts-dense", {"extension": {"artifact_density_multiplier": 1}}),
        (250, "artifacts-no-drift", {"extension": {"artifact_drift_m": 0, "artifact_rotation_degrees_s": 0}}),
        (330, "rings-no-pulses", {"extension": {"ring_pulse_gain": 0}}),
        (412, "fade-later", {"timeline": {"final_fade_seconds": 4}}),
    )
    for seconds, label, override in comparisons:
        alternate = capture_tools.capture(seconds, label, override)
        assert not capture_tools.same(images[seconds], alternate), label
    luminance = {}
    for seconds in (408, 412, 418, 424.1821315192744):
        with Image.open(images[seconds]) as frame:
            luminance[seconds] = ImageStat.Stat(frame.convert("L")).mean[0]
            if seconds > 424:
                assert frame.convert("RGB").getbbox() is None
    assert luminance[418] < luminance[408]
    assert luminance[424.1821315192744] == 0
    report = {"status": "passed", "phases": list(phases), "comparisons": len(comparisons),
              "final_mean_luminance": luminance, "black_at_soundtrack_end": True}
    (OUTPUT.parent / "gpu-validation.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print("GPU: densidade, movimento, rochas, paletas, pulsação e saída para preto validados.")


if __name__ == "__main__":
    main()
