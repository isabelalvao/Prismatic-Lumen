"""Converte capturas reais do renderer em excertos GIF para direção visual."""
import json
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
CAPTURES = ROOT / "plan/art-direction-v1/captures"


def encode(folder: str, output: str, count: int, fps: int) -> dict:
    paths = sorted((CAPTURES / folder).glob("*.png"))
    if len(paths) != count:
        raise ValueError(f"Esperados {count} fotogramas: {folder}")
    with Image.open(paths[count // 2]) as source:
        palette = source.convert("RGB").convert("P", palette=Image.Palette.ADAPTIVE, colors=256)
    frames = []
    for path in paths:
        with Image.open(path) as source:
            if source.size != (640, 360):
                raise ValueError(f"Dimensões inesperadas: {path}")
            frames.append(source.convert("RGB").quantize(palette=palette, dither=Image.Dither.NONE))
    target = CAPTURES / output
    # GIF usa centésimos; alterna80/90ms para conservar12fps sem acelerar a animação.
    durations = [(round((i + 1) * 100 / fps) - round(i * 100 / fps)) * 10 for i in range(count)]
    # Excerto reproduzido uma vez; a sua junção não representa o ciclo do cenário.
    frames[0].save(target, save_all=True, append_images=frames[1:], duration=durations,
                   disposal=2, optimize=False)
    with Image.open(target) as animation:
        elapsed = 0
        for i in range(animation.n_frames):
            animation.seek(i)
            elapsed += animation.info["duration"]
        if elapsed != round(count / fps * 1000):
            raise ValueError(f"Duração incorreta do GIF: {elapsed} ms")
    return {"file": str(target), "frames": count, "fps": fps, "audio": False,
            "camera": "fixed to isolate animation", "source": "actual D3D11 renderer captures"}


if __name__ == "__main__":
    report = [encode("intro-frames", "intro-motion.gif", 144, 12),
              encode("pyramid-frames", "pyramid-lighting.gif", 120, 10)]
    (CAPTURES / "previews.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps(report, indent=2))
