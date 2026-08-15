export type ScenarioAction =
  | { type: "advance"; epoch: number }
  | { type: "snapshot"; epoch: number; note?: string }
  | { type: "confirm"; epoch: number; packet: string; ack: string }
  | { type: "cancel"; epoch: number; packet: string; force?: boolean }
  | { type: "submit" | "retry"; epoch: number; packet: PacketInput };

export type PacketInput = {
  id: string;
  lane: string;
  source: string;
  recipient: string;
  amount: number;
  priority?: "low" | "normal" | "high" | "critical";
  memo?: string;
};

export type DriftScenario = {
  name: string;
  startEpoch: number;
  accounts: Array<{
    id: string;
    balances: Array<{ asset: string; available: number }>;
  }>;
  lanes: Array<{
    id: string;
    asset: string;
    operator: string;
    policy: {
      timeoutEpochs: number;
      feeBps: number;
      maxRetries: number;
      minPacketAmount: number;
      maxPacketAmount: number;
    };
  }>;
  actions: ScenarioAction[];
};

export type CapitalAsset = {
  asset: string;
  liquid: number;
  observedCommitments: number;
  confirmedCommitments: number;
  settledCredits: number;
  earnedFees: number;
  timeoutExposure: number;
  observedUtilizationBps: number;
  confirmationCoverageBps: number;
  overlapBps: number;
};

export type DriftReport = {
  ok: boolean;
  summary: Record<string, number>;
  accounts: Array<{ id: string; balances: Array<Record<string, string | number>> }>;
  settlements: Array<Record<string, string | number>>;
  capital: {
    aggregateLiquid: number;
    aggregateCommitments: number;
    aggregateTimeoutExposure: number;
    openPackets: number;
    timedOutPackets: number;
    assets: CapitalAsset[];
  };
};

export type ClientOptions = {
  binary?: string;
  cwd?: string;
  timeoutMs?: number;
};
