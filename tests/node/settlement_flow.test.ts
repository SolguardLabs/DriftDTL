import { describe, it } from "node:test";
import assert from "node:assert/strict";
import { balance, runFixture, settlement, validateFixture } from "../helpers/runner.ts";

describe("settlement flow", () => {
  it("validates the balanced settlement fixture", () => {
    assert.equal(validateFixture("balanced_settlement.json"), "ok");
  });

  it("settles a single packet and emits stable accounting", () => {
    const report = runFixture("balanced_settlement.json", ["--events"]);

    assert.equal(report.ok, true);
    assert.equal(report.summary.totalSeeded, 1_000_000);
    assert.equal(report.summary.totalSettled, 99_900);
    assert.equal(report.summary.totalFees, 100);
    assert.equal(report.summary.packetsSettled, 1);
    assert.equal(report.summary.packetsOpen, 0);

    assert.equal(balance(report, "maker-a", "dUSD").available, 900_000);
    assert.equal(balance(report, "merchant-a", "dUSD").available, 99_900);
    assert.equal(balance(report, "lane-ops", "dUSD").available, 100);

    const packet = settlement(report, "pkt-balanced-1");
    assert.equal(packet.observedStatus, "consumed");
    assert.equal(packet.confirmedStatus, "settled");
    assert.equal(packet.ackCount, 1);
  });

  it("handles packets delivered out of order", () => {
    const report = runFixture("out_of_order_delivery.json");

    assert.equal(report.summary.packetsSettled, 2);
    assert.equal(report.summary.totalSettled, 119_880);
    assert.equal(balance(report, "maker-b", "dUSD").available, 880_000);
    assert.equal(balance(report, "merchant-b", "dUSD").received, 119_880);
    assert.equal(balance(report, "lane-ops", "dUSD").feesEarned, 120);

    const first = settlement(report, "pkt-ooo-1");
    const second = settlement(report, "pkt-ooo-2");
    assert.equal(first.confirmedStatus, "settled");
    assert.equal(second.confirmedStatus, "settled");
    assert.equal(second.lastUpdateEpoch < first.lastUpdateEpoch, true);
  });
});
