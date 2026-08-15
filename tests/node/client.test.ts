import assert from "node:assert/strict";
import { describe, it } from "node:test";
import { DriftClient } from "../../client/src/index.ts";
import { binary, root } from "../helpers/runner.ts";

describe("TypeScript client", () => {
  const client = new DriftClient({ binary, cwd: root });

  it("validates and executes a checked scenario path", () => {
    assert.equal(client.validate("tests/fixtures/balanced_settlement.json"), "ok");
    const report = client.run("tests/fixtures/balanced_settlement.json");
    assert.equal(report.ok, true);
    assert.equal(report.capital.assets[0]?.asset, "dUSD");
  });

  it("rejects unsupported scenario extensions", () => {
    assert.throws(() => client.validate("README.md"), /\.json extension/);
  });
});
