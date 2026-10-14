"""Testes do cliente com ligações TCP locais reais."""

import importlib.util
import json
import socket
import threading
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location(
    "send_command", Path(__file__).resolve().parents[1] / "tools" / "send_command.py"
)
client = importlib.util.module_from_spec(spec)
spec.loader.exec_module(client)


class ClientTests(unittest.TestCase):
    def test_collects_fragmented_receipt(self):
        received = []
        with socket.socket() as listener:
            listener.bind(("127.0.0.1", 0))
            listener.listen()
            port = listener.getsockname()[1]

            def serve():
                connection, _ = listener.accept()
                with connection:
                    raw = b""
                    while not raw.endswith(b"\n"):
                        raw += connection.recv(128)
                    received.append(json.loads(raw))
                    connection.sendall(b'{"status":')
                    connection.sendall(b'"accepted","revision":7}\n')

            worker = threading.Thread(target=serve, daemon=True)
            worker.start()
            try:
                result = client.send_command({"id": "manual-1"}, port, 1.0)
                self.assertEqual(result, {"status": "accepted", "revision": 7})
                self.assertEqual(received, [{"id": "manual-1"}])
            finally:
                worker.join(timeout=1)

    def test_rejects_oversized_payload_before_connecting(self):
        with self.assertRaises(ValueError):
            client.send_command({"large": "x" * 16385}, 1)

    def test_rejects_non_finite_payload(self):
        with self.assertRaises(ValueError):
            client.send_command({"value": float("nan")}, 1)

    def test_connection_failure_is_visible(self):
        with socket.socket() as reservation:
            reservation.bind(("127.0.0.1", 0))
            port = reservation.getsockname()[1]
            with self.assertRaises(OSError):
                client.send_command({"id": "a"}, port, 0.1)


if __name__ == "__main__":
    unittest.main()
