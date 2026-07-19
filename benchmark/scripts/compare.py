#!/usr/bin/env python3
"""Render a markdown comparison from benchmark result JSON files.

Used automatically by run_benchmarks.py at the end of a run, and usable
standalone to re-render or diff older runs:

    python3 compare.py ../results/20260719T120000Z_close/results.json
"""

import argparse
import json
from pathlib import Path


def _scenario_names(results):
    names = []
    for framework in results["frameworks"].values():
        for name in framework.get("scenarios", {}):
            if name not in names:
                names.append(name)
    return names


def _framework_names(results):
    return list(results["frameworks"].keys())


def _median_rate(results, framework, scenario):
    data = (
        results["frameworks"]
        .get(framework, {})
        .get("scenarios", {})
        .get(scenario, {})
    )
    rate = data.get("requests_per_second")
    return rate["median"] if rate else None


def render_markdown(results, baseline=None):
    metadata = results["metadata"]
    frameworks = _framework_names(results)
    scenarios = _scenario_names(results)
    if baseline not in frameworks:
        baseline = frameworks[0] if frameworks else None

    lines = ["# Benchmark comparison", ""]
    lines.append(f"- **Date:** {metadata['timestamp_utc']}")
    lines.append(f"- **Commit:** {metadata['git_commit']}")
    lines.append(f"- **Machine:** {metadata['cpu']} ({metadata['logical_cores']} cores), {metadata['platform']}")
    load = metadata["load"]
    lines.append(
        f"- **Load:** wrk -t{load['threads']} -c{load['connections']} "
        f"-d{load['duration_seconds']}s, {load['trials']} trial(s), "
        f"connection mode: {load['connection_mode']}"
    )
    versions = ", ".join(
        f"{tool} {version}"
        for tool, version in metadata["tool_versions"].items()
        if version
    )
    lines.append(f"- **Tools:** {versions}")
    lines.append("")

    lines.append("## Throughput (median requests/sec, higher is better)")
    lines.append("")
    header = ["Scenario"] + [
        f"{name}{' (baseline)' if name == baseline else ''}" for name in frameworks
    ]
    lines.append("| " + " | ".join(header) + " |")
    lines.append("|" + "---|" * len(header))
    for scenario in scenarios:
        base_rate = _median_rate(results, baseline, scenario)
        row = [scenario]
        for framework in frameworks:
            rate = _median_rate(results, framework, scenario)
            if rate is None:
                skipped = results["frameworks"][framework].get("skipped")
                row.append("skipped" if skipped else "-")
            elif base_rate and framework != baseline:
                row.append(f"{rate:,.0f} ({rate / base_rate:.2f}x)")
            else:
                row.append(f"{rate:,.0f}")
        lines.append("| " + " | ".join(row) + " |")
    lines.append("")

    lines.append("## Latency (median of trials, milliseconds)")
    lines.append("")
    lines.append("| Scenario | " + " | ".join(frameworks) + " |")
    lines.append("|" + "---|" * (len(frameworks) + 1))
    for scenario in scenarios:
        row = [scenario]
        for framework in frameworks:
            data = (
                results["frameworks"][framework]
                .get("scenarios", {})
                .get(scenario, {})
            )
            if "latency_avg_ms" in data:
                row.append(
                    f"avg {data['latency_avg_ms']}, p99 {data['latency_p99_ms']}"
                )
            else:
                row.append("-")
        lines.append("| " + " | ".join(row) + " |")
    lines.append("")

    skipped = {
        name: framework["skipped"]
        for name, framework in results["frameworks"].items()
        if "skipped" in framework
    }
    if skipped:
        lines.append("## Skipped")
        lines.append("")
        for name, reason in skipped.items():
            lines.append(f"- **{name}**: {reason}")
        lines.append("")

    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("results", nargs="+", help="results.json file(s) to render")
    parser.add_argument("--baseline", default="node-http")
    args = parser.parse_args()

    for results_path in args.results:
        results = json.loads(Path(results_path).read_text())
        print(render_markdown(results, baseline=args.baseline))


if __name__ == "__main__":
    main()
