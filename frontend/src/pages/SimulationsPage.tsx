import React, { useState } from 'react';
import { 
  BarChart2, 
  Play, 
  Zap, 
  TrendingUp, 
  CheckCircle2, 
  Ban 
} from 'lucide-react';
import { api } from '../api/client';
import type { SimulationResult } from '../types';

interface SimulationsPageProps {
  onSimulationRun?: () => void;
}

export const SimulationsPage: React.FC<SimulationsPageProps> = ({
  onSimulationRun,
}) => {
  const [activeScenario, setActiveScenario] = useState('G');
  const [packetCount, setPacketCount] = useState(50000);
  const [running, setRunning] = useState(false);
  const [result, setResult] = useState<SimulationResult | null>(null);
  const [error, setError] = useState<string | null>(null);

  const scenarios = [
    {
      id: 'G',
      title: 'Scenario G: High-Volume Batch Evaluation Benchmark',
      description: 'Executes high-throughput batch evaluation across 6 policies with synthetic multi-protocol packets (reproducing the reference 50,000 packets benchmark).',
      defaultCount: 50000,
      badge: 'Benchmark',
    },
    {
      id: 'A',
      title: 'Scenario A: Basic Web Filtering',
      description: 'Tests Inbound HTTPS (TCP 443) allowance, specific host blocking, and default-deny fallback behavior.',
      defaultCount: 100,
      badge: 'Core Policy',
    },
    {
      id: 'B',
      title: 'Scenario B: Port & Protocol Filtering',
      description: 'Validates outbound UDP DNS query forwarding and inbound TCP Telnet rejection with simulated ICMP unreachable response.',
      defaultCount: 100,
      badge: 'Port Ranges',
    },
    {
      id: 'C',
      title: 'Scenario C: Rule Priority & Overlapping Policies',
      description: 'Demonstrates priority inversion resolution where a specific host allow rule overrides a wider subnet block rule.',
      defaultCount: 200,
      badge: 'Priority Logic',
    },
    {
      id: 'D',
      title: 'Scenario D: Subnet CIDR Bitmask Filtering',
      description: 'Evaluates IPv4 32-bit subnet matching across /16 office subnets and /24 DMZ rogue networks.',
      defaultCount: 400,
      badge: 'CIDR Math',
    },
    {
      id: 'E',
      title: 'Scenario E: Non-Terminal Audit Logging Flow',
      description: 'Demonstrates non-terminal LOG taps recording observation telemetry without halting evaluation of subsequent rules.',
      defaultCount: 200,
      badge: 'Audit Tap',
    },
    {
      id: 'F',
      title: 'Scenario F: Configuration Persistence & Rollback',
      description: 'Tests transactional loading of rule configurations and validates automatic state rollback upon syntax errors.',
      defaultCount: 10,
      badge: 'Transactional',
    },
  ];

  const handleRun = async (scenarioId: string, count: number) => {
    try {
      setRunning(true);
      setError(null);
      const res = await api.runSimulation(scenarioId, count);
      setResult(res);
      if (onSimulationRun) onSimulationRun();
    } catch (err: any) {
      setError(err.message || 'Simulation run failed');
    } finally {
      setRunning(false);
    }
  };

  return (
    <div className="space-y-6">
      {/* Scenario Selection Grid */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        {scenarios.map(sc => {
          const isSelected = activeScenario === sc.id;
          return (
            <div
              key={sc.id}
              onClick={() => {
                setActiveScenario(sc.id);
                setPacketCount(sc.defaultCount);
              }}
              className={`p-5 rounded-card border transition-all cursor-pointer flex flex-col justify-between ${
                isSelected
                  ? 'bg-card border-primary shadow-hover ring-1 ring-primary'
                  : 'bg-card border-border-subtle hover:border-border-hover shadow-card'
              }`}
            >
              <div>
                <div className="flex items-center justify-between mb-2">
                  <span className="text-[11px] font-bold text-primary px-2 py-0.5 rounded bg-primary-light">
                    {sc.badge}
                  </span>
                  <span className="text-xs font-mono font-semibold text-text-secondary">
                    #{sc.id}
                  </span>
                </div>
                <h4 className="font-bold text-sm text-text-primary tracking-tight">
                  {sc.title}
                </h4>
                <p className="text-xs text-text-secondary mt-1.5 line-clamp-3">
                  {sc.description}
                </p>
              </div>

              <div className="pt-4 mt-4 border-t border-border-subtle flex items-center justify-between">
                <span className="text-[11px] text-text-muted font-mono">
                  Default: {sc.defaultCount.toLocaleString()} pkts
                </span>
                <button
                  type="button"
                  onClick={e => {
                    e.stopPropagation();
                    setActiveScenario(sc.id);
                    handleRun(sc.id, isSelected ? packetCount : sc.defaultCount);
                  }}
                  disabled={running}
                  className="inline-flex items-center space-x-1.5 px-3 py-1.5 bg-primary hover:bg-primary-hover text-white text-xs font-semibold rounded-lg shadow-sm transition-all disabled:opacity-50"
                >
                  <Play className={`w-3 h-3 fill-white ${running && activeScenario === sc.id ? 'animate-spin' : ''}`} />
                  <span>{running && activeScenario === sc.id ? 'Evaluating in C++...' : 'Run Live'}</span>
                </button>
              </div>
            </div>
          );
        })}
      </div>

      {/* Benchmark Results Display */}
      {error && (
        <div className="p-4 bg-rose-50 border border-rose-200 rounded-card text-xs text-rose-700">
          <p className="font-semibold">Execution Error:</p>
          <p className="mt-1">{error}</p>
        </div>
      )}

      {result && (
        <div className="bg-card border border-border-subtle rounded-card p-6 shadow-card space-y-6 animate-in fade-in duration-200">
          <div className="flex flex-col sm:flex-row sm:items-center justify-between pb-4 border-b border-border-subtle gap-2">
            <div>
              <div className="flex items-center space-x-2">
                <Zap className="w-5 h-5 text-amber-500 fill-amber-500" />
                <h3 className="text-base font-bold text-text-primary tracking-tight">
                  Live Simulation Execution Results — Scenario {result.scenario}
                </h3>
              </div>
              <p className="text-xs text-text-secondary mt-0.5">
                Evaluated through the C++ FirewallEngine native pipeline.
              </p>
            </div>
            <div className="flex items-center space-x-4">
              <span className="text-xs text-text-secondary">
                Duration:{' '}
                <strong className="text-text-primary font-mono">
                  {result.elapsedSeconds.toFixed(4)}s
                </strong>
              </span>
            </div>
          </div>

          {/* Metric KPI Cards */}
          <div className="grid grid-cols-2 lg:grid-cols-4 gap-4">
            <div className="bg-slate-50/70 p-4 rounded-xl border border-border-subtle">
              <div className="flex items-center space-x-2 text-text-secondary text-xs mb-1">
                <BarChart2 className="w-4 h-4 text-primary" />
                <span>Total Evaluated</span>
              </div>
              <p className="text-xl font-extrabold text-text-primary">
                {result.packetsProcessed.toLocaleString()}
              </p>
            </div>

            <div className="bg-slate-50/70 p-4 rounded-xl border border-border-subtle">
              <div className="flex items-center space-x-2 text-text-secondary text-xs mb-1">
                <TrendingUp className="w-4 h-4 text-allow-text" />
                <span>Evaluated Throughput</span>
              </div>
              <p className="text-xl font-extrabold text-text-primary font-mono">
                {result.throughputPps >= 1000000
                  ? `${(result.throughputPps / 1000000).toFixed(2)} Mpps`
                  : `${Math.round(result.throughputPps).toLocaleString()} pkts/s`}
              </p>
            </div>

            <div className="bg-slate-50/70 p-4 rounded-xl border border-border-subtle">
              <div className="flex items-center space-x-2 text-text-secondary text-xs mb-1">
                <CheckCircle2 className="w-4 h-4 text-allow-text" />
                <span>Allowed Packets</span>
              </div>
              <p className="text-xl font-extrabold text-allow-text">
                {result.packetsAllowed.toLocaleString()}
              </p>
            </div>

            <div className="bg-slate-50/70 p-4 rounded-xl border border-border-subtle">
              <div className="flex items-center space-x-2 text-text-secondary text-xs mb-1">
                <Ban className="w-4 h-4 text-block-text" />
                <span>Blocked / Rejected</span>
              </div>
              <p className="text-xl font-extrabold text-block-text">
                {(result.packetsBlocked + result.packetsRejected).toLocaleString()}
              </p>
            </div>
          </div>

          {/* Rule Hits Table */}
          {result.ruleHitCounts && Object.keys(result.ruleHitCounts).length > 0 && (
            <div>
              <h4 className="text-xs font-bold text-text-primary uppercase tracking-wider mb-3">
                Rule Policy Match Tallies
              </h4>
              <div className="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-6 gap-3">
                {Object.entries(result.ruleHitCounts).map(([rId, hits]) => (
                  <div key={rId} className="bg-slate-50 p-3 rounded-lg border border-border-subtle text-center">
                    <span className="text-[11px] font-mono text-text-secondary">Rule #{rId}</span>
                    <p className="text-sm font-bold text-primary mt-0.5">{hits.toLocaleString()}</p>
                  </div>
                ))}
              </div>
            </div>
          )}
        </div>
      )}
    </div>
  );
};
