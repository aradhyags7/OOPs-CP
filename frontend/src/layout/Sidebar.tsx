import React from 'react';
import { 
  Shield, 
  Home, 
  List, 
  Play, 
  FileText, 
  BarChart2
} from 'lucide-react';

export type NavItem = 'overview' | 'rules' | 'inspector' | 'audit' | 'simulations';

interface SidebarProps {
  activeTab: NavItem;
  setActiveTab: (tab: NavItem) => void;
  engineConnected: boolean;
  version?: string;
}

export const Sidebar: React.FC<SidebarProps> = ({
  activeTab,
  setActiveTab,
  engineConnected,
  version = 'v1.0.0'
}) => {
  const navItems = [
    { id: 'overview' as NavItem, label: 'Overview', icon: Home },
    { id: 'rules' as NavItem, label: 'Rules', icon: List },
    { id: 'inspector' as NavItem, label: 'Packet Inspector', icon: Play },
    { id: 'audit' as NavItem, label: 'Audit Logs', icon: FileText },
    { id: 'simulations' as NavItem, label: 'Simulations', icon: BarChart2 },
  ];

  return (
    <aside className="w-60 min-h-screen bg-sidebar border-r border-border-subtle flex flex-col justify-between p-6 select-none flex-shrink-0">
      <div>
        {/* Brand Header */}
        <div className="flex items-center space-x-3 mb-9 px-1">
          <div className="w-10 h-10 rounded-lg bg-primary/10 flex items-center justify-center text-primary flex-shrink-0 shadow-sm">
            <Shield className="w-6 h-6 fill-primary/20 stroke-primary stroke-[2.2]" />
          </div>
          <div>
            <h1 className="font-bold text-[17px] text-text-primary tracking-tight leading-tight">Firewall</h1>
            <p className="text-xs text-text-secondary font-normal">Control Center</p>
          </div>
        </div>

        {/* Navigation Items */}
        <nav className="space-y-1.5">
          {navItems.map((item) => {
            const Icon = item.icon;
            const isActive = activeTab === item.id;
            return (
              <button
                key={item.id}
                onClick={() => setActiveTab(item.id)}
                className={`w-full flex items-center space-x-3 px-3.5 py-2.5 rounded-lg text-sm font-medium transition-all duration-150 ${
                  isActive
                    ? 'bg-primary-light text-primary font-semibold shadow-sm'
                    : 'text-text-secondary hover:bg-slate-200/50 hover:text-text-primary'
                }`}
              >
                <Icon
                  className={`w-4 h-4 transition-colors ${
                    isActive ? 'text-primary stroke-[2.3]' : 'text-text-secondary'
                  }`}
                />
                <span>{item.label}</span>
              </button>
            );
          })}
        </nav>
      </div>

      {/* Footer Status */}
      <div className="pt-6 border-t border-border-subtle/80 px-1">
        <div className="flex items-center space-x-2">
          <span
            className={`w-2 h-2 rounded-full ${
              engineConnected ? 'bg-allow animate-pulse' : 'bg-block'
            }`}
          />
          <span className="text-xs font-medium text-text-primary">
            {engineConnected ? 'Engine Connected' : 'Engine Disconnected'}
          </span>
        </div>
        <p className="text-[11px] text-text-muted mt-0.5 ml-4 font-mono">{version}</p>
      </div>
    </aside>
  );
};
