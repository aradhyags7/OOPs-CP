import React from 'react';
import type { DecisionRecord } from '../types';

interface RecentDecisionsTableProps {
  decisions: DecisionRecord[];
  onViewAllLogs?: () => void;
}

export const RecentDecisionsTable: React.FC<RecentDecisionsTableProps> = ({
  decisions,
  onViewAllLogs,
}) => {
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

  return (
    <div className="bg-card border border-border-subtle rounded-card shadow-card overflow-hidden">
      <div className="flex items-center justify-between p-6 pb-4">
        <h3 className="text-[15px] font-bold text-text-primary tracking-tight">
          Recent Packet Decisions
        </h3>
        {onViewAllLogs && (
          <button
            onClick={onViewAllLogs}
            className="text-xs text-primary font-semibold hover:underline cursor-pointer"
          >
            View All Logs
          </button>
        )}
      </div>

      <div className="overflow-x-auto">
        <table className="w-full text-left border-collapse">
          <thead>
            <tr className="border-y border-border-subtle bg-slate-50/70 text-[11px] font-semibold text-text-secondary uppercase tracking-wider">
              <th className="py-3 px-6">Time</th>
              <th className="py-3 px-6">Source</th>
              <th className="py-3 px-6">Destination</th>
              <th className="py-3 px-6">Protocol</th>
              <th className="py-3 px-6">Rule</th>
              <th className="py-3 px-6 text-right">Decision</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-border-subtle text-xs">
            {decisions.length > 0 ? (
              decisions.map((record, index) => (
                <tr
                  key={`${record.packetId}-${record.time}-${index}`}
                  className="hover:bg-slate-50/60 transition-colors duration-100"
                >
                  <td className="py-3 px-6 text-text-secondary whitespace-nowrap font-mono text-[11.5px]">
                    {record.time || record.timestamp}
                  </td>
                  <td className="py-3 px-6 text-text-primary font-mono whitespace-nowrap">
                    {record.src}
                  </td>
                  <td className="py-3 px-6 text-text-primary font-mono whitespace-nowrap">
                    {record.dst}
                  </td>
                  <td className="py-3 px-6 font-semibold text-text-secondary whitespace-nowrap">
                    {record.protocol}
                  </td>
                  <td className="py-3 px-6 font-medium text-text-secondary whitespace-nowrap">
                    {record.rule}
                  </td>
                  <td className="py-3 px-6 text-right whitespace-nowrap">
                    <span
                      className={`inline-block px-3 py-0.5 rounded-full text-[10.5px] font-bold tracking-wide ${getBadgeStyle(
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
                <td colSpan={6} className="py-8 text-center text-text-secondary">
                  No packet decisions evaluated yet.
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};
