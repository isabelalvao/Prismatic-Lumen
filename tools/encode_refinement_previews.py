"""Cria excertos com o áudio correspondente às capturas reais da versão 0.12.0."""
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "plan/scene-refinement-v2"
FFMPEG = ROOT / "tools/vendor/video-analysis-runtime/imageio_ffmpeg/binaries/ffmpeg-win-x86_64-v7.1.exe"


def main() -> None:
    report = []
    for name, start, fps, count in (("landscape", 174, 12, 144), ("artifacts", 250, 12, 144),
                                    ("rings", 330, 24, 288), ("ending", 408, 12, 196)):
        folder = OUTPUT / "previews" / f"{name}-frames"
        assert len(list(folder.glob("*.png"))) == count
        target = OUTPUT / f"{name}.mp4"
        duration = count / fps
        command = [str(FFMPEG), "-y", "-v", "error", "-framerate", str(fps),
                   "-i", str(folder / "%04d.png"), "-ss", str(start),
                   "-i", str(ROOT / "assets/audio/ChezNous_MASTER.wav"),
                   "-map", "0:v:0", "-map", "1:a:0", "-af", f"apad=whole_dur={duration}",
                   "-t", str(duration), "-c:v", "libx264", "-crf", "18", "-pix_fmt", "yuv420p",
                   "-c:a", "aac", "-b:a", "192k", "-movflags", "+faststart", str(target)]
        subprocess.run(command, check=True, timeout=60)
        subprocess.run([str(FFMPEG), "-v", "error", "-i", str(target), "-f", "null", "NUL"],
                       check=True, timeout=30)
        report.append({"file": str(target), "start_seconds": start, "frames": count, "fps": fps,
                       "duration_seconds": duration, "audio": "matching excerpt of original MASTER",
                       "camera": "fixed" if name in ("landscape", "artifacts") else "moving",
                       "source": "actual D3D11 renderer captures", "decode_validation": "passed"})
    (OUTPUT / "previews.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print("Quatro excertos com áudio codificados e descodificados sem erros.")


if __name__ == "__main__":
    main()
