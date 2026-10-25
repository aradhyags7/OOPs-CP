import React from 'react';
import { BarChart, Bar, XAxis, Tooltip, ResponsiveContainer } from 'recharts';
import type { RuleHitItem } from '../types';

interface TopRulesBarChartProps {
  topRules: RuleHitItem[];
  onViewAll?: () => void;
}

export const TopRulesBarChart: React.FC<TopRulesBarChartProps> = ({
  topRules,
  onViewAll,
}) => {
  const hasData = topRules && topRules.length > 0 && topRules.some(r => r.hits > 0);

  // Custom X-axis tick showing Rule ID and formatted hit count
  const renderCustomAxisTick = ({ x, y, payload }: any) => {
    const item = topRules.find(r => r.ruleId === payload.value) || {
      ruleId: payload.value,
      label: '0',
    };

    return (
      <g transform={`translate(${x},${y})`}>
        <text
          x={0}
          y={12}
          dy={0}
          textAnchor="middle"
          fill="#111C36"
          fontSize={11}
          fontWeight={600}
        >
          {item.ruleId}
        </text>
        <text
          x={0}
          y={26}
          dy={0}
          textAnchor="middle"
          fill="#64748B"
          fontSize={10}
          fontWeight={400}
        >
          {item.label}
        </text>
      </g>
    );
  };

  return (
    <div className="bg-card border border-border-subtle rounded-card p-6 shadow-card flex flex-col justify-between">
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-[15px] font-bold text-text-primary tracking-tight">
          Top Rules by Hits
        </h3>
        {onViewAll && (
          <button
            onClick={onViewAll}
            className="text-xs text-primary font-semibold hover:underline cursor-pointer"
          >
            View All
          </button>
        )}
      </div>

      <div className="w-full h-48 my-auto">
        {hasData ? (
          <ResponsiveContainer width="100%" height="100%">
            <BarChart
              data={topRules}
              margin={{ top: 12, right: 10, left: 10, bottom: 20 }}
            >
              <XAxis
                dataKey="ruleId"
                interval={0}
                tick={renderCustomAxisTick}
                axisLine={{ stroke: '#E2E8F0' }}
                tickLine={false}
              />
              <Tooltip
                cursor={{ fill: '#F1F5F9' }}
                content={({ active, payload }) => {
                  if (active && payload && payload.length) {
                    const data = payload[0].payload as RuleHitItem;
                    return (
                      <div className="bg-card border border-border-subtle rounded-lg p-2.5 shadow-md text-xs">
                        <p className="font-bold text-text-primary">{data.ruleId} - {data.name}</p>
                        <p className="text-text-secondary mt-0.5">
                          Hit Count: <span className="font-semibold text-primary">{data.hits.toLocaleString()}</span>
                        </p>
                      </div>
                    );
                  }
                  return null;
                }}
              />
              <Bar
                dataKey="hits"
                fill="#60A5FA"
                radius={[4, 4, 0, 0]}
                maxBarSize={36}
              />
            </BarChart>
          </ResponsiveContainer>
        ) : (
          <div className="w-full h-full flex flex-col items-center justify-center text-center p-4">
            <p className="text-xs font-medium text-text-secondary">
              No rule hits recorded yet.
            </p>
            <p className="text-[11px] text-text-muted mt-1">
              Run traffic or inspect packets to populate hit counters.
            </p>
          </div>
        )}
      </div>
    </div>
  );
};
