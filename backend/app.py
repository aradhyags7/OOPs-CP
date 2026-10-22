#!/usr/bin/env python3
"""
Firewall Control Center - Local FastAPI Engine Adapter
======================================================
Serves as the high-performance local REST API bridge between the
React TypeScript frontend and the Modern C++17 Firewall Rule Engine.
"""

import os
import sys
import csv
import json
import threading
import subprocess
from pathlib import Path
from typing import List, Optional, Dict, Any

from fastapi import FastAPI, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field

# Root repository directory
BASE_DIR = Path(__file__).resolve().parent.parent
EXECUTABLE = BASE_DIR / "firewall.exe"
DECISIONS_CSV = BASE_DIR / "firewall_decisions.csv"
AUDIT_LOG = BASE_DIR / "firewall_audit.log"
RULES_CONF = BASE_DIR / "rules" / "sample_rules.conf"
STATS_CONF = BASE_DIR / "state" / "engine_stats.conf"

app = FastAPI(
    title="Firewall Rule Engine Control Center API",
    version="1.0.0",
    description="Local REST API adapter for the Modern C++17 Firewall Rule Engine"
)

# Mutex to prevent concurrent engine subprocess mutations from racing
engine_lock = threading.Lock()

# Enable CORS for local Vite development server
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


def run_engine_command(args: List[str]) -> Dict[str, Any]:
    """Execute firewall.exe with JSON CLI arguments and parse output under thread lock."""
    if not EXECUTABLE.exists():
        raise HTTPException(status_code=500, detail="firewall.exe binary not found. Build it with build.bat.")

    cmd = [str(EXECUTABLE)] + args
    with engine_lock:
        try:
            proc = subprocess.run(
                cmd,
                cwd=str(BASE_DIR),
                capture_output=True,
                text=True,
                encoding="utf-8",
                timeout=20
            )
            if proc.returncode != 0 and not proc.stdout.strip():
                raise HTTPException(status_code=500, detail=f"Engine returned error: {proc.stderr}")

            # Find JSON payload in stdout
            stdout = proc.stdout.strip()
            first_brace = stdout.find("{")
            last_brace = stdout.rfind("}")
            if first_brace != -1 and last_brace != -1 and last_brace > first_brace:
                json_str = stdout[first_brace : last_brace + 1]
                return json.loads(json_str)

            raise HTTPException(status_code=500, detail=f"Invalid JSON from engine: {stdout[:200]}")
        except subprocess.TimeoutExpired:
            raise HTTPException(status_code=504, detail="Engine execution timed out.")
        except json.JSONDecodeError as e:
            raise HTTPException(status_code=500, detail=f"Engine JSON decode error: {e}")


def read_decisions_csv(limit: int = 100) -> List[Dict[str, Any]]:
    """Read decision records from firewall_decisions.csv."""
    if not DECISIONS_CSV.exists():
        return []

    records = []
    try:
        with open(DECISIONS_CSV, "r", encoding="utf-8") as f:
            reader = csv.DictReader(f)
            for row in reader:
                # Map ALLOWED -> ALLOW, BLOCKED -> BLOCK, REJECTED -> REJECT
                dec = row.get("Decision", "BLOCKED")
                badge_dec = "ALLOW" if "ALLOW" in dec else ("REJECT" if "REJECT" in dec else "BLOCK")
                rule_id = row.get("RuleID", "-1")
                rule_label = f"#{rule_id}" if rule_id != "-1" else "Default Policy"

                records.append({
                    "timestamp": row.get("Timestamp", ""),
                    "time": row.get("Timestamp", "").split(" ")[-1] if " " in row.get("Timestamp", "") else row.get("Timestamp", ""),
                    "packetId": row.get("PacketID", "0"),
                    "protocol": row.get("Protocol", "TCP"),
                    "direction": row.get("Direction", "INBOUND"),
                    "src": f"{row.get('SrcIP', '')}:{row.get('SrcPort', '')}" if row.get("SrcPort", "0") != "0" else row.get("SrcIP", ""),
                    "srcIP": row.get("SrcIP", ""),
                    "srcPort": int(row.get("SrcPort", 0) or 0),
                    "dst": f"{row.get('DstIP', '')}:{row.get('DstPort', '')}" if row.get("DstPort", "0") != "0" else row.get("DstIP", ""),
                    "dstIP": row.get("DstIP", ""),
                    "dstPort": int(row.get("DstPort", 0) or 0),
                    "payloadBytes": int(row.get("PayloadBytes", 0) or 0),
                    "decision": badge_dec,
                    "rule": rule_label,
                    "ruleId": int(rule_id),
                    "reason": row.get("Reason", "")
                })
        # Return newest first
        records.reverse()
        return records[:limit]
    except Exception as e:
        print(f"Error reading CSV: {e}", file=sys.stderr)
        return []


# ==============================================================================
# PYDANTIC DATA MODELS
# ==============================================================================
class PacketInspectRequest(BaseModel):
    srcIP: str = Field(..., example="192.168.1.50")
    dstIP: str = Field(..., example="10.0.0.1")
    srcPort: int = Field(51234, ge=0, le=65535)
    dstPort: int = Field(443, ge=0, le=65535)
    protocol: str = Field("TCP", example="TCP")
    direction: str = Field("INBOUND", example="INBOUND")


class AddRuleRequest(BaseModel):
    id: int = Field(..., example=70)
    priority: int = Field(..., example=35)
    action: str = Field("ALLOW", example="ALLOW")
    name: str = Field(..., example="Allow SSH from Bastion")
    protocol: str = Field("TCP", example="TCP")
    dstPort: int = Field(22, ge=0, le=65535)
    srcCidr: str = Field("10.0.50.0/24", example="10.0.50.0/24")
    direction: str = Field("INBOUND", example="INBOUND")


class EditRuleRequest(BaseModel):
    priority: int = Field(..., example=35)
    action: str = Field("ALLOW", example="ALLOW")
    name: str = Field(..., example="Allow SSH from Bastion")
    protocol: str = Field("TCP", example="TCP")
    dstPort: int = Field(22, ge=0, le=65535)
    srcCidr: str = Field("10.0.50.0/24", example="10.0.50.0/24")
    direction: str = Field("INBOUND", example="INBOUND")


class PriorityUpdateRequest(BaseModel):
    priority: int = Field(..., ge=1, le=65535)


class ToggleRuleRequest(BaseModel):
    enabled: bool


# ==============================================================================
# API ROUTE HANDLERS
# ==============================================================================

@app.get("/api/health")
def get_health():
    """Health check endpoint indicating C++ engine status, version, and persistence state."""
    exists = EXECUTABLE.exists()
    return {
        "status": "running" if exists else "stopped",
        "engineConnected": exists,
        "version": "v1.0.0",
        "binary": str(EXECUTABLE),
        "rulesConfig": str(RULES_CONF),
        "stateFile": str(STATS_CONF)
    }


@app.get("/api/dashboard")
def get_dashboard():
    """Fetch live authoritative metrics from C++ engine and CSV records."""
    # 1. Fetch live rules from engine
    rules_data = run_engine_command(["--json-rules"])
    rules_list = rules_data.get("rules", [])

    # 2. Fetch authoritative engine statistics (persisted across runs)
    stats_data = run_engine_command(["--json-stats"])

    total = stats_data.get("packetsProcessed", 0)
    allowed = stats_data.get("packetsAllowed", 0)
    blocked = stats_data.get("packetsBlocked", 0)
    rejected = stats_data.get("packetsRejected", 0)

    if total > 0:
        allowed_pct = round((allowed / total) * 100.0, 1)
        blocked_pct = round((blocked / total) * 100.0, 1)
        rejected_pct = round((rejected / total) * 100.0, 1)
        change_indicator = f"↑ {total:,} packets evaluated"
    else:
        allowed_pct = 0.0
        blocked_pct = 0.0
        rejected_pct = 0.0
        change_indicator = "Ready (0 Packets)"

    # Rule hit tallies from engine
    engine_hits = stats_data.get("ruleHitCounts", {})

    top_rules = []
    # Build list of rules with hits
    for r in rules_list:
        raw_id = r["id"]
        hits = engine_hits.get(str(raw_id), 0)
        hit_label = f"{hits / 1000.0:.1f}K" if hits >= 1000 else str(hits)
        top_rules.append({
            "ruleId": f"#{raw_id}",
            "rawId": raw_id,
            "name": r["name"],
            "hits": hits,
            "label": hit_label
        })

    # Sort descending by hits, then by priority ascending
    top_rules.sort(key=lambda x: (x["hits"], -x["rawId"]), reverse=True)
    top_5_rules = top_rules[:5]

    # 3. Read recent decisions from CSV (top 6 as shown in screenshot)
    recent = read_decisions_csv(limit=6)

    return {
        "summary": {
            "totalPackets": total,
            "allowed": allowed,
            "allowedPercent": allowed_pct,
            "blocked": blocked,
            "blockedPercent": blocked_pct,
            "rejected": rejected,
            "rejectedPercent": rejected_pct,
            "changeIndicator": change_indicator
        },
        "topRules": top_5_rules,
        "recentDecisions": recent,
        "engineStatus": "Running" if EXECUTABLE.exists() else "Stopped"
    }


@app.get("/api/rules")
def get_rules():
    """Fetch active firewall rules from the engine."""
    return run_engine_command(["--json-rules"])


@app.post("/api/rules")
def add_rule(rule: AddRuleRequest):
    """Add a new security rule to the C++ engine."""
    args = [
        "--json-add-rule",
        str(rule.id),
        str(rule.priority),
        rule.action.upper(),
        rule.name,
        rule.protocol.upper(),
        str(rule.dstPort),
        rule.srcCidr,
        rule.direction.upper()
    ]
    return run_engine_command(args)


@app.put("/api/rules/{rule_id}")
def edit_rule(rule_id: int, req: EditRuleRequest):
    """Update conditions and policy attributes of an existing rule."""
    args = [
        "--json-edit-rule",
        str(rule_id),
        str(req.priority),
        req.action.upper(),
        req.name,
        req.protocol.upper(),
        str(req.dstPort),
        req.srcCidr,
        req.direction.upper()
    ]
    return run_engine_command(args)


@app.patch("/api/rules/{rule_id}/priority")
def set_priority(rule_id: int, req: PriorityUpdateRequest):
    """Update priority for a rule (auto re-sorts)."""
    return run_engine_command(["--json-set-priority", str(rule_id), str(req.priority)])


@app.patch("/api/rules/{rule_id}/toggle")
def toggle_rule(rule_id: int, req: ToggleRuleRequest):
    """Enable or disable an existing rule."""
    val = "1" if req.enabled else "0"
    return run_engine_command(["--json-toggle-rule", str(rule_id), val])


@app.delete("/api/rules/{rule_id}")
def delete_rule(rule_id: int):
    """Delete a rule from the engine."""
    return run_engine_command(["--json-delete-rule", str(rule_id)])


@app.post("/api/packets/inspect")
def inspect_packet(pkt: PacketInspectRequest):
    """Evaluate a single packet against live rules using the C++ engine."""
    args = [
        "--json-inspect",
        pkt.srcIP,
        pkt.dstIP,
        str(pkt.srcPort),
        str(pkt.dstPort),
        pkt.protocol.upper(),
        pkt.direction.upper()
    ]
    return run_engine_command(args)


@app.post("/api/stats/reset")
def reset_stats():
    """Reset engine persistent telemetry and clear CSV logs."""
    res = run_engine_command(["--json-reset-stats"])
    if DECISIONS_CSV.exists():
        with open(DECISIONS_CSV, "w", encoding="utf-8") as f:
            f.write("Timestamp,PacketID,Protocol,Direction,SrcIP,SrcPort,DstIP,DstPort,PayloadBytes,Decision,RuleID,Reason\n")
    return res


@app.get("/api/audit")
def get_audit(
    limit: int = Query(100, ge=1, le=5000),
    decision: Optional[str] = Query(None),
    protocol: Optional[str] = Query(None),
    search: Optional[str] = Query(None)
):
    """Fetch structured audit records with optional filtering."""
    records = read_decisions_csv(limit=5000)

    if decision and decision.upper() != "ALL":
        records = [r for r in records if r["decision"].upper() == decision.upper()]

    if protocol and protocol.upper() != "ALL":
        records = [r for r in records if r["protocol"].upper() == protocol.upper()]

    if search:
        s = search.lower()
        records = [
            r for r in records
            if s in r["src"].lower() or s in r["dst"].lower() or s in r["rule"].lower() or s in r["reason"].lower()
        ]

    return {
        "totalRecords": len(records),
        "records": records[:limit]
    }


@app.post("/api/simulations/{scenario_id}/run")
def run_simulation(scenario_id: str, count: int = Query(50000, ge=10, le=500000)):
    """Run an automated demonstration scenario in the C++ engine."""
    sc_code = scenario_id.upper()
    if sc_code not in ["A", "B", "C", "D", "E", "F", "G"]:
        raise HTTPException(status_code=400, detail="Invalid scenario ID. Valid options: A, B, C, D, E, F, G.")

    return run_engine_command(["--json-scenario", sc_code, str(count)])


if __name__ == "__main__":
    import uvicorn
    print(f"Starting Firewall Control Center API on http://127.0.0.1:8000 ...")
    uvicorn.run("app:app", host="127.0.0.1", port=8000, reload=True)
