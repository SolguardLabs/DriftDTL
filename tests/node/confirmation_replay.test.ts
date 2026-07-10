import { describe, it } from "node:test";
import assert from "node:assert/strict";
import { balance, runFixture, settlement } from "../helpers/runner.ts";

describe("confirmation handling", () => {
  it("keeps duplicate acknowledgements idempotent", () => {
    const report = runFixture("duplicate_confirmation.json", ["--events"]);

    assert.equal(report.summary.packetsSettled, 1);
    assert.equal(report.summary.duplicateConfirmations, 1);
    assert.equal(report.summary.totalSettled, 79_920);
    assert.equal(balance(report, "merchant-c", "dUSD").available, 79_920);
    assert.equal(balance(report, "lane-ops", "dUSD").available, 80);

    const packet = settlement(report, "pkt-dup-1");
    assert.equal(packet.ackCount, 1);
    assert.equal(packet.confirmedStatus, "settled");
    assert.equal(
      report.events?.some((event) => event.kind === "duplicate_confirmation"),
      true,
    );
  });

  it("records independent acknowledgement ids for separate packets", () => {
    const report = runFixture("out_of_order_delivery.json");

    assert.equal(settlement(report, "pkt-ooo-1").ackCount, 1);
    assert.equal(settlement(report, "pkt-ooo-2").ackCount, 1);
  });
});
