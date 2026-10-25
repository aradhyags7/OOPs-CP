import React from 'react';
import type { LucideIcon } from 'lucide-react';

interface StatCardProps {
  title: string;
  value: string | number;
  subtext?: string;
  icon: LucideIcon;
  iconBgColor: string;
  iconColor: string;
  progressPercent?: number;
  progressBarColor?: string;
  trendText?: string;
  trendPositive?: boolean;
}

export const StatCard: React.FC<StatCardProps> = ({
  title,
  value,
  subtext,
  icon: Icon,
  iconBgColor,
  iconColor,
  progressPercent,
  progressBarColor = 'bg-primary',
  trendText,
}) => {
  return (
    <div className="bg-card border border-border-subtle rounded-card p-5 shadow-card hover:shadow-hover transition-all duration-150 flex flex-col justify-between">
      <div className="flex items-start justify-between">
        <div className="flex items-center space-x-3.5">
          <div
            className={`w-10 h-10 rounded-lg flex items-center justify-center flex-shrink-0 ${iconBgColor}`}
          >
            <Icon className={`w-5 h-5 ${iconColor}`} />
          </div>
          <div>
            <p className="text-xs font-medium text-text-secondary">{title}</p>
            <h3 className="text-[23px] font-bold text-text-primary tracking-tight leading-tight mt-0.5">
              {typeof value === 'number' ? value.toLocaleString() : value}
            </h3>
          </div>
        </div>
      </div>

      <div className="mt-4 pt-1">
        {trendText && (
          <div className="flex items-center space-x-1 text-xs">
            <span className="font-semibold text-allow-text">{trendText}</span>
          </div>
        )}

        {subtext && !trendText && (
          <div className="flex items-center justify-between text-xs text-text-secondary mb-1.5 font-medium">
            <span>{subtext}</span>
          </div>
        )}

        {typeof progressPercent === 'number' && (
          <div className="w-full h-1.5 bg-slate-100 rounded-full overflow-hidden mt-1.5">
            <div
              className={`h-full rounded-full transition-all duration-500 ${progressBarColor}`}
              style={{ width: `${Math.min(100, Math.max(0, progressPercent))}%` }}
            />
          </div>
        )}
      </div>
    </div>
  );
};
