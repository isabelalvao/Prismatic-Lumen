"""Métricas descritivas do JSONL P0; não estima precisão física ou intenção."""
import math
from collections import Counter

FLAGS = ("position_valid", "position_tracked", "orientation_valid", "orientation_tracked")


def finite_vector(value: object, length: int) -> bool:
    return (isinstance(value, list) and len(value) == length
            and all(isinstance(x, (int, float)) and not isinstance(x, bool)
                    and math.isfinite(x) for x in value))


def summary(values: list[float]) -> dict:
    if not values:
        return {"count": 0, "min": None, "median": None, "p95": None, "max": None}
    ordered = sorted(values)

    def quantile(fraction: float) -> float:
        index = (len(ordered) - 1) * fraction
        left = int(index)
        right = min(left + 1, len(ordered) - 1)
        fraction_between = index - left
        return ordered[left] * (1 - fraction_between) + ordered[right] * fraction_between

    return {"count": len(values), "min": ordered[0], "median": quantile(0.5),
            "p95": quantile(0.95), "max": ordered[-1]}


class SourceMetrics:
    def __init__(self, hand: bool) -> None:
        self.hand = hand
        self.flags = Counter()
        self.statuses = Counter()
        self.joint_valid = [0] * 26
        self.joint_tracked = [0] * 26
        self.samples = self.active = self.usable = 0
        self.bad_vectors = 0
        self.quaternion_max_norm_error = 0.0
        self.speed = []
        self.null_speed = 0
        self.first_time = self.last_time = None
        self.previous_usable = False
        self.known_duration = self.usable_duration = self.unobserved_duration = 0.0
        self.interval_start = None
        self.intervals = []
        self.recoveries = 0

    def add(self, source: dict, time: float, speed: float | None) -> None:
        active = source.get("active", True)
        pose = source["joints"][0] if self.hand else source
        self.samples += 1
        self.active += int(active)
        self.statuses[source.get("status", "head_pose")] += 1
        for flag in FLAGS:
            self.flags[flag] += int(active and pose[flag])
        position = finite_vector(pose["position_m"], 3)
        orientation = (finite_vector(pose["orientation_xyzw"], 4)
                       and math.isfinite(math.hypot(*pose["orientation_xyzw"]))
                       and math.hypot(*pose["orientation_xyzw"]) > 1e-6)
        if pose["position_valid"] and not position:
            self.bad_vectors += 1
        if pose["orientation_valid"] and not orientation:
            self.bad_vectors += 1
        if orientation and pose["orientation_valid"]:
            norm = math.hypot(*pose["orientation_xyzw"])
            self.quaternion_max_norm_error = max(self.quaternion_max_norm_error, abs(norm - 1))
        if self.hand:
            for i, joint in enumerate(source["joints"]):
                valid = active and joint["position_valid"] and finite_vector(joint["position_m"], 3)
                self.joint_valid[i] += int(valid)
                self.joint_tracked[i] += int(valid and joint["position_tracked"])
                if i > 0 and joint["position_valid"] and not finite_vector(joint["position_m"], 3):
                    self.bad_vectors += 1
                if i > 0 and joint["orientation_valid"] and (
                        not finite_vector(joint["orientation_xyzw"], 4)
                        or not math.isfinite(math.hypot(*joint["orientation_xyzw"]))
                        or math.hypot(*joint["orientation_xyzw"]) <= 1e-6):
                    self.bad_vectors += 1
        usable = bool(active and position and pose["position_valid"] and pose["position_tracked"])
        self.usable += int(usable)
        if self.first_time is None:
            self.first_time = time
        if self.last_time is not None:
            elapsed = time - self.last_time
            if 0 < elapsed <= 0.25:
                self.known_duration += elapsed
                if self.previous_usable:
                    self.usable_duration += elapsed
            elif elapsed > 0.25:
                self.unobserved_duration += elapsed
                if self.interval_start is not None:
                    self.intervals.append({"start_seconds": self.interval_start - self.first_time,
                                           "end_seconds": self.last_time - self.first_time,
                                           "recovered": False, "ended_by_gap": True})
                    self.interval_start = None
        if not usable and self.interval_start is None:
            self.interval_start = time
        elif usable and self.interval_start is not None:
            self.intervals.append({"start_seconds": self.interval_start - self.first_time,
                                   "end_seconds": time - self.first_time,
                                   "recovered": True})
            self.recoveries += 1
            self.interval_start = None
        self.last_time = time
        self.previous_usable = usable
        if speed is None:
            self.null_speed += 1
        elif isinstance(speed, (int, float)) and math.isfinite(speed):
            self.speed.append(speed)
        else:
            self.bad_vectors += 1

    def result(self) -> dict:
        percent = lambda count: count / self.samples * 100 if self.samples else None
        intervals = list(self.intervals)
        if self.interval_start is not None:
            intervals.append({"start_seconds": self.interval_start - self.first_time,
                              "end_seconds": self.last_time - self.first_time, "recovered": False})
        result = {"samples": self.samples, "active_sample_percent": percent(self.active),
                  "usable_position_sample_percent": percent(self.usable),
                  "flag_sample_percentages": {flag: percent(self.flags[flag]) for flag in FLAGS},
                  "statuses": dict(self.statuses), "unusable_position_intervals": intervals,
                  "recoveries": self.recoveries, "known_interval_seconds": self.known_duration,
                  "estimated_usable_seconds": self.usable_duration,
                  "estimated_usable_percent": self.usable_duration / self.known_duration * 100 if self.known_duration else None,
                  "unobserved_gap_seconds": self.unobserved_duration,
                  "linear_speed_m_s": summary(self.speed), "null_speed_samples": self.null_speed,
                  "invalid_numeric_values": self.bad_vectors,
                  "quaternion_max_norm_error": self.quaternion_max_norm_error}
        if self.hand:
            result["representative_joint"] = "palm_0"
            result["joint_position_valid_sample_percentages"] = [percent(x) for x in self.joint_valid]
            result["joint_position_tracked_sample_percentages"] = [percent(x) for x in self.joint_tracked]
        return result


class SessionMetrics:
    def __init__(self) -> None:
        self.sources = {"head": SourceMetrics(False), "left": SourceMetrics(True), "right": SourceMetrics(True)}
        self.frame_statuses = Counter()
        self.intervals = []
        self.gaps = []
        self.qpc_lead_ms = []
        self.query_ms = []
        self.head_angular = []
        self.audio_delta = []
        self.first_time = self.last_time = None
        self.xr_samples = self.conversions = 0
        self.previous_origin = None
        self.origins = []

    def add(self, event: dict) -> None:
        xr = event.get("xr")
        if xr is None:
            return
        time = xr["received_monotonic_ns"] / 1e9
        self.xr_samples += 1
        if self.first_time is None:
            self.first_time = time
        if self.last_time is not None:
            elapsed = time - self.last_time
            self.intervals.append(elapsed)
            if elapsed > 0.25:
                self.gaps.append({"start_seconds": self.last_time - self.first_time,
                                  "end_seconds": time - self.first_time, "duration_seconds": elapsed})
        self.last_time = time
        if self.previous_origin is not None and self.previous_origin != xr["origin_epoch"]:
            self.origins.append({"seconds": time - self.first_time, "origin_epoch": xr["origin_epoch"]})
        self.previous_origin = xr["origin_epoch"]
        self.frame_statuses[xr["frame_status"]] += 1
        self.sources["head"].add(xr["head"], time, event["motion"]["head_linear_m_s"])
        for index, name in enumerate(("left", "right")):
            self.sources[name].add(xr["hands"][index], time, event["motion"]["hand_linear_m_s"][index])
        angular = event["motion"]["head_angular_rad_s"]
        if angular is not None and math.isfinite(angular):
            self.head_angular.append(angular)
        if xr.get("predicted_qpc") is not None and xr.get("qpc_frequency", 0) > 0:
            self.conversions += 1
            self.qpc_lead_ms.append((xr["predicted_qpc"] - xr["received_qpc"]) / xr["qpc_frequency"] * 1000)
        if xr.get("query_duration_ns") is not None:
            self.query_ms.append(xr["query_duration_ns"] / 1e6)
        environment = event.get("environment", {})
        if environment.get("audio_seconds") is not None and environment.get("scene_seconds") is not None:
            self.audio_delta.append(environment["audio_seconds"] - environment["scene_seconds"])

    def result(self) -> dict:
        span = self.last_time - self.first_time if self.first_time is not None else 0
        sources = {name: source.result() for name, source in self.sources.items()}
        return {"xr_samples": self.xr_samples, "observed_span_seconds": span,
                "effective_publish_hz": (self.xr_samples - 1) / span if span > 0 else None,
                "sources": sources, "frame_statuses": dict(self.frame_statuses),
                "sample_intervals_seconds": summary(self.intervals), "gaps_over_250ms": self.gaps,
                "origin_changes": self.origins, "query_duration_ms": summary(self.query_ms),
                "xr_to_qpc_sample_percent": self.conversions / self.xr_samples * 100 if self.xr_samples else None,
                "predicted_time_minus_receipt_ms": summary(self.qpc_lead_ms),
                "head_angular_speed_rad_s": summary(self.head_angular),
                "audio_minus_scene_seconds": summary(self.audio_delta),
                "coverage_method": "Sample percentages; time estimates hold the prior state only for intervals <=250ms. Larger gaps remain unknown; first/last unsampled tails excluded.",
                "limitations": ["Missing position means unusable data, not immobility.",
                                "Velocities are checked numerically; no physical reference or intent is inferred.",
                                "A complete recording is not proof of continuous tracking or a completed audiovisual session.",
                                "Query and predicted-time metrics are not physical display/audio latency."]}
