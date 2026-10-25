import React from 'react';
import { FileText, CheckCircle2, Ban, AlertTriangle } from 'lucide-react';
import type { DashboardData } from '../types';
import { StatCard } from '../components/StatCard';
import { PacketDecisionsDonut } from '../components/PacketDecisionsDonut';
import { TopRulesBarChart } from '../components/TopRulesBarChart';
import { RecentDecisionsTable } from '../components/RecentDecisionsTable';

interface OverviewPageProps {
  data: DashboardData | null;
  loading: boolean;
  onNavigateTab: (tab: 'rules' | 'audit' | 'inspector' | 'simulations') => void;
}

export const OverviewPage: React.FC<OverviewPageProps> = ({
  data,
  loading,
  onNavigateTab,
}) => {
  if (loading && !data) {
    return (
      <div className="flex items-center justify-center min-h-[400px]">
        <div className="flex flex-col items-center space-y-3">
          <div className="w-8 h-8 border-3 border-primary border-t-transparent rounded-full animate-spin" />
          <p className="text-xs font-medium text-text-secondary">Loading firewall engine metrics...</p>
        </div>
      </div>
    );
  }

  const summary = data?.summary || {
    totalPackets: 0,
    allowed: 0,
    allowedPercent: 0,
    blocked: 0,
    blockedPercent: 0,
    rejected: 0,
    rejectedPercent: 0,
    changeIndicator: 'Ready',
  };

  return (
    <div className="space-y-6">
      {/* 4 Summary Cards Grid */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        {/* Total Packets */}
        <StatCard
          title="Total Packets"
          value={summary.totalPackets}
          icon={FileText}
          iconBgColor="bg-blue-50/80"
          iconColor="text-primary"
          trendText={summary.changeIndicator}
        />

        {/* Allowed */}
        <StatCard
          title="Allowed"
          value={summary.allowed}
          subtext={`${summary.allowedPercent}%`}
          icon={CheckCircle2}
          iconBgColor="bg-allow-bg"
          iconColor="text-allow-text"
          progressPercent={summary.allowedPercent}
          progressBarColor="bg-allow"
        />

        {/* Blocked */}
        <StatCard
          title="Blocked"
          value={summary.blocked}
          subtext={`${summary.blockedPercent}%`}
          icon={Ban}
          iconBgColor="bg-block-bg"
          iconColor="text-block-text"
          progressPercent={summary.blockedPercent}
          progressBarColor="bg-block"
        />

        {/* Rejected */}
        <StatCard
          title="Rejected"
          value={summary.rejected}
          subtext={`${summary.rejectedPercent}%`}
          icon={AlertTriangle}
          iconBgColor="bg-reject-bg"
          iconColor="text-reject-text"
          progressPercent={summary.rejectedPercent}
          progressBarColor="bg-reject"
        />
      </div>

      {/* Analytical Visuals Grid: Donut + Bar Chart */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-4">
        <PacketDecisionsDonut
          totalPackets={summary.totalPackets}
          allowed={summary.allowed}
          allowedPercent={summary.allowedPercent}
          blocked={summary.blocked}
          blockedPercent={summary.blockedPercent}
          rejected={summary.rejected}
          rejectedPercent={summary.rejectedPercent}
        />

        <TopRulesBarChart
          topRules={data?.topRules || []}
          onViewAll={() => onNavigateTab('rules')}
        />
      </div>

      {/* Recent Packet Decisions Table */}
      <RecentDecisionsTable
        decisions={data?.recentDecisions || []}
        onViewAllLogs={() => onNavigateTab('audit')}
      />
    </div>
  );
};
