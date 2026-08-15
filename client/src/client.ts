import { spawnSync } from "node:child_process";
import { mkdtempSync, rmSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import { isAbsolute, join, resolve } from "node:path";
import type { ClientOptions, DriftReport, DriftScenario } from "./types.ts";

export class DriftClient {
  readonly binary: string;
  readonly cwd: string;
  readonly timeoutMs: number;

  constructor(options: ClientOptions = {}) {
    this.cwd = resolve(options.cwd ?? process.cwd());
    this.binary = resolve(
      this.cwd,
      options.binary ?? join("out", process.platform === "win32" ? "driftdtl.exe" : "driftdtl"),
    );
    this.timeoutMs = options.timeoutMs ?? 10_000;
    if (!isAbsolute(this.binary) || this.timeoutMs < 100 || this.timeoutMs > 120_000) {
      throw new Error("invalid client configuration");
    }
  }

  validate(path: string): string {
    return this.execute(["validate", this.scenarioPath(path)]).trim();
  }

  run(path: string, includeEvents = false): DriftReport {
    const args = ["run", this.scenarioPath(path), "--json"];
    if (includeEvents) args.push("--events");
    return JSON.parse(this.execute(args)) as DriftReport;
  }

  simulate(scenario: DriftScenario, includeEvents = false): DriftReport {
    const directory = mkdtempSync(join(tmpdir(), "driftdtl-sdk-"));
    const path = join(directory, "scenario.json");
    try {
      writeFileSync(path, JSON.stringify(scenario), { encoding: "utf8", flag: "wx" });
      return this.run(path, includeEvents);
    } finally {
      rmSync(directory, { recursive: true, force: true });
    }
  }

  private scenarioPath(path: string): string {
    const resolved = resolve(this.cwd, path);
    if (!resolved.toLowerCase().endsWith(".json")) {
      throw new Error("scenario path must use the .json extension");
    }
    return resolved;
  }

  private execute(args: string[]): string {
    const result = spawnSync(this.binary, args, {
      cwd: this.cwd,
      encoding: "utf8",
      shell: false,
      timeout: this.timeoutMs,
      windowsHide: true,
      maxBuffer: 8 * 1024 * 1024,
    });
    if (result.error) throw result.error;
    if (result.status !== 0) {
      throw new Error(result.stderr.trim() || `driftdtl exited with status ${result.status}`);
    }
    return result.stdout;
  }
}
