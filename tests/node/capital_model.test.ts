import assert from "node:assert/strict";
import { describe, it } from "node:test";
import { runFixture } from "../helpers/runner.ts";

describe("capital model", () => {
  it("reports full coverage after a balanced settlement", () => {
    const report = runFixture("balanced_settlement.json") as ReturnType<typeof runFixture> & {
      capital: {
        aggregateLiquid: number;
        aggregateCommitments: number;
        aggregateTimeoutExposure: number;
        assets: Array<{
          asset: string;
          confirmationCoverageBps: number;
          observedUtilizationBps: number;
        }>;
      };
    };

    assert.equal(report.capital.aggregateLiquid, 1_000_000);
    assert.equal(report.capital.aggregateCommitments, 0);
    assert.equal(report.capital.aggregateTimeoutExposure, 0);
    assert.deepEqual(report.capital.assets, [
      {
        asset: "dUSD",
        liquid: 1_000_000,
        observedCommitments: 0,
        confirmedCommitments: 0,
        settledCredits: 99_900,
        earnedFees: 100,
        timeoutExposure: 0,
        observedUtilizationBps: 0,
        confirmationCoverageBps: 10_000,
        overlapBps: 0,
      },
    ]);
  });

  it("quantifies commitments left in a timeout queue", () => {
    const report = runFixture("legitimate_cancellation.json") as ReturnType<typeof runFixture> & {
      capital: {
        aggregateCommitments: number;
        aggregateTimeoutExposure: number;
        timedOutPackets: number;
      };
    };
    assert.equal(report.capital.aggregateCommitments, 60_000);
    assert.equal(report.capital.aggregateTimeoutExposure, 60_000);
    assert.equal(report.capital.timedOutPackets, 1);
  });
});
