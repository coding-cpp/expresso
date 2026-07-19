# Expresso benchmark suite

Compares expresso against other HTTP frameworks and runtimes using servers
that all mirror `example/main.cpp` — same routes, same payloads, same
headers — so the measurements compare frameworks, not fixtures.

## Layout

```
benchmark/
├── servers/
│   ├── shared/       # framework-agnostic fixtures (about payload, zip
│   │                 # builder, mime map, parity headers)
│   ├── node-http/    # dependency-free node:http mirror (runs on node/bun/deno)
│   └── express/      # express.js mirror (pinned deps, npm ci)
├── scripts/
│   ├── frameworks.json     # registry of frameworks, scenarios, load settings
│   ├── run_benchmarks.py   # orchestrator (python3 stdlib only)
│   └── compare.py          # renders markdown reports from result JSON
└── results/          # timestamped output, gitignored
```

## Prerequisites

- [wrk](https://github.com/wg/wrk) (`brew install wrk` / `apt install wrk`)
- python3 (stdlib only, no pip packages)
- node ≥ 20 for the JavaScript servers; bun and deno are optional and are
  skipped automatically when not installed
- the expresso example built: `cmake -B build && cmake --build build`

## Running

```sh
cd benchmark/scripts
python3 run_benchmarks.py               # full run, all available frameworks
python3 run_benchmarks.py --quick       # 2s smoke run
python3 run_benchmarks.py --frameworks expresso,express --scenarios health
python3 run_benchmarks.py --mode keepalive   # see caveats below
```

Each run writes `benchmark/results/<timestamp>_<mode>/` containing
`results.json` (raw trials + environment metadata), `comparison.md`, and a
log per server. Re-render or inspect old runs with
`python3 compare.py ../results/<run>/results.json`.

## Methodology and reproducibility

- **Connection-close mode by default.** Expresso does not implement
  keep-alive yet, so every response carries `connection: close`. All
  frameworks are therefore driven with `Connection: close` so everyone pays
  the same per-request TCP setup cost. `--mode keepalive` exists for
  measuring the JavaScript servers against each other, but expresso numbers
  in that mode are still close-mode numbers — don't compare across.
- **TIME_WAIT draining.** Close-mode runs burn one ephemeral port per
  request; on macOS especially, back-to-back runs exhaust ports and produce
  garbage numbers (connect errors, 100x throughput collapse). The harness
  waits for TIME_WAIT sockets to drop below a threshold between trials.
- **The macOS close-mode wall (why trials are short on macOS).** Whichever
  TCP peer closes first holds the TIME_WAIT. When the *client* (wrk) wins
  the FIN race, that ephemeral port is pinned for 2×MSL and cannot be reused
  toward the same server — and macOS only has ~16k ephemeral ports. A server
  that closes even slightly late loses the FIN race often (measured: ~47% of
  connections for node vs ~16% for expresso), so a long trial saturates the
  client port space mid-run and throughput collapses: the same node server
  measured 32.5k req/s in a 2s trial but 8.5k in an 8s trial from an
  identical clean start. This is a client-side artifact, not server
  performance. The harness therefore defaults to 2s × 5 trials on macOS
  (`load.platform_overrides` in frameworks.json) so trials finish before the
  wall; Linux keeps 8s × 3 (wider port range, `tcp_tw_reuse`). For
  authoritative sustained close-mode numbers, run the suite on Linux.
- **Warmup + median.** Every scenario gets a warmup run, then N timed trials
  (default 3); the report shows the median. Trials with socket connect
  errors, timeouts, or non-2xx responses are discarded and retried.
- **Environment stamping.** Results embed the git commit (with dirty flag),
  CPU, core count, OS, and tool versions. Compare runs only when those match.
- **Pinned dependencies.** The express server pins exact versions and
  installs with `npm ci` against the committed lockfile.
- Benchmark on an idle machine, on AC power, and prefer comparing numbers
  from the same run over numbers from different days.

## Scenarios

| Name | Route | What it measures |
|---|---|---|
| health | `GET /health` | per-request framework overhead |
| about-json | `GET /about` | ~1 KB JSON serialization |
| static-png | `GET /coffee.png` | file serving (expresso uses sendfile) |
| post-print | `POST /print` | body parsing + JSON echo |

`/download` (zip) and `/pictures` (directory listing) are implemented by
every server for API parity but not timed: the zip route's output size
differs per archiver, which would make transfer-bound comparisons
meaningless.

## Adding a framework

1. Create `benchmark/servers/<name>/` with a server that mirrors
   `example/main.cpp`. Reuse `servers/shared/` for the about payload, mime
   types, parity headers, and the zip builder — responses must stay
   byte-comparable. The server must read `PORT` (and optionally `ASSETS`)
   from the environment.
2. Add an entry to `frameworks.json`: `name`, `cwd` (repo-root relative),
   `start` command, optional `probe` (binary that must exist, else the
   framework is skipped), optional `install`/`installed_marker` and `build`.

Adding another runtime for an existing server (like bun/deno for node-http)
is just another registry entry with a different `start` command.

## Known caveats

- Header parity is approximate: each framework adds its own incidental
  headers (`X-Powered-By`, `Date`, etags), so tiny-response scenarios have a
  few dozen bytes of variance.
- The JavaScript mirrors log `POST /print` bodies like the example does;
  all server stdout goes to the per-run log files so console throughput
  doesn't skew anyone.
