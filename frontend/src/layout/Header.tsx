import React from 'react';
import { RotateCw } from 'lucide-react';

export type ConnectionStatus = 'connected' | 'disconnected' | 'updating';

interface HeaderProps {
  title: string;
  subtitle: string;
  connectionStatus: ConnectionStatus;
  lastUpdated: string | null;
  onRefresh: () => void;
  isRefreshing?: boolean;
}

export const Header: React.FC<HeaderProps> = ({
  title,
  subtitle,
  connectionStatus,
  lastUpdated,
  onRefresh,
  isRefreshing = false,
}) => {
  const getStatusBadge = () => {
    switch (connectionStatus) {
      case 'updating':
        return {
          dot: 'bg-primary animate-ping',
          text: 'text-primary',
          label: 'Updating...',
        };
      case 'connected':
        return {
          dot: 'bg-allow',
          text: 'text-text-primary',
          label: 'Connected',
        };
      case 'disconnected':
      default:
        return {
          dot: 'bg-block',
          text: 'text-block',
          label: 'Disconnected',
        };
    }
  };

  const status = getStatusBadge();

  return (
    <header className="flex flex-col sm:flex-row sm:items-center sm:justify-between pb-7 pt-1 gap-4">
      <div>
        <h2 className="text-[26px] font-bold text-text-primary tracking-tight leading-tight">
          {title}
        </h2>
        <p className="text-sm text-text-secondary font-normal mt-0.5">
          {subtitle}
        </p>
      </div>

      <div className="flex items-center space-x-4">
        {/* Engine Status & Last Updated */}
        <div className="flex flex-col items-end pr-1 text-right">
          <span className="text-[11px] font-medium text-text-muted uppercase tracking-wider">
            Engine State
          </span>
          <div className="flex items-center space-x-1.5 mt-0.5">
            <span className="relative flex h-2 w-2">
              {connectionStatus === 'updating' && (
                <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-primary opacity-75" />
              )}
              <span className={`relative inline-flex rounded-full h-2 w-2 ${status.dot}`} />
            </span>
            <span className={`text-xs font-semibold ${status.text}`}>
              {status.label}
            </span>
          </div>
          {lastUpdated && (
            <span className="text-[10px] text-text-muted font-mono mt-0.5">
              Sync: {lastUpdated}
            </span>
          )}
        </div>

        {/* Refresh Button */}
        <button
          onClick={onRefresh}
          disabled={isRefreshing}
          className="inline-flex items-center space-x-2 px-3.5 py-2 rounded-lg bg-card border border-border-subtle hover:border-border-hover hover:bg-slate-50 text-text-primary text-xs font-semibold shadow-sm transition-all duration-150 disabled:opacity-60"
          title="Manual Refresh from C++ Engine"
        >
          <RotateCw
            className={`w-3.5 h-3.5 text-primary ${isRefreshing || connectionStatus === 'updating' ? 'animate-spin' : ''}`}
          />
          <span>Refresh</span>
        </button>
      </div>
    </header>
  );
};
