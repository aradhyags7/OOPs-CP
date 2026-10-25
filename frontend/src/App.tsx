import React, { useState, useEffect, useCallback, useRef } from 'react';
import { Sidebar } from './layout/Sidebar';
import type { NavItem } from './layout/Sidebar';
import { Header } from './layout/Header';
import type { ConnectionStatus } from './layout/Header';
import { OverviewPage } from './pages/OverviewPage';
import { RulesPage } from './pages/RulesPage';
import { PacketInspectorPage } from './pages/PacketInspectorPage';
import { AuditLogsPage } from './pages/AuditLogsPage';
import { SimulationsPage } from './pages/SimulationsPage';
import { api } from './api/client';
import type { DashboardData, HealthResponse } from './types';
import { AlertCircle } from 'lucide-react';

export const App: React.FC = () => {
  const [activeTab, setActiveTab] = useState<NavItem>('overview');
  const [dashboardData, setDashboardData] = useState<DashboardData | null>(null);
  const [health, setHealth] = useState<HealthResponse | null>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [refreshing, setRefreshing] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [lastUpdated, setLastUpdated] = useState<string | null>(null);
  const [connectionStatus, setConnectionStatus] = useState<ConnectionStatus>('updating');

  // Prevent overlapping concurrent requests
  const isFetchingRef = useRef<boolean>(false);

  const fetchData = useCallback(async (isManualRefresh: boolean = false) => {
    if (isFetchingRef.current) return;
    isFetchingRef.current = true;

    try {
      if (isManualRefresh) {
        setRefreshing(true);
        setConnectionStatus('updating');
      }

      const [healthRes, dashRes] = await Promise.all([
        api.getHealth().catch(() => null),
        api.getDashboard().catch(() => null),
      ]);

      if (healthRes && dashRes) {
        setHealth(healthRes);
        setDashboardData(dashRes);
        setError(null);
        setConnectionStatus('connected');
        setLastUpdated(new Date().toLocaleTimeString());
      } else if (healthRes) {
        setHealth(healthRes);
        setConnectionStatus('connected');
      } else {
        setHealth(null);
        setConnectionStatus('disconnected');
        setError('Cannot connect to the Firewall Rule Engine backend. Ensure the Python API adapter is running on port 8000.');
      }
    } catch (err: any) {
      setConnectionStatus('disconnected');
      setError(err.message || 'Error communicating with engine');
    } finally {
      setLoading(false);
      setRefreshing(false);
      isFetchingRef.current = false;
    }
  }, []);

  // Initial fetch on mount
  useEffect(() => {
    fetchData();
  }, [fetchData]);

  // Automated live polling loop (every 2.5 seconds when page is visible)
  useEffect(() => {
    let intervalId: any = null;

    const startPolling = () => {
      if (!intervalId) {
        intervalId = setInterval(() => {
          if (document.visibilityState === 'visible') {
            fetchData(false);
          }
        }, 2500);
      }
    };

    const stopPolling = () => {
      if (intervalId) {
        clearInterval(intervalId);
        intervalId = null;
      }
    };

    const handleVisibilityChange = () => {
      if (document.visibilityState === 'visible') {
        fetchData(false);
        startPolling();
      } else {
        stopPolling();
      }
    };

    startPolling();
    document.addEventListener('visibilitychange', handleVisibilityChange);

    return () => {
      stopPolling();
      document.removeEventListener('visibilitychange', handleVisibilityChange);
    };
  }, [fetchData]);

  // Tab Header Metadata
  const getHeaderInfo = () => {
    switch (activeTab) {
      case 'overview':
        return {
          title: 'Overview',
          subtitle: 'Real-time view of your firewall rule engine',
        };
      case 'rules':
        return {
          title: 'Firewall Rules',
          subtitle: 'Manage and inspect active security policies and condition chains',
        };
      case 'inspector':
        return {
          title: 'Packet Inspector',
          subtitle: 'Simulate and test individual packets against the C++ engine',
        };
      case 'audit':
        return {
          title: 'Audit Decision Logs',
          subtitle: 'Comprehensive record of evaluated network packets and rule hits',
        };
      case 'simulations':
        return {
          title: 'Demonstration Scenarios',
          subtitle: 'Run automated evaluation suites and high-volume batch benchmarks',
        };
    }
  };

  const headerInfo = getHeaderInfo();
  const isEngineOnline = connectionStatus !== 'disconnected';

  return (
    <div className="flex min-h-screen bg-page text-text-primary antialiased">
      {/* Left Sidebar */}
      <Sidebar
        activeTab={activeTab}
        setActiveTab={setActiveTab}
        engineConnected={isEngineOnline}
        version={health?.version || 'v1.0.0'}
      />

      {/* Main Content Area */}
      <main className="flex-1 flex flex-col min-w-0 overflow-y-auto">
        <div className="max-w-[1340px] w-full mx-auto p-8 sm:p-9">
          {/* Header with Connection Pill & Live Time */}
          <Header
            title={headerInfo.title}
            subtitle={headerInfo.subtitle}
            connectionStatus={connectionStatus}
            lastUpdated={lastUpdated}
            onRefresh={() => fetchData(true)}
            isRefreshing={refreshing}
          />

          {/* Connection Error Banner */}
          {error && (
            <div className="mb-6 p-4 rounded-card bg-rose-50 border border-rose-200 text-rose-800 text-xs flex items-center justify-between animate-in fade-in duration-200">
              <div className="flex items-center space-x-2.5">
                <AlertCircle className="w-4 h-4 flex-shrink-0 text-rose-600" />
                <span>{error}</span>
              </div>
              <button
                onClick={() => fetchData(true)}
                className="px-3 py-1 bg-white border border-rose-200 rounded text-rose-700 font-semibold hover:bg-rose-100 transition-colors"
              >
                Retry Connection
              </button>
            </div>
          )}

          {/* Page Routing */}
          <div className="transition-opacity duration-150">
            {activeTab === 'overview' && (
              <OverviewPage
                data={dashboardData}
                loading={loading}
                onNavigateTab={(tab) => setActiveTab(tab)}
              />
            )}

            {activeTab === 'rules' && (
              <RulesPage onRefreshParent={() => fetchData(false)} />
            )}

            {activeTab === 'inspector' && (
              <PacketInspectorPage onEvaluationComplete={() => fetchData(false)} />
            )}

            {activeTab === 'audit' && <AuditLogsPage />}

            {activeTab === 'simulations' && (
              <SimulationsPage onSimulationRun={() => fetchData(false)} />
            )}
          </div>
        </div>
      </main>
    </div>
  );
};

export default App;
