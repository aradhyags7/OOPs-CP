# Firewall Rule Engine & Control Center (C++17 + React + TypeScript)

A second-year Computer Engineering Object-Oriented Programming (OOP) project demonstrating a high-performance **Firewall Rule Engine implemented in Modern C++17** paired with a professional browser-based **Firewall Control Center** built with **React, TypeScript, and Tailwind CSS**.

---

## 1. System Architecture

```
+-----------------------------------------------------------------------+
|            Frontend: React 18 + TypeScript + Tailwind CSS             |
|         Vite Dev Server (http://127.0.0.1:5173)                       |
|         - Recharts Analytics, Lucide React, Restrained Light Palette  |
|         - Overview, Rules, Packet Inspector, Audit Logs, Simulations  |
+-----------------------------------------------------------------------+
                                  │ HTTP REST API (JSON)
                                  ▼
+-----------------------------------------------------------------------+
|             Backend: Local FastAPI Adapter (backend/app.py)           |
|         Uvicorn Loopback Server (http://127.0.0.1:8000)               |
|         - Thread-safe mutex lock preventing engine subprocess races   |
|         - Reads authoritative C++ persistent state & sample_rules.conf|
|         - Streams decision records from firewall_decisions.csv        |
+-----------------------------------------------------------------------+
                                  │ Native Process CLI Bridge
                                  ▼
+-----------------------------------------------------------------------+
|             Core Engine: Modern C++17 Firewall Rule Engine            |
|         firewall.exe (--json-inspect, --json-rules, --json-stats)     |
|         - RuleManager: Priority-ordered Access Control Lists          |
|         - FirewallEngine: First Terminal Verdict Wins, Default Deny   |
|         - State Persistence: state/engine_stats.conf & conf files     |
|         - Polymorphic RuleCondition: IPCondition, PortCondition, etc. |
+-----------------------------------------------------------------------+
```

---

## 2. State Persistence Model

The C++ engine is the **single source of truth** across the entire stack:
1. **Firewall Policies (`rules/sample_rules.conf`)**:
   - Persists all security rules (ID, priority, enabled status, action, name, and matching conditions).
   - Any rule addition, condition edit, priority re-sorting, toggle, or deletion immediately updates this configuration via `ConfigurationManager::saveConfiguration()`.
2. **Cumulative Statistics (`state/engine_stats.conf`)**:
   - Authoritative cumulative packet counters (total, allowed, blocked, rejected, default hits, logging events, bytes) and per-rule hit tallies.
   - Automatically loaded and updated across individual packet evaluations and batch simulation runs.
3. **Audit Decision Records (`firewall_decisions.csv`)**:
   - Real-time audit trail of every terminal decision reached by `FirewallEngine::processPacket()`.
   - Read by the dashboard for the 6 newest decisions table and by the Audit Logs page with full multi-field filtering.

---

## 3. Quick Start

### Option A: One-Click Launch (Recommended)
From PowerShell:
```powershell
.\run_control_center.bat
```
From Command Prompt:
```cmd
run_control_center.bat
```
This script will:
1. Compile the C++ engine (`firewall.exe`) and unit test suite (`test_engine.exe`) if not present.
2. Start the local Python FastAPI adapter on `http://127.0.0.1:8000`.
3. Start the Vite React development server on `http://127.0.0.1:5173`.
4. Automatically open `http://127.0.0.1:5173/` in your default web browser.

### Option B: Manual Startup

#### Step 1: Compile the C++ Engine & Tests
```cmd
.\build.bat
```
Run the automated test suite (16/16 unit tests):
```cmd
.\test_engine.exe
```

#### Step 2: Start the Backend API Adapter
```cmd
python -m uvicorn backend.app:app --host 127.0.0.1 --port 8000
```
Interactive Swagger API documentation is available at `http://127.0.0.1:8000/docs`.

#### Step 3: Start the React Frontend Dashboard
```cmd
cd frontend
npm run dev
```
Open [http://127.0.0.1:5173](http://127.0.0.1:5173) in your browser.

---

## 3. Dashboard Features (Matching Reference Screenshot)

1. **Overview Dashboard**:
   - **Summary Cards**: Real-time totals for Total Packets, Allowed (with % and green progress bar), Blocked (with % and red progress bar), and Rejected (with % and amber progress bar).
   - **Packet Decisions Donut Chart**: Rendered with Recharts, displaying distribution of verdicts, total packet count in the center, and interactive tooltips.
   - **Top Rules by Hits Bar Chart**: Vertical bar chart highlighting most frequently matched rules with direct links to rule configuration.
   - **Recent Packet Decisions Table**: Live audit feed displaying time, source/destination IP and port, transport protocol, rule ID, and colored verdict badges (`ALLOW`, `BLOCK`, `REJECT`).

2. **Rules Management**:
   - View all active security policies loaded from `rules/sample_rules.conf`.
   - Modify numerical priorities (automatic re-sorting via `std::stable_sort`).
   - Enable / disable rules with instant state persistence.
   - Add new security policies with CIDR prefix masks, port ranges, protocols, and directions.
   - Delete rules with confirmation.

3. **Simulated Packet Inspector**:
   - Test synthetic packets against the live C++ engine without altering OS networking.
   - 1-click test scenarios for HTTPS Web (443), DNS (53), Telnet (23), Office Subnet, Rogue Subnet, and Ping.
   - Live verdict card displaying exact decision, matched rule, specific reason, and non-terminal log taps.

4. **Audit Logs**:
   - Structured search and filtering across all evaluated packets.
   - Filter by verdict (`ALLOW`, `BLOCK`, `REJECT`) or protocol (`TCP`, `UDP`, `ICMP`).
   - One-click CSV export.

5. **Demonstration Scenarios & Benchmark**:
   - Live execution of Scenarios A through G directly inside the browser.
   - Scenario G: 50,000 packets batch benchmark displaying empirical throughput (~1.98 Mpps) and verdict breakdown.

---

## 4. Academic Documentation & Viva Defense

Complete documentation files are generated and ready for faculty submission:

- **[`Firewall_Rule_Engine_Project_Documentation.md`](file:///c:/Users/ASUS/OneDrive/Desktop/OOP%20CP/Firewall_Rule_Engine_Project_Documentation.md)**: Complete 13-section formal technical documentation.
- **[`OOP_Concepts_and_Architecture.md`](file:///c:/Users/ASUS/OneDrive/Desktop/OOP%20CP/OOP_Concepts_and_Architecture.md)**: Object-Oriented design principles manual with UML diagrams and viva Q&A.
- **[`Research_Gap_and_Methodology.md`](file:///c:/Users/ASUS/OneDrive/Desktop/OOP%20CP/Research_Gap_and_Methodology.md)**: Tripartite gap model and 11-phase development lifecycle.
- **[`pdf/`](file:///c:/Users/ASUS/OneDrive/Desktop/OOP%20CP/pdf/)**: Publication-grade compiled PDF documents.

---

## 5. Technology Stack Summary

| Subsystem | Technology | Purpose |
| :--- | :--- | :--- |
| **Core Firewall Engine** | Modern C++17 | Packet inspection, CIDR bitmask arithmetic, rule prioritization, default-deny enforcement |
| **Build System** | MinGW-w64 GCC 16.1.0 / CMake | Fast zero-dependency C++ compilation (`-O2 -Wall -Wextra`) |
| **API Adapter** | Python 3.14 + FastAPI + Uvicorn | Local REST bridge between browser and C++ executable |
| **Frontend Framework** | React 18 + TypeScript | UI state management and strict typing |
| **Frontend Tooling** | Vite | Ultra-fast bundling, HMR, and proxying |
| **Styling** | Tailwind CSS v3 | Restrained light theme matching reference visual design |
| **Visualizations** | Recharts | Donut chart and Top Rules vertical bar chart |
| **Icons** | Lucide React | Professional stroke icons matching reference interface |
