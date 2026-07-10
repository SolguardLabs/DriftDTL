import { spawnSync } from "node:child_process";
import { existsSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

export const root = resolve(dirname(fileURLToPath(import.meta.url)), "..", "..");
export const binary = join(root, "out", process.platform === "win32" ? "driftdtl.exe" : "driftdtl");

export type Balance = {
  asset: string;
  available: number;
  observedLocked: number;
  confirmedLocked: number;
  received: number;
  feesPaid: number;
  feesEarned: number;
};

export type Account = {
  id: string;
  balances: Balance[];
};

export type Settlement = {
  packet: string;
  lane: string;
  asset: string;
  source: string;
  recipient: string;
  operator: string;
  gross: number;
  net: number;
  fee: number;
  observedStatus: string;
  confirmedStatus: string;
  observedEpoch: number;
  timeoutEpoch: number;
  lastUpdateEpoch: number;
  attempt: number;
  ackCount: number;
};

export type DriftReport = {
  ok: boolean;
  summary: {
    epoch: number;
    totalSeeded: number;
    totalSettled: number;
    totalFees: number;
    observedLocked: number;
    confirmedLocked: number;
    packetsOpen: number;
    packetsCancelled: number;
    packetsSettled: number;
    duplicateConfirmations: number;
    warnings: number;
  };
  accounts: Account[];
  settlements: Settlement[];
  events?: Array<{ kind: string; epoch: number; packet: string; lane: string; message: string }>;
};

export function ensureBuilt(): void {
  if (existsSync(binary)) {
    return;
  }
  const result = spawnSync(process.execPath, ["scripts/build.mjs"], {
    cwd: root,
    encoding: "utf8",
  });
  if (result.status !== 0) {
    throw new Error(result.stderr || result.stdout || "build failed");
  }
}

export function runCli(args: string[]): string {
  ensureBuilt();
  const result = spawnSync(binary, args, {
    cwd: root,
    encoding: "utf8",
  });
  if (result.status !== 0) {
    throw new Error(`command failed: ${binary} ${args.join(" ")}\n${result.stderr}`);
  }
  return result.stdout;
}

export function runFixture(name: string, options: string[] = []): DriftReport {
  const fixture = join("tests", "fixtures", name);
  return JSON.parse(runCli(["run", fixture, "--json", ...options])) as DriftReport;
}

export function validateFixture(name: string): string {
  const fixture = join("tests", "fixtures", name);
  return runCli(["validate", fixture]).trim();
}

export function account(report: DriftReport, id: string): Account {
  const found = report.accounts.find((item) => item.id === id);
  if (!found) {
    throw new Error(`missing account ${id}`);
  }
  return found;
}

export function balance(report: DriftReport, accountId: string, asset: string): Balance {
  const found = account(report, accountId).balances.find((item) => item.asset === asset);
  if (!found) {
    throw new Error(`missing balance ${accountId}/${asset}`);
  }
  return found;
}

export function settlement(report: DriftReport, packet: string): Settlement {
  const found = report.settlements.find((item) => item.packet === packet);
  if (!found) {
    throw new Error(`missing settlement ${packet}`);
  }
  return found;
}
