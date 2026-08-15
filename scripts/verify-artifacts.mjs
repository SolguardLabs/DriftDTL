#!/usr/bin/env node
import { existsSync, readFileSync, readdirSync, statSync } from "node:fs";
import { join, resolve } from "node:path";

const root = resolve(import.meta.dirname, "..");
const required = [
  "README.md",
  "SECURITY.md",
  "assets/banner.png",
  "docs/architecture.md",
  "docs/economic-model.md",
  "docs/integration-guide.md",
  "docs/observability.md",
  "docs/operations.md",
  "docs/risk-and-limits.md",
  "docs/settlement-lifecycle.md",
];

for (const path of required) {
  if (!existsSync(join(root, path))) throw new Error(`missing required artifact: ${path}`);
}

const docs = readdirSync(join(root, "docs")).filter((name) => name.endsWith(".md"));
if (docs.length !== 7) throw new Error(`expected 7 operational documents, found ${docs.length}`);
if (statSync(join(root, "assets", "banner.png")).size < 100_000) {
  throw new Error("banner does not satisfy the minimum production asset size");
}

const markdown = ["README.md", "SECURITY.md", ...docs.map((name) => join("docs", name))]
  .map((path) => readFileSync(join(root, path), "utf8"))
  .join("\n");
const diagrams = markdown.match(/```mermaid/g)?.length ?? 0;
if (diagrams < 20) throw new Error(`expected at least 20 Mermaid diagrams, found ${diagrams}`);

console.log(`artifacts ok: ${docs.length} documents, ${diagrams} diagrams`);
