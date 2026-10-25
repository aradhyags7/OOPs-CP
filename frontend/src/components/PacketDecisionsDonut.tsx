import React from 'react';
import { PieChart, Pie, Cell, ResponsiveContainer, Tooltip } from 'recharts';

interface PacketDecisionsDonutProps {
  totalPackets: number;
  allowed: number;
  allowedPercent: number;
  blocked: number;
  blockedPercent: number;
  rejected: number;
  rejectedPercent: number;
}

export const PacketDecisionsDonut: React.FC<PacketDecisionsDonutProps> = ({
  totalPackets,
  allowed,
  allowedPercent,
  blocked,
  blockedPercent,
  rejected,
  rejectedPercent,
}) => {
  const data = [
    { name: 'Allowed', value: allowed, color: '#22B86A', percent: allowedPercent },
    { name: 'Blocked', value: blocked, color: '#E55353', percent: blockedPercent },
    { name: 'Rejected', value: rejected, color: '#F59E0B', percent: rejectedPercent },
  ];

  // If total is 0, show empty state
  const hasData = totalPackets > 0;
  const displayData = hasData
    ? data
    : [{ name: 'No Packets Evaluated', value: 1, color: '#E2E8F0', percent: 0 }];

  return (
    <div className="bg-card border border-border-subtle rounded-card p-6 shadow-card flex flex-col justify-between">
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-[15px] font-bold text-text-primary tracking-tight">
          Packet Decisions
        </h3>
      </div>

      <div className="flex flex-col sm:flex-row items-center justify-between gap-6 my-auto">
        {/* Donut Chart with Center Number */}
        <div className="relative w-48 h-48 flex-shrink-0 flex items-center justify-center">
          <ResponsiveContainer width="100%" height="100%">
            <PieChart>
              <Pie
                data={displayData}
                cx="50%"
                cy="50%"
                innerRadius={62}
                outerRadius={88}
                paddingAngle={hasData ? 2 : 0}
                dataKey="value"
                strokeWidth={0}
              >
                {displayData.map((entry, index) => (
                  <Cell key={`cell-${index}`} fill={entry.color} />
                ))}
              </Pie>
              {hasData && (
                <Tooltip
                  formatter={(val: any) => [`${Number(val || 0).toLocaleString()} pkts`, 'Volume']}
                  contentStyle={{
                    backgroundColor: '#FFFFFF',
                    borderColor: '#E2E8F0',
                    borderRadius: '8px',
                    fontSize: '12px',
                    boxShadow: '0 4px 6px -1px rgba(0,0,0,0.08)',
                  }}
                />
              )}
            </PieChart>
          </ResponsiveContainer>

          {/* Absolute Center Text */}
          <div className="absolute inset-0 flex flex-col items-center justify-center pointer-events-none">
            <span className="text-[20px] font-extrabold text-text-primary tracking-tight leading-none">
              {totalPackets.toLocaleString()}
            </span>
            <span className="text-[11px] text-text-secondary font-medium mt-1">
              packets
            </span>
          </div>
        </div>

        {/* Legend beside the donut */}
        <div className="w-full sm:w-56 space-y-3.5 pr-2">
          {/* Allowed */}
          <div className="flex items-center justify-between text-xs">
            <div className="flex items-center space-x-2.5">
              <span className="w-2.5 h-2.5 rounded-full bg-[#22B86A]" />
              <span className="font-medium text-text-primary">Allowed</span>
            </div>
            <div className="flex items-center space-x-3 text-right">
              <span className="font-semibold text-text-primary">
                {allowed.toLocaleString()}
              </span>
              <span className="text-text-secondary text-[11px] w-12 text-right">
                {allowedPercent}%
              </span>
            </div>
          </div>

          {/* Blocked */}
          <div className="flex items-center justify-between text-xs">
            <div className="flex items-center space-x-2.5">
              <span className="w-2.5 h-2.5 rounded-full bg-[#E55353]" />
              <span className="font-medium text-text-primary">Blocked</span>
            </div>
            <div className="flex items-center space-x-3 text-right">
              <span className="font-semibold text-text-primary">
                {blocked.toLocaleString()}
              </span>
              <span className="text-text-secondary text-[11px] w-12 text-right">
                {blockedPercent}%
              </span>
            </div>
          </div>

          {/* Rejected */}
          <div className="flex items-center justify-between text-xs">
            <div className="flex items-center space-x-2.5">
              <span className="w-2.5 h-2.5 rounded-full bg-[#F59E0B]" />
              <span className="font-medium text-text-primary">Rejected</span>
            </div>
            <div className="flex items-center space-x-3 text-right">
              <span className="font-semibold text-text-primary">
                {rejected.toLocaleString()}
              </span>
              <span className="text-text-secondary text-[11px] w-12 text-right">
                {rejectedPercent}%
              </span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
