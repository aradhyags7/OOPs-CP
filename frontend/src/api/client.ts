import type {
  DashboardData,
  HealthResponse,
  RulesResponse,
  PacketInspectRequest,
  PacketInspectResponse,
  AuditResponse,
  SimulationResult
} from '../types';

const API_BASE = '/api';

export class ApiError extends Error {
  status: number;
  constructor(status: number, message: string) {
    super(message);
    this.status = status;
    this.name = 'ApiError';
  }
}

async function request<T>(endpoint: string, options?: RequestInit): Promise<T> {
  try {
    const res = await fetch(`${API_BASE}${endpoint}`, {
      headers: {
        'Content-Type': 'application/json',
        ...options?.headers,
      },
      ...options,
    });

    if (!res.ok) {
      let errorMsg = `Server error ${res.status}`;
      try {
        const errorData = await res.json();
        errorMsg = errorData.detail || errorData.message || errorMsg;
      } catch {
        // fallback
      }
      throw new ApiError(res.status, errorMsg);
    }

    return await res.json();
  } catch (err) {
    if (err instanceof ApiError) {
      throw err;
    }
    throw new Error(`Connection to Firewall Engine failed: ${(err as Error).message}`);
  }
}

export const api = {
  getHealth: () => request<HealthResponse>('/health'),
  getDashboard: () => request<DashboardData>('/dashboard'),
  getRules: () => request<RulesResponse>('/rules'),
  addRule: (rule: {
    id: number;
    priority: number;
    action: string;
    name: string;
    protocol: string;
    dstPort: number;
    srcCidr: string;
    direction: string;
  }) => request<{ status: string; ruleId: number }>('/rules', {
    method: 'POST',
    body: JSON.stringify(rule),
  }),
  updateRulePriority: (id: number, priority: number) =>
    request<{ status: string; ruleId: number; priority: number }>(`/rules/${id}/priority`, {
      method: 'PATCH',
      body: JSON.stringify({ priority }),
    }),
  toggleRule: (id: number, enabled: boolean) =>
    request<{ status: string; ruleId: number; enabled: boolean }>(`/rules/${id}/toggle`, {
      method: 'PATCH',
      body: JSON.stringify({ enabled }),
    }),
  deleteRule: (id: number) =>
    request<{ status: string; ruleId: number }>(`/rules/${id}`, {
      method: 'DELETE',
    }),
  inspectPacket: (pkt: PacketInspectRequest) =>
    request<PacketInspectResponse>('/packets/inspect', {
      method: 'POST',
      body: JSON.stringify(pkt),
    }),
  getAuditLogs: (params?: { decision?: string; protocol?: string; search?: string; limit?: number }) => {
    const q = new URLSearchParams();
    if (params?.decision) q.set('decision', params.decision);
    if (params?.protocol) q.set('protocol', params.protocol);
    if (params?.search) q.set('search', params.search);
    if (params?.limit) q.set('limit', String(params.limit));
    return request<AuditResponse>(`/audit?${q.toString()}`);
  },
  editRule: (id: number, rule: {
    priority: number;
    action: string;
    name: string;
    protocol: string;
    dstPort: number;
    srcCidr: string;
    direction: string;
  }) => request<{ status: string; ruleId: number }>(`/rules/${id}`, {
    method: 'PUT',
    body: JSON.stringify(rule),
  }),
  resetStats: () => request<{ status: string; message: string }>('/stats/reset', {
    method: 'POST',
  }),
  runSimulation: (scenarioId: string, count: number = 50000) =>
    request<SimulationResult>(`/simulations/${scenarioId}/run?count=${count}`, {
      method: 'POST',
    }),
};
