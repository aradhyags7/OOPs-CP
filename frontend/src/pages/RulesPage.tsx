import React, { useState, useEffect } from 'react';
import { 
  Plus, 
  Trash2, 
  Search, 
  ShieldCheck, 
  AlertCircle,
  Check,
  X,
  Pencil,
  Copy,
  RotateCw
} from 'lucide-react';
import { api } from '../api/client';
import type { FirewallRuleItem } from '../types';

interface RulesPageProps {
  onRefreshParent?: () => void;
}

function parseRuleConditions(condStr: string) {
  let proto = 'ANY';
  if (condStr.includes('PROTO=TCP')) proto = 'TCP';
  else if (condStr.includes('PROTO=UDP')) proto = 'UDP';
  else if (condStr.includes('PROTO=ICMP')) proto = 'ICMP';

  let port = 0;
  const portMatch = condStr.match(/PORT\((?:DST|SRC)\)=(\d+)/);
  if (portMatch) port = parseInt(portMatch[1], 10);

  let dir = 'ANY';
  if (condStr.includes('DIR=INBOUND')) dir = 'INBOUND';
  else if (condStr.includes('DIR=OUTBOUND')) dir = 'OUTBOUND';

  let cidr = 'ANY';
  const ipMatch = condStr.match(/IP\((?:SRC|DST)\)=([^\s,]+)/);
  if (ipMatch) cidr = ipMatch[1];

  return { proto, port, dir, cidr };
}

export const RulesPage: React.FC<RulesPageProps> = ({ onRefreshParent }) => {
  const [rules, setRules] = useState<FirewallRuleItem[]>([]);
  const [defaultPolicy, setDefaultPolicy] = useState('BLOCKED');
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [search, setSearch] = useState('');
  const [actionFilter, setActionFilter] = useState('ALL');
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [modalMode, setModalMode] = useState<'add' | 'edit' | 'duplicate'>('add');
  const [submitting, setSubmitting] = useState(false);

  // Form State
  const [formData, setFormData] = useState({
    id: 70,
    priority: 35,
    action: 'ALLOW',
    name: '',
    protocol: 'TCP',
    dstPort: 443,
    srcCidr: '192.168.1.0/24',
    direction: 'INBOUND',
  });

  const fetchRules = async () => {
    try {
      setLoading(true);
      setError(null);
      const res = await api.getRules();
      setRules(res.rules || []);
      setDefaultPolicy(res.defaultPolicy || 'BLOCKED');
    } catch (err: any) {
      setError(err.message || 'Failed to fetch rules from C++ engine');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchRules();
  }, []);

  const handleToggle = async (id: number, currentEnabled: boolean) => {
    try {
      await api.toggleRule(id, !currentEnabled);
      setRules(prev =>
        prev.map(r => (r.id === id ? { ...r, enabled: !currentEnabled } : r))
      );
      if (onRefreshParent) onRefreshParent();
    } catch (err: any) {
      alert(`Failed to toggle rule: ${err.message}`);
    }
  };

  const handleDelete = async (id: number) => {
    if (!window.confirm(`Are you sure you want to delete Rule #${id}?`)) return;
    try {
      await api.deleteRule(id);
      setRules(prev => prev.filter(r => r.id !== id));
      if (onRefreshParent) onRefreshParent();
    } catch (err: any) {
      alert(`Failed to delete rule: ${err.message}`);
    }
  };

  const handlePriorityChange = async (id: number, newPrio: number) => {
    try {
      await api.updateRulePriority(id, newPrio);
      await fetchRules();
      if (onRefreshParent) onRefreshParent();
    } catch (err: any) {
      alert(`Failed to update priority: ${err.message}`);
    }
  };

  const openAddModal = () => {
    const maxId = rules.reduce((max, r) => Math.max(max, r.id), 0);
    setModalMode('add');
    setFormData({
      id: maxId + 10,
      priority: 35,
      action: 'ALLOW',
      name: '',
      protocol: 'TCP',
      dstPort: 443,
      srcCidr: '192.168.1.0/24',
      direction: 'INBOUND',
    });
    setIsModalOpen(true);
  };

  const openEditModal = (rule: FirewallRuleItem) => {
    const parsed = parseRuleConditions(rule.conditions);
    setModalMode('edit');
    setFormData({
      id: rule.id,
      priority: rule.priority,
      action: rule.action,
      name: rule.name,
      protocol: parsed.proto,
      dstPort: parsed.port,
      srcCidr: parsed.cidr,
      direction: parsed.dir,
    });
    setIsModalOpen(true);
  };

  const openDuplicateModal = (rule: FirewallRuleItem) => {
    const parsed = parseRuleConditions(rule.conditions);
    const maxId = rules.reduce((max, r) => Math.max(max, r.id), 0);
    setModalMode('duplicate');
    setFormData({
      id: maxId + 10,
      priority: rule.priority + 1,
      action: rule.action,
      name: `${rule.name} (Copy)`,
      protocol: parsed.proto,
      dstPort: parsed.port,
      srcCidr: parsed.cidr,
      direction: parsed.dir,
    });
    setIsModalOpen(true);
  };

  const handleFormSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!formData.name.trim()) {
      alert('Please provide a rule description/name');
      return;
    }

    try {
      setSubmitting(true);
      if (modalMode === 'edit') {
        await api.editRule(formData.id, {
          priority: formData.priority,
          action: formData.action,
          name: formData.name,
          protocol: formData.protocol,
          dstPort: formData.dstPort,
          srcCidr: formData.srcCidr,
          direction: formData.direction,
        });
      } else {
        await api.addRule(formData);
      }
      setIsModalOpen(false);
      await fetchRules();
      if (onRefreshParent) onRefreshParent();
    } catch (err: any) {
      alert(`Operation failed: ${err.message}`);
    } finally {
      setSubmitting(false);
    }
  };

  const filteredRules = rules.filter(r => {
    const matchesSearch = 
      r.name.toLowerCase().includes(search.toLowerCase()) ||
      r.conditions.toLowerCase().includes(search.toLowerCase()) ||
      String(r.id).includes(search) ||
      r.action.toLowerCase().includes(search.toLowerCase());

    const matchesAction = 
      actionFilter === 'ALL' || r.action.toUpperCase() === actionFilter.toUpperCase();

    return matchesSearch && matchesAction;
  });

  const getActionBadge = (action: string) => {
    switch (action.toUpperCase()) {
      case 'ALLOW':
        return 'bg-allow-bg text-allow-text border border-allow/20';
      case 'BLOCK':
        return 'bg-block-bg text-block-text border border-block/20';
      case 'REJECT':
        return 'bg-reject-bg text-reject-text border border-reject/20';
      case 'LOG':
        return 'bg-blue-50 text-primary border border-primary/20';
      default:
        return 'bg-slate-100 text-slate-700';
    }
  };

  return (
    <div className="space-y-6">
      {/* Action Bar */}
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between gap-4 bg-card p-4 rounded-card border border-border-subtle shadow-card">
        <div className="flex flex-1 items-center gap-3 max-w-xl">
          <div className="relative flex-1">
            <Search className="w-4 h-4 absolute left-3.5 top-1/2 -translate-y-1/2 text-text-muted" />
            <input
              type="text"
              placeholder="Search rules by name, ID, or condition..."
              value={search}
              onChange={e => setSearch(e.target.value)}
              className="w-full pl-9 pr-4 py-2 bg-slate-50 border border-border-subtle rounded-lg text-xs text-text-primary placeholder:text-text-muted focus:outline-none focus:ring-1 focus:ring-primary focus:border-primary transition-all"
            />
          </div>

          <select
            value={actionFilter}
            onChange={e => setActionFilter(e.target.value)}
            className="px-3 py-2 bg-slate-50 border border-border-subtle rounded-lg text-xs font-semibold text-text-primary focus:ring-1 focus:ring-primary"
          >
            <option value="ALL">All Actions</option>
            <option value="ALLOW">ALLOW</option>
            <option value="BLOCK">BLOCK</option>
            <option value="REJECT">REJECT</option>
            <option value="LOG">LOG</option>
          </select>
        </div>

        <div className="flex items-center space-x-3">
          <button
            onClick={fetchRules}
            disabled={loading}
            className="p-2 border border-border-subtle hover:bg-slate-50 rounded-lg text-text-secondary transition-all"
            title="Refresh Rules from Engine"
          >
            <RotateCw className={`w-3.5 h-3.5 ${loading ? 'animate-spin' : ''}`} />
          </button>

          <button
            onClick={openAddModal}
            className="inline-flex items-center space-x-2 px-3.5 py-2 rounded-lg bg-primary hover:bg-primary-hover text-white text-xs font-semibold shadow-sm transition-all"
          >
            <Plus className="w-4 h-4" />
            <span>Add Security Rule</span>
          </button>
        </div>
      </div>

      {/* Rules Table Card */}
      <div className="bg-card border border-border-subtle rounded-card shadow-card overflow-hidden">
        <div className="px-6 py-4 border-b border-border-subtle flex items-center justify-between">
          <div className="flex items-center space-x-2">
            <ShieldCheck className="w-4 h-4 text-primary" />
            <h3 className="text-sm font-bold text-text-primary">
              Active Security Policies ({rules.length})
            </h3>
          </div>
          <span className="text-xs text-text-secondary">
            System Fallback: <strong className="text-text-primary">{defaultPolicy}</strong>
          </span>
        </div>

        {error && (
          <div className="p-4 bg-rose-50 border-b border-rose-100 text-xs text-rose-700 flex items-center space-x-2">
            <AlertCircle className="w-4 h-4 flex-shrink-0" />
            <span>{error}</span>
          </div>
        )}

        <div className="overflow-x-auto">
          <table className="w-full text-left border-collapse text-xs">
            <thead>
              <tr className="border-b border-border-subtle bg-slate-50/70 text-[11px] font-semibold text-text-secondary uppercase tracking-wider">
                <th className="py-3 px-6">ID</th>
                <th className="py-3 px-6">Priority</th>
                <th className="py-3 px-6">Status</th>
                <th className="py-3 px-6">Action</th>
                <th className="py-3 px-6">Rule Name</th>
                <th className="py-3 px-6">Matching Conditions</th>
                <th className="py-3 px-6 text-right">Actions</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-border-subtle">
              {loading && rules.length === 0 ? (
                <tr>
                  <td colSpan={7} className="py-8 text-center text-text-secondary">
                    Loading rules from engine...
                  </td>
                </tr>
              ) : filteredRules.length > 0 ? (
                filteredRules.map(rule => (
                  <tr
                    key={rule.id}
                    className={`hover:bg-slate-50/60 transition-colors ${
                      !rule.enabled ? 'opacity-60 bg-slate-50/40' : ''
                    }`}
                  >
                    <td className="py-3 px-6 font-mono font-semibold text-text-primary">
                      #{rule.id}
                    </td>
                    <td className="py-3 px-6">
                      <select
                        value={rule.priority}
                        onChange={e => handlePriorityChange(rule.id, Number(e.target.value))}
                        className="bg-slate-50 border border-border-subtle rounded px-2 py-1 text-xs font-mono font-medium focus:ring-1 focus:ring-primary"
                        title="Change Rule Priority (Lower executes first)"
                      >
                        {[5, 10, 15, 20, 25, 30, 35, 40, 50, 60, 70, 80, 90, 100].map(p => (
                          <option key={p} value={p}>
                            {p}
                          </option>
                        ))}
                      </select>
                    </td>
                    <td className="py-3 px-6">
                      <button
                        onClick={() => handleToggle(rule.id, rule.enabled)}
                        className={`inline-flex items-center space-x-1.5 px-2.5 py-1 rounded-full text-[11px] font-semibold transition-colors ${
                          rule.enabled
                            ? 'bg-emerald-50 text-emerald-700 hover:bg-emerald-100'
                            : 'bg-slate-100 text-slate-500 hover:bg-slate-200'
                        }`}
                        title={rule.enabled ? 'Click to Disable' : 'Click to Enable'}
                      >
                        {rule.enabled ? (
                          <>
                            <Check className="w-3 h-3" />
                            <span>Enabled</span>
                          </>
                        ) : (
                          <>
                            <X className="w-3 h-3" />
                            <span>Disabled</span>
                          </>
                        )}
                      </button>
                    </td>
                    <td className="py-3 px-6">
                      <span
                        className={`inline-block px-2.5 py-0.5 rounded-full font-bold text-[10.5px] ${getActionBadge(
                          rule.action
                        )}`}
                      >
                        {rule.action}
                      </span>
                    </td>
                    <td className="py-3 px-6 font-medium text-text-primary">
                      {rule.name}
                    </td>
                    <td className="py-3 px-6 text-text-secondary font-mono text-[11px]">
                      {rule.conditions}
                    </td>
                    <td className="py-3 px-6 text-right whitespace-nowrap">
                      <div className="flex items-center justify-end space-x-1.5">
                        <button
                          onClick={() => openEditModal(rule)}
                          className="text-text-muted hover:text-primary p-1.5 rounded transition-colors"
                          title="Edit Rule Conditions"
                        >
                          <Pencil className="w-3.5 h-3.5" />
                        </button>

                        <button
                          onClick={() => openDuplicateModal(rule)}
                          className="text-text-muted hover:text-primary p-1.5 rounded transition-colors"
                          title="Duplicate Rule"
                        >
                          <Copy className="w-3.5 h-3.5" />
                        </button>

                        <button
                          onClick={() => handleDelete(rule.id)}
                          className="text-text-muted hover:text-block p-1.5 rounded transition-colors"
                          title="Delete Rule"
                        >
                          <Trash2 className="w-3.5 h-3.5" />
                        </button>
                      </div>
                    </td>
                  </tr>
                ))
              ) : (
                <tr>
                  <td colSpan={7} className="py-8 text-center text-text-secondary">
                    No matching rules found.
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* Add / Edit / Duplicate Modal */}
      {isModalOpen && (
        <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/40 backdrop-blur-sm p-4">
          <div className="bg-card rounded-card border border-border-subtle shadow-xl w-full max-w-lg p-6 animate-in fade-in zoom-in-95 duration-150">
            <div className="flex items-center justify-between pb-4 border-b border-border-subtle mb-4">
              <h3 className="text-base font-bold text-text-primary">
                {modalMode === 'edit'
                  ? `Edit Rule #${formData.id}`
                  : modalMode === 'duplicate'
                  ? `Duplicate Rule #${formData.id}`
                  : 'Add Security Policy Rule'}
              </h3>
              <button
                onClick={() => setIsModalOpen(false)}
                className="text-text-muted hover:text-text-primary"
              >
                <X className="w-4 h-4" />
              </button>
            </div>

            <form onSubmit={handleFormSubmit} className="space-y-4 text-xs">
              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block font-medium text-text-secondary mb-1">
                    Rule ID {modalMode === 'edit' ? '(Read-only)' : '(Unique Integer)'}
                  </label>
                  <input
                    type="number"
                    value={formData.id}
                    disabled={modalMode === 'edit'}
                    onChange={e => setFormData({ ...formData, id: Number(e.target.value) })}
                    className="w-full px-3 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-mono focus:ring-1 focus:ring-primary disabled:opacity-60"
                    required
                  />
                </div>
                <div>
                  <label className="block font-medium text-text-secondary mb-1">
                    Priority (Lower executes first)
                  </label>
                  <input
                    type="number"
                    value={formData.priority}
                    onChange={e => setFormData({ ...formData, priority: Number(e.target.value) })}
                    className="w-full px-3 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-mono focus:ring-1 focus:ring-primary"
                    required
                  />
                </div>
              </div>

              <div>
                <label className="block font-medium text-text-secondary mb-1">
                  Rule Name / Description
                </label>
                <input
                  type="text"
                  placeholder="e.g. Allow Inbound SSH from Management"
                  value={formData.name}
                  onChange={e => setFormData({ ...formData, name: e.target.value })}
                  className="w-full px-3 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-text-primary focus:ring-1 focus:ring-primary"
                  required
                />
              </div>

              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block font-medium text-text-secondary mb-1">
                    Action Type
                  </label>
                  <select
                    value={formData.action}
                    onChange={e => setFormData({ ...formData, action: e.target.value })}
                    className="w-full px-3 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-semibold focus:ring-1 focus:ring-primary"
                  >
                    <option value="ALLOW">ALLOW</option>
                    <option value="BLOCK">BLOCK</option>
                    <option value="REJECT">REJECT</option>
                    <option value="LOG">LOG (Non-Terminal)</option>
                  </select>
                </div>
                <div>
                  <label className="block font-medium text-text-secondary mb-1">
                    Protocol
                  </label>
                  <select
                    value={formData.protocol}
                    onChange={e => setFormData({ ...formData, protocol: e.target.value })}
                    className="w-full px-3 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-text-primary focus:ring-1 focus:ring-primary"
                  >
                    <option value="TCP">TCP</option>
                    <option value="UDP">UDP</option>
                    <option value="ICMP">ICMP</option>
                    <option value="ANY">ANY</option>
                  </select>
                </div>
              </div>

              <div className="grid grid-cols-2 gap-3">
                <div>
                  <label className="block font-medium text-text-secondary mb-1">
                    Destination Port (0 for any)
                  </label>
                  <input
                    type="number"
                    value={formData.dstPort}
                    onChange={e => setFormData({ ...formData, dstPort: Number(e.target.value) })}
                    className="w-full px-3 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-mono focus:ring-1 focus:ring-primary"
                  />
                </div>
                <div>
                  <label className="block font-medium text-text-secondary mb-1">
                    Direction
                  </label>
                  <select
                    value={formData.direction}
                    onChange={e => setFormData({ ...formData, direction: e.target.value })}
                    className="w-full px-3 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-text-primary focus:ring-1 focus:ring-primary"
                  >
                    <option value="INBOUND">INBOUND</option>
                    <option value="OUTBOUND">OUTBOUND</option>
                    <option value="ANY">ANY</option>
                  </select>
                </div>
              </div>

              <div>
                <label className="block font-medium text-text-secondary mb-1">
                  Source IP / Subnet (CIDR or ANY)
                </label>
                <input
                  type="text"
                  placeholder="e.g. 192.168.1.0/24 or ANY"
                  value={formData.srcCidr}
                  onChange={e => setFormData({ ...formData, srcCidr: e.target.value })}
                  className="w-full px-3 py-1.5 bg-slate-50 border border-border-subtle rounded-lg text-text-primary font-mono focus:ring-1 focus:ring-primary"
                />
              </div>

              <div className="flex items-center justify-end space-x-3 pt-3 border-t border-border-subtle mt-4">
                <button
                  type="button"
                  onClick={() => setIsModalOpen(false)}
                  className="px-4 py-2 border border-border-subtle rounded-lg hover:bg-slate-50 text-text-secondary font-medium transition-all"
                >
                  Cancel
                </button>
                <button
                  type="submit"
                  disabled={submitting}
                  className="px-4 py-2 bg-primary hover:bg-primary-hover text-white rounded-lg font-semibold shadow-sm transition-all disabled:opacity-50"
                >
                  {submitting
                    ? 'Saving...'
                    : modalMode === 'edit'
                    ? 'Update Rule'
                    : 'Confirm & Save Rule'}
                </button>
              </div>
            </form>
          </div>
        </div>
      )}
    </div>
  );
};
