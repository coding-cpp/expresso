#!/usr/bin/env python3
"""Run the expresso benchmark suite across every available framework.

Starts each server from benchmark/scripts/frameworks.json in turn, drives it
with wrk, and writes a timestamped JSON result plus a markdown comparison to
benchmark/results/. Python stdlib only; the sole external tool is wrk.

Reproducibility measures baked in:
  - connection-close mode by default (expresso does not keep connections
    alive yet, so keep-alive would compare different workloads)
  - warmup run before timing, median over N trials
  - waits for TIME_WAIT sockets to drain between runs so ephemeral-port
    exhaustion cannot corrupt close-mode numbers
  - a trial with connect errors or non-2xx responses is invalid and retried
  - every result embeds the git commit, hardware, and tool versions
"""

import argparse
import datetime
import json
import platform
import re
import shutil
import socket
import statistics
import subprocess
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

import compare

SCRIPTS_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPTS_DIR.parents[1]
RESULTS_DIR = REPO_ROOT / "benchmark" / "results"

READY_TIMEOUT_SECONDS = 15
STOP_TIMEOUT_SECONDS = 5


def log(message):
    print(f"[bench] {message}", flush=True)


def run_command(command, cwd=None, timeout=None):
    return subprocess.run(
        command, cwd=cwd, capture_output=True, text=True, timeout=timeout
    )


def tool_version(command):
    if shutil.which(command[0]) is None:
        return None
    try:
        result = run_command(command, timeout=10)
        output = (result.stdout or result.stderr).strip()
        return output.splitlines()[0] if output else ""
    except (OSError, subprocess.TimeoutExpired):
        return None


def cpu_model():
    if platform.system() == "Darwin":
        result = run_command(["sysctl", "-n", "machdep.cpu.brand_string"])
        return result.stdout.strip()
    try:
        for line in Path("/proc/cpuinfo").read_text().splitlines():
            if line.startswith("model name"):
                return line.split(":", 1)[1].strip()
    except OSError:
        pass
    return platform.processor()


def git_revision():
    result = run_command(["git", "rev-parse", "HEAD"], cwd=REPO_ROOT)
    revision = result.stdout.strip()
    dirty = run_command(["git", "status", "--porcelain"], cwd=REPO_ROOT).stdout.strip()
    return f"{revision}{'-dirty' if dirty else ''}" if revision else "unknown"


def collect_metadata(config):
    import os

    return {
        "timestamp_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "git_commit": git_revision(),
        "platform": platform.platform(),
        "cpu": cpu_model(),
        "logical_cores": os.cpu_count(),
        "load": config["load"],
        "tool_versions": {
            "wrk": tool_version(["wrk", "--version"]),
            "node": tool_version(["node", "--version"]),
            "bun": tool_version(["bun", "--version"]),
            "deno": tool_version(["deno", "--version"]),
            "python": platform.python_version(),
        },
    }


def time_wait_count():
    if platform.system() == "Darwin":
        result = run_command(["netstat", "-an", "-p", "tcp"])
    else:
        result = run_command(["ss", "-tan"])
    return sum("TIME_WAIT" in line for line in result.stdout.splitlines())


def wait_for_time_wait_drain(load):
    threshold = load["time_wait_threshold"]
    deadline = time.monotonic() + load["time_wait_max_wait_seconds"]
    count = time_wait_count()
    while count > threshold and time.monotonic() < deadline:
        log(f"waiting for {count} TIME_WAIT sockets to drain below {threshold}...")
        time.sleep(3)
        count = time_wait_count()
    if count > threshold:
        log(f"warning: proceeding with {count} TIME_WAIT sockets still open")


def port_in_use(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        return probe.connect_ex(("127.0.0.1", port)) == 0


def wait_until_ready(port):
    deadline = time.monotonic() + READY_TIMEOUT_SECONDS
    url = f"http://127.0.0.1:{port}/health"
    while time.monotonic() < deadline:
        try:
            with urllib.request.urlopen(url, timeout=2) as response:
                if response.status == 200:
                    return True
        except (urllib.error.URLError, ConnectionError, TimeoutError):
            time.sleep(0.1)
    return False


class ServerProcess:
    def __init__(self, framework, port, log_path):
        self.framework = framework
        self.port = port
        self.log_path = log_path
        self.process = None

    def __enter__(self):
        import os

        if port_in_use(self.port):
            raise RuntimeError(
                f"port {self.port} is already in use; stop the other process first"
            )
        env = dict(os.environ, PORT=str(self.port))
        self.log_file = open(self.log_path, "w")
        self.process = subprocess.Popen(
            self.framework["start"],
            cwd=REPO_ROOT / self.framework["cwd"],
            env=env,
            stdout=self.log_file,
            stderr=subprocess.STDOUT,
        )
        if not wait_until_ready(self.port):
            self.__exit__(None, None, None)
            raise RuntimeError(
                f"{self.framework['name']} did not become ready on port "
                f"{self.port}; see {self.log_path}"
            )
        return self

    def __exit__(self, *_):
        if self.process is not None:
            self.process.terminate()
            try:
                self.process.wait(STOP_TIMEOUT_SECONDS)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        self.log_file.close()
        deadline = time.monotonic() + STOP_TIMEOUT_SECONDS
        while port_in_use(self.port) and time.monotonic() < deadline:
            time.sleep(0.1)


LATENCY_UNITS = {"us": 0.001, "ms": 1.0, "s": 1000.0, "m": 60000.0}


def parse_latency_ms(text):
    match = re.fullmatch(r"([\d.]+)(us|ms|s|m)", text)
    if match is None:
        return None
    return round(float(match.group(1)) * LATENCY_UNITS[match.group(2)], 3)


def parse_wrk_output(output):
    parsed = {
        "total_requests": 0,
        "requests_per_second": None,
        "latency_avg_ms": None,
        "latency_p99_ms": None,
        "transfer_per_second": None,
        "non_2xx_responses": 0,
        "socket_errors": {"connect": 0, "read": 0, "write": 0, "timeout": 0},
    }
    match = re.search(r"(\d+) requests in", output)
    if match:
        parsed["total_requests"] = int(match.group(1))
    match = re.search(r"Requests/sec:\s+([\d.]+)", output)
    if match:
        parsed["requests_per_second"] = float(match.group(1))
    match = re.search(r"Transfer/sec:\s+([\d.]+\w+)", output)
    if match:
        parsed["transfer_per_second"] = match.group(1)
    match = re.search(r"^\s*Latency\s+([\d.]+(?:us|ms|s|m))", output, re.MULTILINE)
    if match:
        parsed["latency_avg_ms"] = parse_latency_ms(match.group(1))
    match = re.search(r"^\s*99%\s+([\d.]+(?:us|ms|s|m))", output, re.MULTILINE)
    if match:
        parsed["latency_p99_ms"] = parse_latency_ms(match.group(1))
    match = re.search(r"Non-2xx or 3xx responses:\s+(\d+)", output)
    if match:
        parsed["non_2xx_responses"] = int(match.group(1))
    match = re.search(
        r"Socket errors: connect (\d+), read (\d+), write (\d+), timeout (\d+)", output
    )
    if match:
        parsed["socket_errors"] = {
            "connect": int(match.group(1)),
            "read": int(match.group(2)),
            "write": int(match.group(3)),
            "timeout": int(match.group(4)),
        }
    return parsed


def write_wrk_script(scenario, close_mode, directory):
    """wrk needs a lua script for anything beyond a plain GET."""
    lines = []
    if scenario["method"] != "GET":
        lines.append(f'wrk.method = "{scenario["method"]}"')
    if scenario.get("body"):
        lines.append(f"wrk.body = {json.dumps(scenario['body'])}")
    if scenario.get("content_type"):
        lines.append(f'wrk.headers["Content-Type"] = "{scenario["content_type"]}"')
    if close_mode:
        lines.append('wrk.headers["Connection"] = "close"')
    if not lines:
        return None
    script_path = directory / f"{scenario['name']}.lua"
    script_path.write_text("\n".join(lines) + "\n")
    return script_path


def run_wrk(scenario, port, load, duration, script_dir):
    close_mode = load["connection_mode"] == "close"
    command = [
        "wrk",
        f"-t{load['threads']}",
        f"-c{load['connections']}",
        f"-d{duration}s",
        "--latency",
    ]
    script = write_wrk_script(scenario, close_mode, script_dir)
    if script is not None:
        command += ["-s", str(script)]
    elif close_mode:
        command += ["-H", "Connection: close"]
    command.append(f"http://127.0.0.1:{port}{scenario['path']}")

    result = run_command(command, timeout=duration + 30)
    return result.stdout


def trial_is_valid(parsed):
    # A handful of connect errors among tens of thousands of connections is
    # loopback noise; a large fraction means port exhaustion corrupted the
    # trial. Non-2xx responses are never acceptable.
    error_budget = max(5, parsed["total_requests"] * 0.005)
    errors = parsed["socket_errors"]["connect"] + parsed["socket_errors"]["timeout"]
    return (
        parsed["requests_per_second"] is not None
        and parsed["non_2xx_responses"] == 0
        and errors <= error_budget
    )


def run_scenario(scenario, port, load, script_dir):
    log(f"  scenario {scenario['name']}: warmup {load['warmup_seconds']}s")
    run_wrk(scenario, port, load, load["warmup_seconds"], script_dir)

    trials = []
    attempts_left = load["trials"] + 2  # allow a couple of invalid-trial retries
    while len(trials) < load["trials"] and attempts_left > 0:
        attempts_left -= 1
        if load["connection_mode"] == "close":
            wait_for_time_wait_drain(load)
        output = run_wrk(scenario, port, load, load["duration_seconds"], script_dir)
        parsed = parse_wrk_output(output)
        parsed["raw_output"] = output
        parsed["valid"] = trial_is_valid(parsed)
        if parsed["valid"]:
            trials.append(parsed)
            log(f"    trial {len(trials)}: {parsed['requests_per_second']:.0f} req/s")
        else:
            log("    invalid trial (errors or non-2xx responses), retrying")
            trials_note = parsed
            trials_note.pop("raw_output", None)
            log(f"    details: {json.dumps(trials_note)}")

    if not trials:
        return {"error": "no valid trials", "trials": []}

    rates = [trial["requests_per_second"] for trial in trials]
    return {
        "requests_per_second": {
            "median": round(statistics.median(rates), 2),
            "min": min(rates),
            "max": max(rates),
        },
        "latency_avg_ms": statistics.median(
            trial["latency_avg_ms"] for trial in trials
        ),
        "latency_p99_ms": statistics.median(
            trial["latency_p99_ms"] for trial in trials
        ),
        "trials": trials,
    }


def prepare_framework(framework):
    probe = framework.get("probe")
    if probe is not None and shutil.which(probe) is None:
        return f"runtime '{probe}' not installed"

    cwd = REPO_ROOT / framework["cwd"]
    if not cwd.is_dir():
        return f"directory {framework['cwd']} does not exist"

    marker = framework.get("installed_marker")
    if framework.get("install") and (marker is None or not (cwd / marker).exists()):
        log(f"  installing dependencies: {' '.join(framework['install'])}")
        result = run_command(framework["install"], cwd=cwd, timeout=300)
        if result.returncode != 0:
            return f"install failed: {result.stderr.strip()[-500:]}"

    if framework.get("build"):
        log(f"  building: {' '.join(framework['build'])}")
        result = run_command(framework["build"], cwd=cwd, timeout=600)
        if result.returncode != 0:
            return f"build failed: {result.stderr.strip()[-500:]}"

    return None


def parse_arguments():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--config", default=str(SCRIPTS_DIR / "frameworks.json"))
    parser.add_argument("--frameworks", help="comma-separated subset to run")
    parser.add_argument("--scenarios", help="comma-separated subset to run")
    parser.add_argument("--mode", choices=["close", "keepalive"])
    parser.add_argument("--trials", type=int)
    parser.add_argument("--duration", type=int, help="seconds per trial")
    parser.add_argument("--threads", type=int)
    parser.add_argument("--connections", type=int)
    parser.add_argument("--port", type=int)
    parser.add_argument("--baseline", default="node-http")
    parser.add_argument("--out", default=str(RESULTS_DIR))
    parser.add_argument(
        "--quick", action="store_true", help="1 trial of 2s each, for smoke tests"
    )
    return parser.parse_args()


def apply_overrides(config, args):
    load = config["load"]
    # On macOS loopback, long close-mode trials exhaust the ~16k ephemeral
    # ports with client-held TIME_WAIT entries and throughput collapses
    # mid-trial (see README: "The macOS close-mode wall"). Short trials that
    # finish before the wall, repeated more often, measure honestly.
    platform_overrides = load.pop("platform_overrides", {})
    load.update(platform_overrides.get(platform.system().lower(), {}))
    if args.quick:
        load.update({"trials": 1, "duration_seconds": 2, "warmup_seconds": 1})
    if args.mode:
        load["connection_mode"] = args.mode
    if args.trials:
        load["trials"] = args.trials
    if args.duration:
        load["duration_seconds"] = args.duration
    if args.threads:
        load["threads"] = args.threads
    if args.connections:
        load["connections"] = args.connections
    if args.port:
        config["port"] = args.port

    def keep(items, csv):
        if csv is None:
            return items
        wanted = [name.strip() for name in csv.split(",")]
        unknown = set(wanted) - {item["name"] for item in items}
        if unknown:
            sys.exit(f"unknown names: {', '.join(sorted(unknown))}")
        return [item for item in items if item["name"] in wanted]

    config["frameworks"] = keep(config["frameworks"], args.frameworks)
    config["scenarios"] = keep(config["scenarios"], args.scenarios)
    return config


def main():
    args = parse_arguments()
    if shutil.which("wrk") is None:
        sys.exit("wrk is required: https://github.com/wg/wrk (brew install wrk)")

    config = apply_overrides(json.loads(Path(args.config).read_text()), args)
    port = config["port"]
    load = config["load"]

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    run_dir = out_dir / f"{stamp}_{load['connection_mode']}"
    run_dir.mkdir()

    results = {"metadata": collect_metadata(config), "frameworks": {}}

    for framework in config["frameworks"]:
        name = framework["name"]
        log(f"framework {name}")
        skip_reason = prepare_framework(framework)
        if skip_reason is not None:
            log(f"  skipped: {skip_reason}")
            results["frameworks"][name] = {"skipped": skip_reason}
            continue

        try:
            with ServerProcess(framework, port, run_dir / f"{name}.log"):
                scenarios = {}
                for scenario in config["scenarios"]:
                    scenarios[scenario["name"]] = run_scenario(
                        scenario, port, load, run_dir
                    )
                results["frameworks"][name] = {
                    "language": framework.get("language"),
                    "scenarios": scenarios,
                }
        except RuntimeError as error:
            log(f"  failed: {error}")
            results["frameworks"][name] = {"skipped": str(error)}

        if load["connection_mode"] == "close":
            wait_for_time_wait_drain(load)

    json_path = run_dir / "results.json"
    json_path.write_text(json.dumps(results, indent=2))

    markdown = compare.render_markdown(results, baseline=args.baseline)
    markdown_path = run_dir / "comparison.md"
    markdown_path.write_text(markdown)

    print()
    print(markdown)
    log(f"results: {json_path}")
    log(f"report:  {markdown_path}")


if __name__ == "__main__":
    main()
