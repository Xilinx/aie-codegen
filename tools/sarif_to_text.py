#!/usr/bin/env python3
###############################################################################
# Copyright (C) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
"""Convert a CodeQL SARIF file to a human-readable text report."""

import argparse
import json
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

SEVERITY_LABELS = {
    "error": "High",
    "warning": "Medium",
    "note": "Low",
    "recommendation": "Info",
}
SEVERITY_ORDER = {sev: i for i, sev in enumerate(SEVERITY_LABELS)}


@dataclass
class Finding:
    rule_id: str
    severity: str
    security_score: str
    cwe_list: str
    message: str
    location: str
    rule_description: str


@dataclass
class Report:
    tool_name: str
    tool_version: str
    findings: list[Finding] = field(default_factory=list)


def _resolve_severity(
    result: dict[str, Any], rule: dict[str, Any]
) -> str:
    """Get severity from result properties, falling back to the rule definition."""
    return (
        result.get("properties", {}).get("problem.severity")
        or rule.get("properties", {}).get("problem.severity", "unknown")
    )


def _format_cwe(rule: dict[str, Any]) -> str:
    tags = rule.get("properties", {}).get("tags", [])
    cwe_tags = [t for t in tags if t.startswith("external/cwe/cwe-")]
    if not cwe_tags:
        return "N/A"
    return ", ".join(t.replace("external/cwe/", "").upper() for t in cwe_tags)


def _format_location(result: dict[str, Any]) -> str:
    locs = result.get("locations", [])
    if not locs:
        return "unknown"
    phys = locs[0].get("physicalLocation", {})
    path = phys.get("artifactLocation", {}).get("uri", "unknown")
    region = phys.get("region", {})
    line = region.get("startLine", "?")
    col = region.get("startColumn")
    return f"{path}:{line}:{col}" if col else f"{path}:{line}"


def parse_sarif(sarif_path: Path) -> Report:
    """Parse a SARIF file into a Report."""
    data = json.loads(sarif_path.read_text())

    run = data.get("runs", [{}])[0]
    driver = run.get("tool", {}).get("driver", {})
    report = Report(
        tool_name=driver.get("name", "CodeQL"),
        tool_version=driver.get("semanticVersion", driver.get("version", "unknown")),
    )

    rules = {r["id"]: r for r in driver.get("rules", [])}

    for result in run.get("results", []):
        rule_id = result.get("ruleId", "unknown")
        rule = rules.get(rule_id, {})
        report.findings.append(
            Finding(
                rule_id=rule_id,
                severity=_resolve_severity(result, rule),
                security_score=rule.get("properties", {}).get(
                    "security-severity", ""
                ),
                cwe_list=_format_cwe(rule),
                message=result.get("message", {}).get("text", ""),
                location=_format_location(result),
                rule_description=rule.get("shortDescription", {}).get("text", ""),
            )
        )

    report.findings.sort(key=lambda f: SEVERITY_ORDER.get(f.severity, 99))
    return report


def format_report(report: Report) -> str:
    """Render a Report as human-readable text."""
    lines: list[str] = []
    lines.append(f"CodeQL Security Report ({report.tool_name} {report.tool_version})")
    lines.append("=" * 70)
    lines.append("")

    counts: dict[str, int] = defaultdict(int)
    for f in report.findings:
        counts[f.severity] += 1

    lines.append(f"Total findings: {len(report.findings)}")
    for sev, label in SEVERITY_LABELS.items():
        if sev in counts:
            lines.append(f"  {label}: {counts[sev]}")
    lines.append("")
    lines.append("-" * 70)

    for i, f in enumerate(report.findings, 1):
        sev_label = SEVERITY_LABELS.get(f.severity, f.severity)
        if f.security_score:
            sev_label += f" (score: {f.security_score})"

        lines.append("")
        lines.append(f"[{i}] {sev_label} | {f.rule_id} | {f.cwe_list}")
        lines.append(f"    Location: {f.location}")
        lines.append(f"    {f.message}")
        if f.rule_description:
            lines.append(f"    Rule: {f.rule_description}")

    lines.append("")
    lines.append("-" * 70)
    lines.append(f"End of report ({len(report.findings)} finding(s))")
    lines.append("")
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("sarif", type=Path, help="Input SARIF file")
    parser.add_argument("output", type=Path, help="Output text report path")
    args = parser.parse_args()

    report = parse_sarif(args.sarif)
    text = format_report(report)

    args.output.write_text(text)
    print(text)


if __name__ == "__main__":
    main()
