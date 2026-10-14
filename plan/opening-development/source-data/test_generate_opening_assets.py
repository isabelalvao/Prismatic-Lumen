"""Testes do contrato binário e da mudança de base dos assets iniciais."""
from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.dont_write_bytecode = True
SCRIPT = Path(__file__).resolve().parents[3] / "tools/generate_opening_assets.py"
SPEC = importlib.util.spec_from_file_location("opening_asset_generator", SCRIPT)
GENERATOR = None
if SCRIPT.exists() and SPEC and SPEC.loader:
    GENERATOR = importlib.util.module_from_spec(SPEC)
    SPEC.loader.exec_module(GENERATOR)


class OpeningAssetTests(unittest.TestCase):
    def setUp(self) -> None:
        if GENERATOR is None:
            self.fail("O gerador e o contrato .fmesh ainda não foram implementados")

    def fixture(self, directory: Path, bad_index: bool = False) -> Path:
        # Duas primitivas reais: preservam escala, normais e coordenadas UV.
        payload = bytearray()
        views = []
        accessors = []

        def add(values: list[tuple], kind: str, component: int) -> int:
            fmt = "H" if component == 5123 else "f"
            start = len(payload)
            for row in values:
                payload.extend(struct.pack("<" + fmt * len(row), *row))
            views.append({"buffer": 0, "byteOffset": start,
                          "byteLength": len(payload) - start})
            accessors.append({"bufferView": len(views) - 1,
                              "componentType": component, "type": kind,
                              "count": len(values)})
            return len(accessors) - 1

        primitives = []
        for offset in (0.0, 2.0):
            positions = [(offset, 0.0, 0.0), (offset + 0.5, 0.0, 0.0),
                         (offset, 0.25, 0.0)]
            attributes = {
                "POSITION": add(positions, "VEC3", 5126),
                "NORMAL": add([(0.0, 0.6, 0.8)] * 3, "VEC3", 5126),
                "TEXCOORD_0": add([(0.25, 0.75)] * 3, "VEC2", 5126),
            }
            indices = [(0,), (1,), (9 if bad_index else 2,)]
            primitives.append({"attributes": attributes,
                               "indices": add(indices, "SCALAR", 5123)})
        data = {"asset": {"version": "2.0"},
                "buffers": [{"uri": "fixture.bin", "byteLength": len(payload)}],
                "bufferViews": views, "accessors": accessors,
                "meshes": [{"primitives": primitives}]}
        path = directory / "fixture.gltf"
        path.write_text(json.dumps(data), encoding="utf-8")
        (directory / "fixture.bin").write_bytes(payload)
        return path

    def test_binary_preserves_geometry_and_offsets_second_primitive(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = self.fixture(Path(directory))
            binary = GENERATOR.convert_mesh(path)
        magic, vertices, indices = struct.unpack_from("<8sII", binary)
        self.assertEqual((magic, vertices, indices), (b"FNTMESH1", 6, 6))
        self.assertEqual(len(binary), 16 + 6 * 32 + 6 * 2)
        row = struct.unpack_from("<8f", binary, 16 + 3 * 32)
        self.assertEqual(row[:3], (200.0, 0.0, 0.0))
        self.assertAlmostEqual(row[4], 0.6, places=6)
        self.assertAlmostEqual(row[5], 0.8, places=6)
        self.assertEqual(row[6:], (0.25, 0.75))
        self.assertEqual(struct.unpack_from("<6H", binary, 16 + 6 * 32),
                         (0, 1, 2, 3, 4, 5))

    def test_rejects_index_outside_source_primitive(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = self.fixture(Path(directory), bad_index=True)
            with self.assertRaisesRegex(ValueError, "índice"):
                GENERATOR.convert_mesh(path)

    def test_rejects_truncated_external_buffer(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            path = self.fixture(Path(directory))
            data = path.with_suffix(".bin").read_bytes()
            path.with_suffix(".bin").write_bytes(data[:-1])
            with self.assertRaisesRegex(ValueError, "buffer"):
                GENERATOR.convert_mesh(path)

    def test_matrix_conjugates_axis_swap_without_rescaling_authored_row(self) -> None:
        row = {"Location": [10.0, 20.0, 30.0],
               "RotationX": [0.0, 1.0, 0.0],
               "RotationY": [-1.0, 0.0, 0.0],
               "RotationZ": [0.0, 0.0, 1.0], "Scale": [2.0, 3.0, 4.0]}
        matrix = GENERATOR.scatter_matrix(row, [100.0, 200.0, 300.0])
        self.assertEqual(matrix, [0.0, 0.0, 2.0, 0.0,
                                  0.0, 4.0, 0.0, 0.0,
                                  -3.0, 0.0, 0.0, 0.0,
                                  110.0, 330.0, 220.0, 1.0])


if __name__ == "__main__":
    unittest.main()
