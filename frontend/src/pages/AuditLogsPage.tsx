import React, { useState, useEffect } from 'react';
import { 
  FileText, 
  Search, 
  Filter, 
  RotateCw,
  Download
} from 'lucide-react';
import { api } from '../api/client';
import type { DecisionRecord } from '../types';

export const AuditLogsPage: React.FC = () => {
  const [logs, setLogs] = useState<DecisionRecord[]>([]);
  const [totalCount, setTotalCount] = useState(0);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  // Filters
  const [decisionFilter, setDecisionFilter] = useState('ALL');
  const [protocolFilter, setProtocolFilter] = useState('ALL');
  const [search, setSearch] = useState('');

  const fetchLogs = async () => {
    try {
      setLoading(true);
      setError(null);
      const res = await api.getAuditLogs({
        decision: decisionFilter,
        protocol: protocolFilter,
        search: search.trim() || undefined,
        limit: 500,
      });
      setLogs(res.records || []);
      setTotalCount(res.totalRecords || 0);
    } catch (err: any) {
      setError(err.message || 'Failed to fetch audit logs');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchLogs();
  }, [decisionFilter, protocolFilter]);

  const handleSearchSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    fetchLogs();
  };

  const getBadgeStyle = (decision: string) => {
    switch (decision.toUpperCase()) {
      case 'ALLOW':
      case 'ALLOWED':
        return 'bg-allow-bg text-allow-text border border-allow/20';
      case 'BLOCK':
      case 'BLOCKED':
        return 'bg-block-bg text-block-text border border-block/20';
      case 'REJECT':
      case 'REJECTED':
        return 'bg-reject-bg text-reject-text border border-reject/20';
      default:
        return 'bg-slate-100 text-slate-700';
    }
  };

  const exportCsv = () => {
    if (logs.length === 0) return;
    const headers = ["Timestamp", "PacketID", "Protocol", "Direction", "Source", "Destination", "Decision", "Rule", "Reason"];
    const rows = logs.map(l => [
      l.timestamp,
      l.packetId,
      l.protocol,
      l.direction,
      l.src,
      l.dst,
      l.decision,
      `"${l.rule}"`,
      `"${l.reason}"`
    ]);
    const csvContent = "data:text/csv;charset=utf-8," + [headers.join(","), ...rows.map(e => e.join(","))].join("\n");
    const encodedUri = encodeURI(csvContent);
    const link = document.createElement("a");
    link.setAttribute("href", encodedUri);
    link.setAttribute("download", `firewall_audit_${new Date().toISOString().slice(0,10)}.csv`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  return (
    <div className="space-y-6">
      {/* Control and Filter Bar */}
      <div className="bg-card p-4 rounded-card border border-border-subtle shadow-card flex flex-col md:flex-row md:items-center justify-between gap-4">
        {/* Search Input */}
        <form onSubmit={handleSearchSubmit} className="relative flex-1 max-w-md">
          <Search className="w-4 h-4 absolute left-3.5 top-1/2 -translate-y-1/2 text-text-muted" />
          <input
            type="text"
            placeholder="Search by IP, Port, Rule ID, or Reason..."
            value={search}
            onChange={e => setSearch(e.target.value)}
            className="w-full pl-9 pr-4 py-2 bg-slate-50 border border-border-subtle rounded-lg text-xs text-text-primary placeholder:text-text-muted focus:outline-none focus:ring-1 focus:ring-primary"
          />
        </form>

        {/* Filter Dropdowns */}
        <div className="flex flex-wrap items-center gap-3">
          <div className="flex items-center space-x-1.5 text-xs text-text-secondary">
            <Filter className="w-3.5 h-3.5" />
            <span className="font-medium">Verdict:</span>
            <select
              value={decisionFilter}
              onChange={e => setDecisionFilter(e.target.value)}
              className="px-2.5 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-xs font-semibold text-text-primary"
            >
              <option value="ALL">All Verdicts</option>
              <option value="ALLOW">Allow</option>
              <option value="BLOCK">Block</option>
              <option value="REJECT">Reject</option>
            </select>
          </div>

          <div className="flex items-center space-x-1.5 text-xs text-text-secondary">
            <span className="font-medium">Protocol:</span>
            <select
              value={protocolFilter}
              onChange={e => setProtocolFilter(e.target.value)}
              className="px-2.5 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-xs font-semibold text-text-primary"
            >
              <option value="ALL">All Protocols</option>
              <option value="TCP">TCP</option>
              <option value="UDP">UDP</option>
              <option value="ICMP">ICMP</option>
            </select>
          </div>

          <button
            onClick={fetchLogs}
            disabled={loading}
            className="p-2 border border-border-subtle hover:bg-slate-50 rounded-lg text-text-secondary transition-all"
            title="Refresh Logs"
          >
            <RotateCw className={`w-3.5 h-3.5 ${loading ? 'animate-spin' : ''}`} />
          </button>

          <button
            onClick={exportCsv}
            disabled={logs.length === 0}
            className="inline-flex items-center space-x-1.5 px-3 py-1.5 bg-card border border-border-subtle hover:bg-slate-50 text-text-primary text-xs font-semibold rounded-lg shadow-sm transition-all disabled:opacity-50"
          >
            <Download className="w-3.5 h-3.5 text-primary" />
            <span>Export CSV</span>
          </button>
        </div>
      </div>

      {/* Audit Table Card */}
      <div className="bg-card border border-border-subtle rounded-card shadow-card overflow-hidden">
        <div className="px-6 py-4 border-b border-border-subtle flex items-center justify-between">
          <div className="flex items-center space-x-2">
            <FileText className="w-4 h-4 text-primary" />
            <h3 className="text-sm font-bold text-text-primary">
              Audit Decision Records ({totalCount})
            </h3>
          </div>
          <span className="text-xs text-text-secondary">
            Source: <code className="text-primary font-semibold">firewall_decisions.csv</code>
          </span>
        </div>

        {error && (
          <div className="p-4 bg-rose-50 border-b border-rose-100 text-xs text-rose-700">
            {error}
          </div>
        )}

        <div className="overflow-x-auto">
          <table className="w-full text-left border-collapse text-xs">
            <thead>
              <tr className="border-b border-border-subtle bg-slate-50/70 text-[11px] font-semibold text-text-secondary uppercase tracking-wider">
                <th className="py-3 px-6">Timestamp</th>
                <th className="py-3 px-6">Source</th>
                <th className="py-3 px-6">Destination</th>
                <th className="py-3 px-6">Protocol</th>
                <th className="py-3 px-6">Rule</th>
                <th className="py-3 px-6">Reason</th>
                <th className="py-3 px-6 text-right">Verdict</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-border-subtle">
              {loading ? (
                <tr>
                  <td colSpan={7} className="py-8 text-center text-text-secondary">
                    Loading audit records...
                  </td>
                </tr>
              ) : logs.length > 0 ? (
                logs.map((record, index) => (
                  <tr
                    key={`${record.packetId}-${record.timestamp}-${index}`}
                    className="hover:bg-slate-50/60 transition-colors"
                  >
                    <td className="py-3 px-6 font-mono text-text-secondary whitespace-nowrap text-[11px]">
                      {record.timestamp}
                    </td>
                    <td className="py-3 px-6 font-mono font-medium text-text-primary whitespace-nowrap">
                      {record.src}
                    </td>
                    <td className="py-3 px-6 font-mono font-medium text-text-primary whitespace-nowrap">
                      {record.dst}
                    </td>
                    <td className="py-3 px-6 font-semibold text-text-secondary whitespace-nowrap">
                      {record.protocol} [{record.direction}]
                    </td>
                    <td className="py-3 px-6 font-medium text-text-secondary whitespace-nowrap">
                      {record.rule}
                    </td>
                    <td className="py-3 px-6 text-text-secondary truncate max-w-xs">
                      {record.reason}
                    </td>
                    <td className="py-3 px-6 text-right whitespace-nowrap">
                      <span
                        className={`inline-block px-3 py-0.5 rounded-full text-[10.5px] font-bold ${getBadgeStyle(
                          record.decision
                        )}`}
                      >
                        {record.decision.toUpperCase().replace('ED', '')}
                      </span>
                    </td>
                  </tr>
                ))
              ) : (
                <tr>
                  <td colSpan={7} className="py-8 text-center text-text-secondary">
                    No audit records match the selected filters.
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
};
