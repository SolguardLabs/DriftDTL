import { describe, it } from "node:test";
import assert from "node:assert/strict";
import { balance, runFixture, settlement } from "../helpers/runner.ts";

describe("timeout and retry", () => {
  it("restores visible liquidity after a timeout cancellation", () => {
    const report = runFixture("legitimate_cancellation.json");

    assert.equal(report.summary.packetsCancelled, 1);
    assert.equal(report.summary.packetsSettled, 0);
    assert.equal(report.summary.observedLocked, 0);
    assert.equal(balance(report, "maker-d", "dUSD").available, 200_000);
    assert.equal(balance(report, "merchant-d", "dUSD").available, 0);

    const packet = settlement(report, "pkt-cancel-1");
    assert.equal(packet.observedStatus, "cancelled");
    assert.equal(packet.confirmedStatus, "prepared");
  });

  it("settles a retry after the first attempt times out", () => {
    const report = runFixture("retry_after_timeout.json");

    assert.equal(report.summary.packetsSettled, 1);
    assert.equal(report.summary.totalSettled, 99_900);
    assert.equal(balance(report, "maker-e", "dUSD").available, 200_000);
    assert.equal(balance(report, "merchant-e", "dUSD").available, 99_900);
    assert.equal(balance(report, "lane-ops", "dUSD").available, 100);

    const packet = settlement(report, "pkt-retry-1");
    assert.equal(packet.attempt, 1);
    assert.equal(packet.observedStatus, "consumed");
    assert.equal(packet.confirmedStatus, "settled");
  });
});
