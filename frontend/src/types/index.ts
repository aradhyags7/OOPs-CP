// TypeScript interfaces matching backend and C++ engine data contracts

export type DecisionType = 'ALLOW' | 'BLOCK' | 'REJECT';
export type ProtocolType = 'TCP' | 'UDP' | 'ICMP';
export type DirectionType = 'INBOUND' | 'OUTBOUND';

export interface SummaryStats {
  totalPackets: number;
  allowed: number;
  allowedPercent: number;
  blocked: number;
  blockedPercent: number;
  rejected: number;
  rejectedPercent: number;
  changeIndicator: string;
}

export interface RuleHitItem {
  ruleId: string;
  rawId: number;
  name: string;
  hits: number;
  label: string;
}

export interface DecisionRecord {
  timestamp: string;
  time: string;
  packetId: string;
  protocol: ProtocolType;
  direction: DirectionType;
  src: string;
  srcIP: string;
  srcPort: number;
  dst: string;
  dstIP: string;
  dstPort: number;
  payloadBytes: number;
  decision: DecisionType;
  rule: string;
  ruleId: number;
  reason: string;
}

export interface DashboardData {
  summary: SummaryStats;
  topRules: RuleHitItem[];
  recentDecisions: DecisionRecord[];
  engineStatus: string;
}

export interface FirewallRuleItem {
  id: number;
  priority: number;
  enabled: boolean;
  action: 'ALLOW' | 'BLOCK' | 'REJECT' | 'LOG';
  name: string;
  conditions: string;
}

export interface RulesResponse {
  status: string;
  defaultPolicy: string;
  rules: FirewallRuleItem[];
}

export interface PacketInspectRequest {
  srcIP: string;
  dstIP: string;
  srcPort: number;
  dstPort: number;
  protocol: string;
  direction: string;
}

export interface PacketInspectResponse {
  status: string;
  packet: {
    srcIP: string;
    dstIP: string;
    srcPort: number;
    dstPort: number;
    protocol: string;
    direction: string;
  };
  decision: 'ALLOWED' | 'BLOCKED' | 'REJECTED';
  matchedRuleId: number;
  matchedRuleName: string;
  reason: string;
  loggedRuleIds: number[];
}

export interface SimulationResult {
  status: string;
  scenario: string;
  elapsedSeconds: number;
  throughputPps: number;
  packetsProcessed: number;
  packetsAllowed: number;
  packetsBlocked: number;
  packetsRejected: number;
  ruleHitCounts: Record<string, number>;
}

export interface AuditResponse {
  totalRecords: number;
  records: DecisionRecord[];
}

export interface HealthResponse {
  status: string;
  engineConnected: boolean;
  version: string;
  binary: string;
  rulesConfig: string;
}
