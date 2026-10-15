"""Verifica integridade e reconstitui descritores do JSONL P0, sem inferir intenção."""
import argparse
import json
import math
from pathlib import Path
from session_metrics import SessionMetrics, finite_vector


def linear_speed(first: dict, second: dict, elapsed: float) -> float | None:
    if not all(p["position_valid"] and p["position_tracked"]
               and finite_vector(p["position_m"], 3) for p in (first, second)):
        return None
    return math.dist(first["position_m"], second["position_m"]) / elapsed


def angular_speed(first: dict, second: dict, elapsed: float) -> float | None:
    if not all(p["orientation_valid"] and p["orientation_tracked"]
               and finite_vector(p["orientation_xyzw"], 4) for p in (first, second)):
        return None
    x, y = first["orientation_xyzw"], second["orientation_xyzw"]
    x_norm, y_norm = math.hypot(*x), math.hypot(*y)
    if any(not math.isfinite(norm) or norm <= 1e-6 for norm in (x_norm, y_norm)):
        return None
    cosine = min(1.0, max(0.0, abs(sum((a / x_norm) * (b / y_norm) for a, b in zip(x, y)))))
    return 2 * math.acos(cosine) / elapsed


def derive(first: dict | None, second: dict) -> dict:
    result = {"head_linear_m_s": None, "head_angular_rad_s": None,
              "hand_linear_m_s": [None, None]}
    if (first is None or first["frame_status"] != "located"
            or second["frame_status"] != "located"
            or first["origin_epoch"] != second["origin_epoch"]
            or first["xr_time_ns"] is None or second["xr_time_ns"] is None):
        return result
    elapsed = (second["xr_time_ns"] - first["xr_time_ns"]) / 1e9
    if elapsed <= 0 or elapsed > 0.25:
        return result
    result["head_linear_m_s"] = linear_speed(first["head"], second["head"], elapsed)
    result["head_angular_rad_s"] = angular_speed(first["head"], second["head"], elapsed)
    for index in range(2):
        a, b = first["hands"][index], second["hands"][index]
        if a["active"] and b["active"] and a["status"] == b["status"] == "located":
            result["hand_linear_m_s"][index] = linear_speed(a["joints"][0], b["joints"][0], elapsed)
    return result


def equivalent(first: float | None | list, second: float | None | list) -> bool:
    if isinstance(first, list):
        return (isinstance(second, list) and len(first) == len(second)
                and all(equivalent(a, b) for a, b in zip(first, second)))
    if first is None or second is None:
        return first is None and second is None
    return (isinstance(second, (int, float)) and not isinstance(second, bool)
            and math.isfinite(second) and math.isclose(first, second, rel_tol=1e-4, abs_tol=1e-4))


def integer(value, name, minimum=0):
    if type(value) is not int or value < minimum or value > 2**63 - 1:
        raise ValueError(f"Inteiro inválido: {name}")


def validate_number(value, name):
    if not isinstance(value, (int, float)) or isinstance(value, bool) or not math.isfinite(value):
        raise ValueError(f"Número inválido: {name}")


def session_events(path: Path | str):
    def reject_constant(value):
        raise ValueError(f"Número JSON não finito: {value}")

    def finite_float(value):
        result = float(value)
        validate_number(result, "JSON float")
        return result

    with Path(path).open(encoding="utf-8") as source:
        for number, line in enumerate(source, 1):
            event = json.loads(line, parse_constant=reject_constant, parse_float=finite_float)
            if not isinstance(event, dict):
                raise ValueError(f"Evento na linha {number} não é um objeto JSON.")
            yield number, event


def inspect(path: Path | str, expected_executable_sha256: str | None = None) -> dict:
    errors = []
    previous = None
    previous_received = None
    previous_sequence = None
    rows = 0
    manifest = end = None
    metrics = SessionMetrics()
    previous_xr_time = None
    previous_xr_receipt = None
    counts = {"observations": 0, "head_position_valid": 0,
              "left_active": 0, "right_active": 0, "origin_changes": 0}
    for number, event in session_events(path):
        rows += 1
        kind = event.get("type")
        if number == 1:
            manifest = event
            if kind != "session_manifest" or event.get("schema_version") != 1:
                errors.append("Manifesto ausente ou versão desconhecida.")
        elif kind == "session_manifest":
            errors.append("Manifesto repetido.")
        if (kind in {"observation", "command", "configuration_changed", "application_end"}
                and event.get("session_id") != manifest.get("session_id")):
            errors.append(f"ID de sessão incompatível na linha {number}.")
        if end is not None:
            errors.append("Há dados depois do encerramento.")
        if kind == "session_end":
            end = event
        if kind != "observation":
            continue
        counts["observations"] += 1
        received = event["received_monotonic_ns"]
        integer(received, "received_monotonic_ns")
        if previous_received is not None and received < previous_received:
            errors.append(f"Relógio de receção regressou na linha {number}.")
        previous_received = received
        xr = event.get("xr")
        if xr is None:
            continue
        if (not isinstance(xr, dict) or len(xr.get("hands", [])) != 2
                or any(len(hand.get("joints", [])) != 26 for hand in xr["hands"])):
            raise ValueError(f"Estrutura XR incompleta na linha {number}.")
        for field in ("sequence", "origin_epoch", "received_monotonic_ns", "received_qpc", "query_duration_ns"):
            integer(xr[field], f"xr.{field}")
        integer(xr["qpc_frequency"], "xr.qpc_frequency", minimum=1)
        if xr.get("predicted_qpc") is not None:
            integer(xr["predicted_qpc"], "xr.predicted_qpc")
        if previous_xr_receipt is not None and xr["received_monotonic_ns"] < previous_xr_receipt:
            errors.append(f"Relógio de aquisição XR regressou na linha {number}.")
        previous_xr_receipt = xr["received_monotonic_ns"]
        for hand in xr["hands"]:
            if type(hand["active"]) is not bool:
                raise ValueError("Flag active não booleana.")
        for pose in [xr["head"], *(joint for hand in xr["hands"] for joint in hand["joints"])]:
            for flag in ("position_valid", "position_tracked", "orientation_valid", "orientation_tracked"):
                if type(pose[flag]) is not bool:
                    raise ValueError(f"Flag {flag} não booleana.")
        for field in ("audio_seconds", "scene_seconds"):
            value = event.get("environment", {}).get(field)
            if value is not None:
                validate_number(value, field)
                if value < 0:
                    raise ValueError(f"Tempo negativo: {field}")
        if previous_sequence is not None and xr["sequence"] <= previous_sequence:
            errors.append(f"Sequência XR não cresce na linha {number}.")
        previous_sequence = xr["sequence"]
        if xr.get("xr_time_ns") is not None:
            integer(xr["xr_time_ns"], "xr.xr_time_ns", minimum=1)
            if previous_xr_time is not None and xr["xr_time_ns"] <= previous_xr_time:
                errors.append(f"Tempo XR não cresce na linha {number}.")
            previous_xr_time = xr["xr_time_ns"]
        expected = derive(previous, xr)
        if any(not equivalent(value, event["motion"][key]) for key, value in expected.items()):
            errors.append(f"Descritores não se reconstituem na linha {number}.")
        counts["head_position_valid"] += int(xr["head"]["position_valid"])
        counts["left_active"] += int(xr["hands"][0]["active"])
        counts["right_active"] += int(xr["hands"][1]["active"])
        if previous is not None:
            counts["origin_changes"] += int(previous["origin_epoch"] != xr["origin_epoch"])
        previous = xr
        metrics.add(event)
    if end is None:
        errors.append("Encerramento ausente; ficheiro incompleto.")
    elif (not end.get("complete") or not end.get("normal_exit")
          or end.get("dropped") or end.get("write_failed")):
        errors.append("Sessão interrompida ou com perda de registo.")
    if end is not None and end.get("accepted") != rows - 2:
        errors.append("Contagem de eventos não coincide com o encerramento.")
    tracking_report = metrics.result()
    for name, source in tracking_report["sources"].items():
        if source["invalid_numeric_values"]:
            errors.append(f"{name}: valores inválidos apesar de flags de validade.")
    recorded_hash = manifest.get("executable_sha256") if manifest else None
    matches = (recorded_hash is not None and recorded_hash.lower() == expected_executable_sha256.lower()
               if expected_executable_sha256 else None)
    if matches is False:
        errors.append("SHA-256 do executável difere do esperado.")
    return {"session_id": manifest.get("session_id") if manifest else None,
            "runtime": manifest.get("runtime") if manifest else None,
            "tracking": manifest.get("tracking") if manifest else None,
            "counts": counts, "errors": errors,
            "valid_record": not errors,
            "tool_version": "p0-inspector-2", "tracking_analysis": tracking_report,
            "build_identity": {"app_version": manifest.get("app_version") if manifest else None,
                               "base_commit": manifest.get("build_commit") if manifest else None,
                               "executable_sha256": recorded_hash, "matches_expected_executable": matches,
                               "note": "P0 records the base Git commit; the binary hash identifies the local development build."},
            "scope": "Integridade, reconstituição e cobertura observada; não mede precisão física ou experiência subjetiva."}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("session", type=Path)
    parser.add_argument("--output", type=Path, help="Guardar análise JSON completa.")
    parser.add_argument("--expect-executable-sha256", help="Comparar identidade do binário gravado.")
    arguments = parser.parse_args()
    try:
        result = inspect(arguments.session, arguments.expect_executable_sha256)
        if arguments.output:
            if arguments.output.resolve() == arguments.session.resolve():
                raise ValueError("O relatório não pode substituir o JSONL de origem.")
            arguments.output.parent.mkdir(parents=True, exist_ok=True)
            arguments.output.write_text(json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False), encoding="utf-8")
    except (OSError, ValueError, KeyError, TypeError, AttributeError, IndexError, OverflowError) as error:
        print(json.dumps({"valid_record": False, "errors": [str(error)]}, ensure_ascii=False))
        return 1
    displayed = result
    if arguments.output:
        displayed = {"analysis_file": str(arguments.output), "valid_record": result["valid_record"],
                     "observations": result["counts"]["observations"],
                     "usable_position_sample_percentages": {name: source["usable_position_sample_percent"]
                                                            for name, source in result["tracking_analysis"]["sources"].items()},
                     "errors": result["errors"]}
    print(json.dumps(displayed, ensure_ascii=False, indent=2, allow_nan=False))
    return 0 if result["valid_record"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

