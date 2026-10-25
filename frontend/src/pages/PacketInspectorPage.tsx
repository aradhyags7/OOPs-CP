import React, { useState } from 'react';
import { 
  Play, 
  ShieldCheck, 
  ShieldAlert, 
  ShieldX, 
  Cpu, 
  ArrowRight,
  Sparkles,
  Info
} from 'lucide-react';
import { api } from '../api/client';
import type { PacketInspectResponse } from '../types';

interface PacketInspectorPageProps {
  onEvaluationComplete?: () => void;
}

export const PacketInspectorPage: React.FC<PacketInspectorPageProps> = ({
  onEvaluationComplete,
}) => {
  const [formData, setFormData] = useState({
    srcIP: '10.0.1.50',
    dstIP: '192.0.2.1',
    srcPort: 51234,
    dstPort: 443,
    protocol: 'TCP',
    direction: 'INBOUND',
  });

  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState<PacketInspectResponse | null>(null);
  const [error, setError] = useState<string | null>(null);

  const presets = [
    {
      label: 'HTTPS Web (443)',
      data: { srcIP: '10.0.1.25', dstIP: '192.0.2.1', srcPort: 51234, dstPort: 443, protocol: 'TCP', direction: 'INBOUND' }
    },
    {
      label: 'DNS Query (53)',
      data: { srcIP: '192.168.1.50', dstIP: '8.8.8.8', srcPort: 53421, dstPort: 53, protocol: 'UDP', direction: 'OUTBOUND' }
    },
    {
      label: 'Telnet Inbound (23)',
      data: { srcIP: '198.51.100.5', dstIP: '10.0.0.1', srcPort: 48112, dstPort: 23, protocol: 'TCP', direction: 'INBOUND' }
    },
    {
      label: 'Office Subnet Host',
      data: { srcIP: '10.0.5.20', dstIP: '192.0.2.1', srcPort: 5000, dstPort: 8080, protocol: 'TCP', direction: 'INBOUND' }
    },
    {
      label: 'Rogue Subnet Host',
      data: { srcIP: '192.168.100.25', dstIP: '10.0.0.1', srcPort: 55000, dstPort: 80, protocol: 'TCP', direction: 'INBOUND' }
    },
    {
      label: 'ICMP Ping',
      data: { srcIP: '172.16.0.10', dstIP: '10.0.0.1', srcPort: 0, dstPort: 0, protocol: 'ICMP', direction: 'INBOUND' }
    }
  ];

  const handleInspect = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      setLoading(true);
      setError(null);
      const res = await api.inspectPacket(formData);
      setResult(res);
      if (onEvaluationComplete) onEvaluationComplete();
    } catch (err: any) {
      setError(err.message || 'Inspection failed');
    } finally {
      setLoading(false);
    }
  };

  const applyPreset = (presetData: typeof formData) => {
    setFormData(presetData);
    setResult(null);
  };

  const getDecisionTheme = (decision: string) => {
    switch (decision) {
      case 'ALLOWED':
        return {
          icon: ShieldCheck,
          bg: 'bg-allow-bg border-allow/30 text-allow-text',
          badge: 'bg-allow text-white',
          title: 'PACKET ALLOWED',
        };
      case 'REJECTED':
        return {
          icon: ShieldAlert,
          bg: 'bg-reject-bg border-reject/30 text-reject-text',
          badge: 'bg-reject text-white',
          title: 'PACKET REJECTED (ICMP Unreachable)',
        };
      case 'BLOCKED':
      default:
        return {
          icon: ShieldX,
          bg: 'bg-block-bg border-block/30 text-block-text',
          badge: 'bg-block text-white',
          title: 'PACKET BLOCKED (Silently Dropped)',
        };
    }
  };

  return (
    <div className="space-y-6">
      {/* Information Banner */}
      <div className="bg-blue-50/70 border border-blue-100 rounded-card p-4 flex items-start space-x-3 text-xs text-text-secondary">
        <Info className="w-4 h-4 text-primary flex-shrink-0 mt-0.5" />
        <div>
          <span className="font-semibold text-text-primary">Simulated User-Space Packet Inspection:</span>{' '}
          This tool constructs a synthetic packet object and executes the C++ engine's{' '}
          <code className="text-primary font-semibold">FirewallEngine::processPacket()</code>{' '}
          pipeline with live priority traversal and CIDR bitmask evaluation.
        </div>
      </div>

      {/* Main Grid: Form Left, Presets & Results Right */}
      <div className="grid grid-cols-1 lg:grid-cols-12 gap-6">
        {/* Left Form (7 cols) */}
        <div className="lg:col-span-7 bg-card border border-border-subtle rounded-card p-6 shadow-card">
          <div className="flex items-center space-x-2 pb-4 border-b border-border-subtle mb-5">
            <Cpu className="w-5 h-5 text-primary" />
            <h3 className="text-sm font-bold text-text-primary">
              Synthetic Packet Parameters
            </h3>
          </div>

          {/* Quick Presets */}
          <div className="mb-5">
            <div className="flex items-center space-x-1.5 mb-2">
              <Sparkles className="w-3.5 h-3.5 text-primary" />
              <span className="text-[11px] font-semibold text-text-secondary uppercase tracking-wider">
                Quick Test Scenarios
              </span>
            </div>
            <div className="flex flex-wrap gap-2">
              {presets.map((p, idx) => (
                <button
                  key={idx}
                  type="button"
                  onClick={() => applyPreset(p.data)}
                  className="px-2.5 py-1 rounded bg-slate-100 hover:bg-primary-light hover:text-primary text-text-secondary text-[11px] font-medium transition-colors"
                >
                  {p.label}
                </button>
              ))}
            </div>
          </div>

          <form onSubmit={handleInspect} className="space-y-4 text-xs">
            {/* IP Addresses */}
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              <div>
                <label className="block font-medium text-text-secondary mb-1">
                  Source IPv4 Address
                </label>
                <input
                  type="text"
                  value={formData.srcIP}
                  onChange={e => setFormData({ ...formData, srcIP: e.target.value })}
                  placeholder="e.g. 192.168.1.50"
                  className="w-full px-3 py-2 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-mono focus:ring-1 focus:ring-primary"
                  required
                />
              </div>
              <div>
                <label className="block font-medium text-text-secondary mb-1">
                  Destination IPv4 Address
                </label>
                <input
                  type="text"
                  value={formData.dstIP}
                  onChange={e => setFormData({ ...formData, dstIP: e.target.value })}
                  placeholder="e.g. 192.0.2.1"
                  className="w-full px-3 py-2 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-mono focus:ring-1 focus:ring-primary"
                  required
                />
              </div>
            </div>

            {/* Ports */}
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              <div>
                <label className="block font-medium text-text-secondary mb-1">
                  Source Transport Port (0-65535)
                </label>
                <input
                  type="number"
                  value={formData.srcPort}
                  onChange={e => setFormData({ ...formData, srcPort: Number(e.target.value) })}
                  className="w-full px-3 py-2 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-mono focus:ring-1 focus:ring-primary"
                  disabled={formData.protocol === 'ICMP'}
                />
              </div>
              <div>
                <label className="block font-medium text-text-secondary mb-1">
                  Destination Transport Port (0-65535)
                </label>
                <input
                  type="number"
                  value={formData.dstPort}
                  onChange={e => setFormData({ ...formData, dstPort: Number(e.target.value) })}
                  className="w-full px-3 py-2 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-mono focus:ring-1 focus:ring-primary"
                  disabled={formData.protocol === 'ICMP'}
                />
              </div>
            </div>

            {/* Protocol & Direction */}
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              <div>
                <label className="block font-medium text-text-secondary mb-1">
                  Transport Protocol
                </label>
                <select
                  value={formData.protocol}
                  onChange={e => setFormData({ ...formData, protocol: e.target.value })}
                  className="w-full px-3 py-2 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-semibold focus:ring-1 focus:ring-primary"
                >
                  <option value="TCP">TCP (Transmission Control Protocol)</option>
                  <option value="UDP">UDP (User Datagram Protocol)</option>
                  <option value="ICMP">ICMP (Internet Control Message Protocol)</option>
                </select>
              </div>
              <div>
                <label className="block font-medium text-text-secondary mb-1">
                  Traffic Direction
                </label>
                <select
                  value={formData.direction}
                  onChange={e => setFormData({ ...formData, direction: e.target.value })}
                  className="w-full px-3 py-2 bg-slate-50 border border-border-subtle rounded-lg text-text-primary focus:ring-1 focus:ring-primary"
                >
                  <option value="INBOUND">INBOUND (External to Internal)</option>
                  <option value="OUTBOUND">OUTBOUND (Internal to External)</option>
                </select>
              </div>
            </div>

            <div className="pt-2">
              <button
                type="submit"
                disabled={loading}
                className="w-full py-2.5 px-4 bg-primary hover:bg-primary-hover text-white rounded-lg font-semibold shadow-sm transition-all flex items-center justify-center space-x-2 disabled:opacity-60"
              >
                <Play className={`w-4 h-4 fill-white ${loading ? 'animate-spin' : ''}`} />
                <span>{loading ? 'Evaluating in C++ Engine...' : 'Inspect Packet via C++ Engine'}</span>
              </button>
            </div>
          </form>
        </div>

        {/* Right Output Verdict (5 cols) */}
        <div className="lg:col-span-5 space-y-4">
          {error && (
            <div className="p-4 bg-rose-50 border border-rose-200 rounded-card text-xs text-rose-700">
              <p className="font-semibold">Inspection Error:</p>
              <p className="mt-1">{error}</p>
            </div>
          )}

          {result ? (
            (() => {
              const theme = getDecisionTheme(result.decision);
              const ThemeIcon = theme.icon;

              return (
                <div className="bg-card border border-border-subtle rounded-card p-6 shadow-card space-y-5 animate-in fade-in duration-200">
                  <div className="flex items-center justify-between pb-3 border-b border-border-subtle">
                    <span className="text-xs font-semibold text-text-secondary uppercase tracking-wider">
                      Evaluation Verdict
                    </span>
                    <span className="text-[11px] font-mono text-text-muted">
                      Engine Traversal: Complete
                    </span>
                  </div>

                  {/* Big Banner */}
                  <div className={`p-4 rounded-xl border flex items-center space-x-3.5 ${theme.bg}`}>
                    <ThemeIcon className="w-8 h-8 flex-shrink-0" />
                    <div>
                      <h4 className="font-extrabold text-sm tracking-wide">
                        {theme.title}
                      </h4>
                      <p className="text-xs opacity-90 mt-0.5">
                        {result.reason}
                      </p>
                    </div>
                  </div>

                  {/* Decision Details */}
                  <div className="space-y-3 text-xs bg-slate-50/70 p-4 rounded-lg border border-border-subtle">
                    <div className="flex items-center justify-between py-1 border-b border-border-subtle/50">
                      <span className="text-text-secondary font-medium">Terminal Verdict:</span>
                      <span className={`px-2 py-0.5 rounded font-bold text-[11px] ${theme.badge}`}>
                        {result.decision}
                      </span>
                    </div>

                    <div className="flex items-center justify-between py-1 border-b border-border-subtle/50">
                      <span className="text-text-secondary font-medium">Matched Rule ID:</span>
                      <span className="font-mono font-semibold text-text-primary">
                        {result.matchedRuleId !== -1 ? `#${result.matchedRuleId}` : 'None (Default Fallback)'}
                      </span>
                    </div>

                    <div className="flex items-center justify-between py-1 border-b border-border-subtle/50">
                      <span className="text-text-secondary font-medium">Rule Policy Name:</span>
                      <span className="font-semibold text-text-primary">
                        {result.matchedRuleName}
                      </span>
                    </div>

                    {result.loggedRuleIds && result.loggedRuleIds.length > 0 && (
                      <div className="flex items-center justify-between py-1">
                        <span className="text-text-secondary font-medium">Non-Terminal Log Taps:</span>
                        <div className="flex space-x-1">
                          {result.loggedRuleIds.map(id => (
                            <span key={id} className="px-1.5 py-0.5 bg-blue-100 text-primary rounded font-mono font-bold text-[10px]">
                              Rule #{id}
                            </span>
                          ))}
                        </div>
                      </div>
                    )}
                  </div>

                  {/* Packet Header Summary */}
                  <div className="text-[11px] font-mono text-text-secondary bg-slate-100/70 p-3 rounded border border-border-subtle flex items-center justify-between">
                    <span>
                      {result.packet.srcIP}:{result.packet.srcPort}
                    </span>
                    <ArrowRight className="w-3.5 h-3.5 text-text-muted" />
                    <span>
                      {result.packet.dstIP}:{result.packet.dstPort}
                    </span>
                    <span className="font-semibold text-text-primary">
                      [{result.packet.protocol} {result.packet.direction}]
                    </span>
                  </div>
                </div>
              );
            })()
          ) : (
            <div className="bg-card border border-border-subtle rounded-card p-8 shadow-card flex flex-col items-center justify-center text-center min-h-[300px] text-text-muted">
              <Play className="w-10 h-10 stroke-[1.2] mb-3 text-slate-300" />
              <p className="text-xs font-semibold text-text-secondary">
                Awaiting Packet Inspection
              </p>
              <p className="text-[11px] mt-1 max-w-xs">
                Enter packet parameters or select a quick test scenario on the left, then click Inspect.
              </p>
            </div>
          )}
        </div>
      </div>
    </div>
  );
};
