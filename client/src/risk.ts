import type { CapitalAsset } from "./types.ts";

export type CapitalBand = "ample" | "monitored" | "constrained";

export function capitalBand(asset: CapitalAsset): CapitalBand {
  if (asset.confirmationCoverageBps >= 15_000 && asset.observedUtilizationBps <= 6_000) {
    return "ample";
  }
  if (asset.confirmationCoverageBps >= 10_000 && asset.observedUtilizationBps <= 8_000) {
    return "monitored";
  }
  return "constrained";
}

export function headroom(asset: CapitalAsset, targetUtilizationBps = 7_500): number {
  if (targetUtilizationBps <= 0 || targetUtilizationBps > 10_000) {
    throw new RangeError("target utilization must be within 1..10000 bps");
  }
  const operatingBase = asset.liquid + asset.observedCommitments;
  const targetCommitments = Math.floor((operatingBase * targetUtilizationBps) / 10_000);
  return Math.max(0, targetCommitments - asset.observedCommitments);
}
