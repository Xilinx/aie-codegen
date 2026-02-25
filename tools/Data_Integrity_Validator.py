#!/usr/bin/env python3
"""
Universal Data Integrity Validator for Assembly Files

This script provides comprehensive data integrity validation for UC_DMA_BD 
assembly optimization scripts. It ensures absolute data payload size and 
integrity while running all optimization scripts.

Key Features:
- Validates UC_DMA_BD instruction sizes match actual data payload entries
- Cross-validates between multiple data section types (WRITE_data, DMAWRITE_data)
- Provides detailed integrity reports with exact mismatch locations
- Can be used standalone or integrated with other optimization scripts
- Supports both pre and post-optimization validation
- Handles hex and decimal size formats consistently

Usage:
    # Validate a single file
    python Data_Integrity_Validator.py input_file.asm

    # Validate all .asm files in a directory (recursive)
    python Data_Integrity_Validator.py <folder_path>
  
Integration:
  from Data_Integrity_Validator import DataIntegrityValidator
  validator = DataIntegrityValidator()
  is_valid = validator.validate_file("file.asm")
"""

import re
import os
import sys
import argparse
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass
from datetime import datetime

def safe_print(text):
    """Print text with Unicode fallback for Windows console compatibility."""
    try:
        print(text)
    except UnicodeEncodeError:
        # Fallback for Windows console - replace emojis and Unicode chars with text alternatives
        fallback_text = (text.replace('[START]', '[START]')
                        .replace('[OK]', '[OK]')
                        .replace('[ERROR]', '[ERROR]')
                        .replace('[TOOL]', '[TOOL]')
                        .replace('[OK]', '[OK]')
                        .replace('[ERROR]', '[ERROR]')
                        .replace('->', '->')
                        .replace('[FILE]', '[FILE]')
                        .replace('[INFO]', '[INFO]')
                        .replace('[SIZE]', '[SIZE]')
                        .replace('?', '[LIST]')
                        .replace('?', '[HELP]')
                        .replace('[NUMBERS]', '[NUM]')
                        .replace('[NOTE]', '[WRITE]')
                        .replace('[SEARCH]', '[SEARCH]')
                        .replace('[CLEAN]', '[CLEAN]')
                        .replace('[WARN]', '[WARN]')
                        .replace('[DELETE]', '[DELETE]')
                        .replace('[SUCCESS]', '[SUCCESS]')
                        .replace('?', '[CRITICAL]'))
        print(fallback_text)


@dataclass
class UcDmaBdReference:
    """Represents a UC_DMA_BD instruction reference"""
    line_number: int
    reg_off: str
    label: str
    expected_size: int
    last_flag: str
    full_line: str


@dataclass
class DataSection:
    """Represents a data section payload"""
    label: str
    line_start: int
    line_end: int
    data_entries: List[str]
    actual_count: int


@dataclass
class IntegrityResult:
    """Represents an integrity validation result"""
    label: str
    expected_size: int
    actual_size: int
    status: str  # "PASSED", "FAILED", "MISSING"
    line_number: int
    details: str


@dataclass
class FileValidationSummary:
    """Aggregated validation summary for a single file."""
    file_path: str
    passed: int
    failed: int
    missing: int
    total: int
    is_valid: bool
    issues: List[IntegrityResult]


class DataIntegrityValidator:
    """Universal data integrity validator for assembly files"""
    
    def __init__(self, debug_mode: bool = True):
        self.debug_mode = debug_mode
        self.uc_dma_bd_references: Dict[str, List[UcDmaBdReference]] = {}
        self.data_sections: Dict[str, DataSection] = {}
        self.integrity_results: List[IntegrityResult] = []

    def reset_state(self) -> None:
        """Reset all per-file state so this instance can validate multiple files."""
        self.uc_dma_bd_references = {}
        self.data_sections = {}
        self.integrity_results = []
        
    def debug_print(self, message: str) -> None:
        """Print debug information if debug mode is enabled."""
        if self.debug_mode:
            safe_print(f"DEBUG: {message}")
    
    def parse_file(self, filepath: str) -> List[str]:
        """Parse assembly file and return lines with comprehensive validation."""
        # Validate filepath
        if not filepath:
            safe_print(f"[ERROR] ERROR: File path cannot be empty")
            sys.exit(1)
            
        if not os.path.exists(filepath):
            safe_print(f"[ERROR] ERROR: File '{filepath}' not found")
            sys.exit(1)
            
        if not os.path.isfile(filepath):
            safe_print(f"[ERROR] ERROR: '{filepath}' is not a valid file")
            sys.exit(1)
            
        if not os.access(filepath, os.R_OK):
            safe_print(f"[ERROR] ERROR: No read permission for file '{filepath}'")
            sys.exit(1)
            
        if os.path.getsize(filepath) == 0:
            safe_print(f"[ERROR] ERROR: File '{filepath}' is empty")
            sys.exit(1)
            
        try:
            # Try different encodings for robustness
            encodings_to_try = ['utf-8', 'utf-8-sig', 'latin-1', 'cp1252']
            lines = None
            
            for encoding in encodings_to_try:
                try:
                    with open(filepath, 'r', encoding=encoding) as f:
                        lines = f.readlines()
                    self.debug_print(f"Successfully read {len(lines)} lines from {filepath} (encoding: {encoding})")
                    break
                except UnicodeDecodeError:
                    continue
                    
            if lines is None:
                safe_print(f"[ERROR] ERROR: Unable to decode file '{filepath}' with any supported encoding")
                sys.exit(1)
                
            if not lines:
                safe_print(f"[ERROR] ERROR: File '{filepath}' contains no readable lines")
                sys.exit(1)
                
            # Validate line count (reasonable bounds)
            if len(lines) > 10000000:  # 10 million lines limit
                safe_print(f"[WARN] WARNING: File has {len(lines)} lines which may cause performance issues")
                
            return lines
            
        except PermissionError:
            safe_print(f"[ERROR] ERROR: Permission denied reading file '{filepath}'")
            sys.exit(1)
        except OSError as e:
            safe_print(f"[ERROR] ERROR: OS error reading file '{filepath}': {e}")
            sys.exit(1)
        except MemoryError:
            safe_print(f"[ERROR] ERROR: File '{filepath}' too large to load into memory")
            sys.exit(1)
        except Exception as e:
            safe_print(f"[ERROR] ERROR: Failed to read file '{filepath}': {e}")
            sys.exit(1)
    
    def extract_uc_dma_bd_references(self, lines: List[str]) -> None:
        """Extract all UC_DMA_BD instruction references with robust error handling."""
        safe_print("[SEARCH] Extracting UC_DMA_BD instruction references...")
        
        if not lines:
            safe_print("[WARN] WARNING: No lines to parse for UC_DMA_BD references")
            return
        
        # Pattern: UC_DMA_BD 0, RegOff, @label, size, 0, last_flag
        pattern = r'\s*UC_DMA_BD\s+0,\s*(0x[0-9a-fA-F]+),\s*@(\w+),\s*([0-9]+|0x[0-9a-fA-F]+),\s*0,\s*([01])'
        
        for i, line in enumerate(lines):
            if not line:
                continue
                
            try:
                match = re.search(pattern, line.strip())
                if match:
                    reg_off = match.group(1)
                    label = match.group(2)
                    size_str = match.group(3)
                    last_flag = match.group(4)
                    
                    # Validate label format
                    if not re.match(r'^[A-Za-z_][A-Za-z0-9_]*$', label):
                        self.debug_print(f"Invalid label format '{label}' at line {i+1}, skipping")
                        continue
                    
                    # Handle both decimal and hex sizes with validation
                    try:
                        expected_size = int(size_str, 16) if size_str.startswith('0x') else int(size_str)
                        
                        # Validate size range
                        if expected_size < 0:
                            self.debug_print(f"Negative size {expected_size} at line {i+1}, skipping")
                            continue
                        if expected_size > 0xFFFF:  # Reasonable limit
                            self.debug_print(f"Size {expected_size} too large at line {i+1}, skipping")
                            continue
                            
                    except (ValueError, OverflowError) as e:
                        self.debug_print(f"Invalid size format '{size_str}' at line {i+1}: {e}")
                        continue
                    
                    # Validate reg_off format
                    try:
                        reg_off_val = int(reg_off, 16)
                        if reg_off_val < 0 or reg_off_val > 0xFFFFFFFF:
                            self.debug_print(f"RegOff out of range at line {i+1}, skipping")
                            continue
                    except (ValueError, OverflowError) as e:
                        self.debug_print(f"Invalid RegOff format '{reg_off}' at line {i+1}: {e}")
                        continue
                    
                    # Validate last_flag
                    if last_flag not in ['0', '1']:
                        self.debug_print(f"Invalid last flag '{last_flag}' at line {i+1}, skipping")
                        continue
                    
                    reference = UcDmaBdReference(
                        line_number=i + 1,
                        reg_off=reg_off,
                        label=label,
                        expected_size=expected_size,
                        last_flag=last_flag,
                        full_line=line.strip()
                    )
                    
                    if label not in self.uc_dma_bd_references:
                        self.uc_dma_bd_references[label] = []
                    self.uc_dma_bd_references[label].append(reference)
                    
                    self.debug_print(f"Found UC_DMA_BD reference: {label} (size={expected_size}) at line {i+1}")
                    
            except Exception as e:
                self.debug_print(f"Error parsing line {i+1}: {e}")
                continue
        
        total_references = sum(len(refs) for refs in self.uc_dma_bd_references.values())
        safe_print(f"[INFO] Found {total_references} UC_DMA_BD references for {len(self.uc_dma_bd_references)} unique labels")
    
    def extract_data_sections(self, lines: List[str]) -> None:
        """Extract all data section payloads with robust bounds checking."""
        safe_print("[SEARCH] Extracting data section payloads...")
        
        if not lines:
            safe_print("[WARN] WARNING: No lines to parse for data sections")
            return
        
        i = 0
        while i < len(lines):
            try:
                line = lines[i].strip() if i < len(lines) else ""
                
                # Check for data section labels (WRITE_data_X: or DMAWRITE_data_X:)
                label_match = re.match(r'(WRITE_data_\d+|DMAWRITE_data_\d+):', line)
                if label_match:
                    label = label_match.group(1)
                    line_start = i + 1
                    data_entries = []
                    
                    # Collect all .long entries for this data section
                    j = i + 1
                    while j < len(lines):
                        try:
                            data_line = lines[j].strip()
                            
                            # Validate .long entries
                            if data_line.startswith('.long'):
                                # Basic validation of .long format
                                long_pattern = r'\.long\s+(0x[0-9a-fA-F]+|[0-9]+|\w+)'
                                if re.match(long_pattern, data_line):
                                    data_entries.append(data_line)
                                else:
                                    self.debug_print(f"Malformed .long entry at line {j+1}: '{data_line}'")
                                j += 1
                            elif data_line == '' or data_line.startswith('#') or data_line.startswith('//') or data_line.startswith(';'):
                                # Skip empty lines and comments
                                j += 1
                            elif data_line.endswith(':'):
                                # Found next section label
                                break
                            elif data_line.startswith('.'):
                                # Found directive (not .long)
                                break
                            else:
                                # Other content, continue or break based on context
                                j += 1
                                
                        except IndexError:
                            self.debug_print(f"Index error at line {j+1} while parsing data section '{label}'")
                            break
                        except Exception as e:
                            self.debug_print(f"Error parsing line {j+1} in data section '{label}': {e}")
                            j += 1
                            continue
                    
                    # Validate data section
                    if not data_entries:
                        self.debug_print(f"Data section '{label}' has no entries (lines {line_start}-{j})")
                    
                    data_section = DataSection(
                        label=label,
                        line_start=line_start,
                        line_end=j,
                        data_entries=data_entries,
                        actual_count=len(data_entries)
                    )
                    
                    # Check for duplicate labels
                    if label in self.data_sections:
                        self.debug_print(f"WARNING: Duplicate data section label '{label}' found (overwriting previous)")
                    
                    self.data_sections[label] = data_section
                    self.debug_print(f"Found data section: {label} with {len(data_entries)} entries (lines {line_start}-{j})")
                    
                    i = j
                else:
                    i += 1
                    
            except Exception as e:
                self.debug_print(f"Error processing line {i+1}: {e}")
                i += 1
                continue
        
        safe_print(f"[INFO] Found {len(self.data_sections)} data sections")
        
        # Print summary by type
        write_data_count = len([label for label in self.data_sections.keys() if label.startswith('WRITE_data_')])
        dmawrite_data_count = len([label for label in self.data_sections.keys() if label.startswith('DMAWRITE_data_')])
        safe_print(f"   - WRITE_data sections: {write_data_count}")
        safe_print(f"   - DMAWRITE_data sections: {dmawrite_data_count}")
    
    def validate_integrity(self) -> bool:
        """Perform comprehensive data integrity validation."""
        
        # Skip integrity check if no data sections found
        if not self.data_sections:
            safe_print("[INFO] No DMAWRITE_data or WRITE_data sections found - skipping integrity check")
            return True
        
        safe_print("\n[SEARCH] Starting comprehensive data integrity validation...")
        
        all_valid = True
        
        # Check each UC_DMA_BD reference against its data section
        for label, references in self.uc_dma_bd_references.items():
            for ref in references:
                if label in self.data_sections:
                    data_section = self.data_sections[label]
                    
                    if ref.expected_size == data_section.actual_count:
                        result = IntegrityResult(
                            label=label,
                            expected_size=ref.expected_size,
                            actual_size=data_section.actual_count,
                            status="PASSED",
                            line_number=ref.line_number,
                            details=f"Size matches: {ref.expected_size} entries"
                        )
                        self.debug_print(f"[OK] INTEGRITY OK: {label} - size={ref.expected_size} matches data entries={data_section.actual_count}")
                    else:
                        result = IntegrityResult(
                            label=label,
                            expected_size=ref.expected_size,
                            actual_size=data_section.actual_count,
                            status="FAILED",
                            line_number=ref.line_number,
                            details=f"Size mismatch: expected {ref.expected_size}, found {data_section.actual_count}"
                        )
                        safe_print(f"? INTEGRITY ERROR: {label} - UC_DMA_BD size={ref.expected_size}, actual data entries={data_section.actual_count} (line {ref.line_number})")
                        all_valid = False
                else:
                    result = IntegrityResult(
                        label=label,
                        expected_size=ref.expected_size,
                        actual_size=0,
                        status="MISSING",
                        line_number=ref.line_number,
                        details=f"Data section not found"
                    )
                    safe_print(f"[ERROR] MISSING DATA: {label} referenced at line {ref.line_number} but data section not found")
                    all_valid = False
                
                self.integrity_results.append(result)
        
        # Check for orphaned data sections (data sections without UC_DMA_BD references)
        referenced_labels = set(self.uc_dma_bd_references.keys())
        data_labels = set(self.data_sections.keys())
        orphaned_labels = data_labels - referenced_labels
        
        if orphaned_labels:
            safe_print(f"\n[WARN]  WARNING: Found {len(orphaned_labels)} orphaned data sections (no UC_DMA_BD references):")
            for label in sorted(orphaned_labels):
                safe_print(f"   - {label} ({self.data_sections[label].actual_count} entries)")
        
        return all_valid
    
    def generate_report(self, output_file: Optional[str] = None) -> str:
        """Generate detailed integrity validation report."""
        report_lines = []
        report_lines.append("=" * 80)
        report_lines.append("DATA INTEGRITY VALIDATION REPORT")
        report_lines.append("=" * 80)
        report_lines.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report_lines.append("")
        
        # Summary statistics
        passed = len([r for r in self.integrity_results if r.status == "PASSED"])
        failed = len([r for r in self.integrity_results if r.status == "FAILED"])
        missing = len([r for r in self.integrity_results if r.status == "MISSING"])
        total = len(self.integrity_results)
        
        report_lines.append("SUMMARY:")
        report_lines.append(f"  Total validations: {total}")
        report_lines.append(f"  Passed: {passed} ({passed/total*100:.1f}%)" if total > 0 else "  Passed: 0")
        report_lines.append(f"  Failed: {failed} ({failed/total*100:.1f}%)" if total > 0 else "  Failed: 0")
        report_lines.append(f"  Missing: {missing} ({missing/total*100:.1f}%)" if total > 0 else "  Missing: 0")
        report_lines.append("")
        
        # Overall status
        if failed == 0 and missing == 0:
            report_lines.append("OVERALL STATUS: [OK] ALL INTEGRITY CHECKS PASSED")
        else:
            report_lines.append("OVERALL STATUS: [ERROR] INTEGRITY ISSUES DETECTED")
        report_lines.append("")
        
        # Detailed results
        if failed > 0 or missing > 0:
            report_lines.append("DETAILED ISSUES:")
            report_lines.append("-" * 40)
            for result in self.integrity_results:
                if result.status in ["FAILED", "MISSING"]:
                    report_lines.append(f"Line {result.line_number}: {result.label}")
                    report_lines.append(f"  Status: {result.status}")
                    report_lines.append(f"  Details: {result.details}")
                    report_lines.append("")
        
        # Data section summary
        report_lines.append("DATA SECTIONS FOUND:")
        report_lines.append("-" * 40)
        for label in sorted(self.data_sections.keys()):
            section = self.data_sections[label]
            report_lines.append(f"{label}: {section.actual_count} entries (lines {section.line_start}-{section.line_end})")
        
        report_text = "\n".join(report_lines)
        
        if output_file:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(report_text)
            safe_print(f"[FILE] Detailed report written to: {output_file}")
        
        return report_text
    
    def validate_file(self, filepath: str, report_file: Optional[str] = None) -> bool:
        """
        Main validation function - validates a single assembly file.
        
        Args:
            filepath: Path to assembly file to validate
            report_file: Optional path to write detailed report
            
        Returns:
            True if all integrity checks pass, False otherwise
        """
        # Ensure we don't carry state across multiple files.
        self.reset_state()
        safe_print(f"[START] Starting data integrity validation for: {filepath}")
        
        # Parse file
        lines = self.parse_file(filepath)
        
        # Extract UC_DMA_BD references and data sections
        self.extract_uc_dma_bd_references(lines)
        self.extract_data_sections(lines)
        
        # Perform validation
        is_valid = self.validate_integrity()
        
        # Generate report
        report = self.generate_report(report_file)
        
        # Print summary
        passed = len([r for r in self.integrity_results if r.status == "PASSED"])
        failed = len([r for r in self.integrity_results if r.status == "FAILED"])
        missing = len([r for r in self.integrity_results if r.status == "MISSING"])
        
        safe_print(f"\n[INFO] VALIDATION SUMMARY:")
        safe_print(f"   [OK] Passed: {passed}")
        safe_print(f"   [ERROR] Failed: {failed}")
        safe_print(f"   [SEARCH] Missing: {missing}")
        safe_print(f"   [SIZE] Total: {passed + failed + missing}")
        
        if is_valid:
            safe_print("\n[SUCCESS] ALL DATA INTEGRITY CHECKS PASSED!")
        else:
            safe_print("\n? DATA INTEGRITY ISSUES DETECTED!")
        
        return is_valid


def generate_combined_report(base_dir: str, file_summaries: List[FileValidationSummary],
                             output_file: Optional[str] = None) -> str:
    """Generate a combined report for all validated asm files."""
    report_lines: List[str] = []
    report_lines.append("=" * 100)
    report_lines.append("COMBINED DATA INTEGRITY VALIDATION REPORT")
    report_lines.append("=" * 100)
    report_lines.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    report_lines.append(f"Base directory: {base_dir}")
    report_lines.append(f"Files validated: {len(file_summaries)}")
    report_lines.append("")

    total_passed = sum(s.passed for s in file_summaries)
    total_failed = sum(s.failed for s in file_summaries)
    total_missing = sum(s.missing for s in file_summaries)
    total_checks = sum(s.total for s in file_summaries)
    total_valid_files = sum(1 for s in file_summaries if s.is_valid)
    total_invalid_files = len(file_summaries) - total_valid_files

    report_lines.append("OVERALL SUMMARY:")
    report_lines.append(f"  Files passed: {total_valid_files}")
    report_lines.append(f"  Files failed: {total_invalid_files}")
    report_lines.append(f"  Total validations: {total_checks}")
    report_lines.append(f"  Passed: {total_passed}")
    report_lines.append(f"  Failed: {total_failed}")
    report_lines.append(f"  Missing: {total_missing}")
    report_lines.append("")

    if total_failed == 0 and total_missing == 0:
        report_lines.append("OVERALL STATUS: [OK] ALL FILES PASSED DATA INTEGRITY VALIDATION")
    else:
        report_lines.append("OVERALL STATUS: [ERROR] DATA INTEGRITY ISSUES DETECTED")
    report_lines.append("")

    report_lines.append("PER-FILE SUMMARY:")
    report_lines.append("-" * 100)
    for summary in file_summaries:
        rel_path = os.path.relpath(summary.file_path, base_dir)
        status = "PASSED" if summary.is_valid else "FAILED"
        report_lines.append(
            f"{rel_path}: {status} | Total={summary.total}, Passed={summary.passed}, "
            f"Failed={summary.failed}, Missing={summary.missing}"
        )
    report_lines.append("")

    issue_file_summaries = [s for s in file_summaries if s.failed > 0 or s.missing > 0]
    if issue_file_summaries:
        report_lines.append("DETAILED ISSUES BY FILE:")
        report_lines.append("-" * 100)
        for summary in issue_file_summaries:
            rel_path = os.path.relpath(summary.file_path, base_dir)
            report_lines.append(f"File: {rel_path}")
            for issue in summary.issues:
                report_lines.append(f"  - Line {issue.line_number}: {issue.label} [{issue.status}] - {issue.details}")
            report_lines.append("")

    report_text = "\n".join(report_lines)

    if output_file:
        os.makedirs(os.path.dirname(output_file), exist_ok=True)
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(report_text)
        safe_print(f"[FILE] Combined report written to: {output_file}")

    return report_text


def _default_report_path(input_path: str, is_file_input: bool) -> str:
    """Return default report path in the same directory as this script."""
    script_dir = os.path.dirname(os.path.abspath(__file__))

    if is_file_input:
        input_name = os.path.splitext(os.path.basename(input_path))[0]
    else:
        input_name = os.path.basename(os.path.normpath(input_path))

    if not input_name:
        input_name = "integrity"

    return os.path.join(script_dir, f"{input_name}_integrity_report.txt")


def main():
    """Command-line interface for the data integrity validator."""
    parser = argparse.ArgumentParser(description="Universal Data Integrity Validator for Assembly Files")
    parser.add_argument(
        "input_path",
        help="Input assembly file (.asm) OR a directory containing .asm files to validate",
    )
    parser.add_argument(
        "report_path",
        nargs='?',
        help=(
            "Deprecated and ignored. "
            "Reports are now always generated in the script directory using "
            "<input_name>_integrity_report.txt naming."
        ),
    )
    parser.add_argument("--debug", action="store_true", help="Enable debug output")
    parser.add_argument("--quiet", action="store_true", help="Suppress non-error output")
    
    args = parser.parse_args()
    
    if not os.path.exists(args.input_path):
        safe_print(f"[ERROR] ERROR: Input path '{args.input_path}' not found")
        sys.exit(1)
    
    # Create validator
    validator = DataIntegrityValidator(debug_mode=args.debug and not args.quiet)
    
    try:
        input_path = os.path.abspath(args.input_path)
        if args.report_path:
            safe_print("[WARN] WARNING: report_path argument is ignored. Using default naming in script directory.")

        if os.path.isfile(input_path):
            output_report = _default_report_path(input_path, is_file_input=True)
            is_valid = validator.validate_file(input_path, output_report)
            sys.exit(0 if is_valid else 1)

        if not os.path.isdir(input_path):
            safe_print(f"[ERROR] ERROR: '{input_path}' is not a file or directory")
            sys.exit(1)

        # Directory mode: validate all *.asm files in the directory (recursive)
        asm_files: List[str] = []
        for root, _dirs, files in os.walk(input_path):
            for name in files:
                if name.lower().endswith(".asm"):
                    asm_files.append(os.path.join(root, name))
        asm_files.sort(key=lambda p: os.path.relpath(p, input_path).lower())

        if not asm_files:
            safe_print(f"[ERROR] ERROR: No .asm files found in directory '{input_path}'")
            sys.exit(1)

        safe_print(f"[INFO] Validating {len(asm_files)} .asm file(s) under directory (recursive): {input_path}")

        all_valid = True
        file_summaries: List[FileValidationSummary] = []
        for asm_file in asm_files:
            is_valid = validator.validate_file(asm_file, None)

            passed = len([r for r in validator.integrity_results if r.status == "PASSED"])
            failed = len([r for r in validator.integrity_results if r.status == "FAILED"])
            missing = len([r for r in validator.integrity_results if r.status == "MISSING"])
            total = passed + failed + missing
            issues = [r for r in validator.integrity_results if r.status in ["FAILED", "MISSING"]]
            file_summaries.append(FileValidationSummary(
                file_path=asm_file,
                passed=passed,
                failed=failed,
                missing=missing,
                total=total,
                is_valid=is_valid,
                issues=issues
            ))

            all_valid = all_valid and is_valid

        combined_report_file = _default_report_path(input_path, is_file_input=False)
        generate_combined_report(input_path, file_summaries, combined_report_file)

        total_valid_files = sum(1 for s in file_summaries if s.is_valid)
        total_invalid_files = len(file_summaries) - total_valid_files
        safe_print("\n[INFO] DIRECTORY VALIDATION SUMMARY:")
        safe_print(f"   [FILE] Files validated: {len(file_summaries)}")
        safe_print(f"   [OK] Files passed: {total_valid_files}")
        safe_print(f"   [ERROR] Files failed: {total_invalid_files}")

        sys.exit(0 if all_valid else 1)
        
    except Exception as e:
        safe_print(f"[ERROR] ERROR: Validation failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()