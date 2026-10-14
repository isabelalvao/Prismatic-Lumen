"""Rejeição de propriedades truncadas ou que atravessem os limites de um export."""
import struct
import unittest
from tools.inspect_fantasynth_assets import Reader, properties


class InspectorTests(unittest.TestCase):
    names = ["None", "Parameter", "NameProperty", "Value", "FloatProperty"]

    def tag(self, kind: int, size: int, value: bytes) -> bytes:
        return struct.pack("<iiiiiiB", 1, 0, kind, 0, size, 0, 0) + value

    def test_name_size_must_be_eight_bytes(self):
        data = self.tag(2, 4, struct.pack("<ii", 3, 0)) + struct.pack("<ii", 0, 0)
        with self.assertRaises(ValueError):
            properties(Reader(data), self.names, len(data))

    def test_value_cannot_cross_export_boundary(self):
        data = self.tag(4, 4, struct.pack("<f", 1.0)) + struct.pack("<ii", 0, 0)
        with self.assertRaises(ValueError):
            properties(Reader(data), self.names, 27)

    def test_valid_name_property_and_terminator(self):
        data = self.tag(2, 8, struct.pack("<ii", 3, 0)) + struct.pack("<ii", 0, 0)
        result = properties(Reader(data), self.names, len(data))
        self.assertEqual(result[0]["value"], "Value")

    def test_float_size_is_not_silently_ignored(self):
        data = self.tag(4, 8, b"\x00" * 8) + struct.pack("<ii", 0, 0)
        with self.assertRaises(ValueError):
            properties(Reader(data), self.names, len(data))


if __name__ == "__main__":
    unittest.main()
