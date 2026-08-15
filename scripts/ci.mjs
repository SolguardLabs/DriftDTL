#!/usr/bin/env node
import { spawnSync } from "node:child_process";

const npmCommand = process.platform === "win32" ? "npm.cmd" : "npm";
const steps = [
  ["run", "build", "--", "--warnings"],
  ["run", "test:ts"],
  ["run", "typecheck"],
  ["run", "format:check"],
  ["run", "verify:artifacts"],
];

for (const args of steps) {
  const command = process.platform === "win32" ? "cmd.exe" : npmCommand;
  const commandArgs = process.platform === "win32" ? ["/d", "/s", "/c", npmCommand, ...args] : args;
  const result = spawnSync(command, commandArgs, {
    stdio: "inherit",
    shell: false,
  });
  if (result.status !== 0) {
    process.exit(result.status ?? 1);
  }
}
