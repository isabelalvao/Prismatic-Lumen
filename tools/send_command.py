"""Cliente local para enviar um comando JSON ao Fantasynt."""

from __future__ import annotations

import argparse
import json
import socket
import sys
from pathlib import Path


def send_command(document: dict, port: int = 47631, timeout: float = 3.0) -> dict:
    if not isinstance(document, dict):
        raise ValueError("O comando tem de ser um objeto JSON")
    payload = json.dumps(document, separators=(",", ":"), ensure_ascii=False, allow_nan=False).encode("utf-8")
    if len(payload) > 16384:
        raise ValueError("O comando ultrapassa 16 KiB")
    if not 1 <= port <= 65535:
        raise ValueError("Porta inválida")
    with socket.create_connection(("127.0.0.1", port), timeout=timeout) as connection:
        connection.sendall(payload + b"\n")
        response = bytearray()
        while b"\n" not in response:
            block = connection.recv(4096)
            if not block:
                raise OSError("A ligação terminou antes da confirmação")
            response.extend(block)
            if len(response) > 65536:
                raise ValueError("Confirmação demasiado grande")
        result = json.loads(bytes(response).split(b"\n", 1)[0])
        if not isinstance(result, dict) or result.get("status") not in {"accepted", "rejected"}:
            raise ValueError("Confirmação inválida")
        return result


def _unique_object(pairs: list[tuple[str, object]]) -> dict:
    result = {}
    for name, value in pairs:
        if name in result:
            raise ValueError(f"Chave JSON repetida: {name}")
        result[name] = value
    return result


def _reject_constant(value: str) -> None:
    raise ValueError(f"Número JSON inválido: {value}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file", type=Path)
    parser.add_argument("--port", type=int, default=47631)
    parser.add_argument("--timeout", type=float, default=3.0)
    args = parser.parse_args()
    try:
        document = json.loads(
            args.file.read_text(encoding="utf-8-sig"),
            object_pairs_hook=_unique_object,
            parse_constant=_reject_constant,
        )
        result = send_command(document, args.port, args.timeout)
        print(json.dumps(result, ensure_ascii=False, indent=2))
        return 0 if result["status"] == "accepted" else 2
    except (OSError, ValueError) as error:
        print(f"Erro: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
