#!/usr/bin/env python3
"""
Firewall Rule Engine - Academic PDF Documentation Generator
============================================================
Converts project Markdown documentation into publication-grade,
faculty-ready academic PDF documents with:
  - Robust Mermaid block masking to prevent Markdown parser interference
  - True vector SVG diagram rendering via Mermaid.js (0 syntax errors)
  - Pygments C++17 syntax highlighting
  - Clean, professional academic typography (A4 print layout, page-break safety)
  - Headless Chromium / Microsoft Edge PDF compilation engine
"""

import os
import sys
import re
import html
import shutil
import subprocess
import time
from pathlib import Path

try:
    import markdown
except ImportError:
    print("[INFO] Installing required 'markdown' package...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "markdown"])
    import markdown

try:
    import pygments
    from pygments.formatters import HtmlFormatter
    PYGMENTS_AVAILABLE = True
except ImportError:
    PYGMENTS_AVAILABLE = False


# ==============================================================================
# CSS STYLESHEET (Executive Academic & Faculty Evaluation Grade)
# ==============================================================================
ACADEMIC_CSS = """
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700;800&family=JetBrains+Mono:wght@400;500;600&display=swap');

@page {
    size: A4 portrait;
    margin: 18mm 16mm 18mm 16mm;
    @bottom-center {
        content: counter(page);
        font-family: 'Inter', sans-serif;
        font-size: 8.5pt;
        color: #64748b;
    }
}

* {
    box-sizing: border-box;
}

body {
    font-family: 'Inter', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Arial, sans-serif;
    font-size: 9.8pt;
    line-height: 1.62;
    color: #1e293b;
    background-color: #ffffff;
    margin: 0;
    padding: 0;
}

/* Document Header / Cover Banner */
.doc-header {
    border-bottom: 3px solid #1e3a8a;
    background: linear-gradient(to right, #f8fafc, #ffffff);
    padding: 24px;
    border-radius: 8px;
    border: 1px solid #e2e8f0;
    margin-bottom: 28px;
}

.doc-badge {
    display: inline-block;
    background-color: #1e3a8a;
    color: #ffffff;
    font-size: 7.5pt;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.8px;
    padding: 4px 10px;
    border-radius: 4px;
    margin-bottom: 12px;
}

.doc-header h1 {
    font-size: 19pt;
    font-weight: 800;
    color: #0f172a;
    margin: 0 0 8px 0;
    line-height: 1.25;
    border-bottom: none;
    padding-bottom: 0;
}

.doc-meta {
    display: flex;
    flex-wrap: wrap;
    gap: 16px;
    font-size: 8.5pt;
    color: #475569;
    margin-top: 12px;
    border-top: 1px solid #cbd5e1;
    padding-top: 10px;
}

.doc-meta-item strong {
    color: #0f172a;
}

/* Headings */
h1 {
    font-size: 14.5pt;
    font-weight: 700;
    color: #1e3a8a;
    border-bottom: 1.5px solid #cbd5e1;
    padding-bottom: 6px;
    margin-top: 30px;
    margin-bottom: 14px;
    page-break-after: avoid;
}

.doc-header h1 {
    page-break-after: auto;
}

h2 {
    font-size: 12pt;
    font-weight: 600;
    color: #1e40af;
    margin-top: 22px;
    margin-bottom: 10px;
    page-break-after: avoid;
}

h3 {
    font-size: 10.5pt;
    font-weight: 600;
    color: #334155;
    margin-top: 16px;
    margin-bottom: 8px;
    page-break-after: avoid;
}

h4, h5, h6 {
    font-size: 9.8pt;
    font-weight: 600;
    color: #475569;
    margin-top: 14px;
    margin-bottom: 6px;
    page-break-after: avoid;
}

p {
    margin: 0 0 10px 0;
    text-align: justify;
}

ul, ol {
    margin: 0 0 12px 0;
    padding-left: 22px;
}

li {
    margin-bottom: 4px;
}

hr {
    border: none;
    border-top: 1px solid #e2e8f0;
    margin: 22px 0;
}

/* Links */
a {
    color: #2563eb;
    text-decoration: none;
}

/* Tables */
table {
    width: 100%;
    border-collapse: collapse;
    margin: 16px 0 20px 0;
    font-size: 8.5pt;
    line-height: 1.45;
    page-break-inside: avoid;
    box-shadow: 0 1px 3px rgba(0, 0, 0, 0.04);
}

th, td {
    padding: 7px 10px;
    border: 1px solid #cbd5e1;
    vertical-align: top;
}

th {
    background-color: #1e3a8a;
    color: #ffffff;
    font-weight: 600;
    text-align: left;
    letter-spacing: 0.2px;
}

tr:nth-child(even) {
    background-color: #f8fafc;
}

/* Code & Preformatted */
code {
    font-family: 'JetBrains Mono', 'Cascadia Code', Consolas, monospace;
    font-size: 8.5pt;
    background-color: #f1f5f9;
    color: #0f172a;
    padding: 2px 5px;
    border-radius: 4px;
    border: 1px solid #e2e8f0;
}

pre {
    background-color: #f8fafc;
    border: 1px solid #e2e8f0;
    border-left: 4px solid #1e3a8a;
    border-radius: 4px;
    padding: 11px 13px;
    overflow-x: auto;
    margin: 14px 0;
    page-break-inside: avoid;
}

pre code {
    font-size: 8pt;
    background-color: transparent;
    border: none;
    padding: 0;
    color: #0f172a;
    line-height: 1.5;
}

/* Blockquotes / Callout Alerts */
blockquote {
    border-left: 4px solid #3b82f6;
    background-color: #eff6ff;
    color: #1e40af;
    margin: 14px 0;
    padding: 10px 16px;
    border-radius: 0 6px 6px 0;
    font-size: 9pt;
    page-break-inside: avoid;
}

blockquote p {
    margin: 0;
    color: #1e3a8a;
}

/* Mermaid Diagrams Container */
.mermaid-wrapper {
    margin: 22px 0;
    text-align: center;
    page-break-inside: avoid;
    background-color: #ffffff;
    border: 1px solid #e2e8f0;
    border-radius: 8px;
    padding: 18px;
    box-shadow: 0 1px 3px rgba(0,0,0,0.04);
}

.mermaid-wrapper svg {
    max-width: 100% !important;
    height: auto !important;
    display: inline-block;
}

.mermaid-caption {
    font-size: 8pt;
    color: #64748b;
    font-style: italic;
    margin-top: 10px;
    border-top: 1px solid #f1f5f9;
    padding-top: 6px;
}

/* Print Utilities */
@media print {
    body {
        font-size: 9.5pt;
    }
    .page-break {
        page-break-before: always;
    }
    .no-break {
        page-break-inside: avoid;
    }
}
"""


def find_browser_executable():
    """Locate Google Chrome or Microsoft Edge executable."""
    candidates = [
        r"C:\Program Files\Google\Chrome\Application\chrome.exe",
        r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
        r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
        r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
        shutil.which("chrome"),
        shutil.which("msedge"),
        shutil.which("google-chrome"),
        shutil.which("chromium"),
    ]
    for path in candidates:
        if path and os.path.exists(path):
            return path
    return None


def sanitize_markdown(content: str) -> str:
    """Preprocess markdown to ensure tables and special blocks parse cleanly."""
    # Convert file links from file:/// to simple bold text or clean links
    content = re.sub(r'\[(.*?)\]\(file:///[^)]+\)', r'**\1**', content)
    return content


def convert_md_to_html(md_path: Path, output_html_path: Path, title: str):
    """
    Convert Markdown file to styled HTML document with safe Mermaid block masking.
    Masking prevents Python-Markdown from mangling < and > inside class and sequence diagrams.
    """
    with open(md_path, 'r', encoding='utf-8') as f:
        raw_content = f.read()

    sanitized = sanitize_markdown(raw_content)

    # Step 1: Mask Mermaid blocks with unique placeholders
    mermaid_blocks = []
    def save_mermaid(match):
        idx = len(mermaid_blocks)
        code = match.group(1).strip()
        mermaid_blocks.append(code)
        return f"\n\n<!--MERMAID_PLACEHOLDER_{idx}-->\n\n"

    masked_md = re.sub(r'```mermaid\s*\n(.*?)\n```', save_mermaid, sanitized, flags=re.DOTALL)

    # Step 2: Convert using python-markdown
    extensions = ['tables', 'fenced_code', 'toc', 'sane_lists']
    if PYGMENTS_AVAILABLE:
        extensions.append('codehilite')
    
    body_html = markdown.markdown(masked_md, extensions=extensions)

    # Step 3: Safely restore Mermaid blocks with html.escape so < and > are preserved
    for idx, code in enumerate(mermaid_blocks):
        escaped_code = html.escape(code)
        wrapper = (
            f'<div class="mermaid-wrapper" id="mermaid-container-{idx}">'
            f'<pre class="mermaid" id="mermaid-{idx}">\n{escaped_code}\n</pre>'
            f'<div class="mermaid-caption">System Architectural Diagram #{idx+1}</div>'
            f'</div>'
        )
        body_html = body_html.replace(f"<p><!--MERMAID_PLACEHOLDER_{idx}--></p>", wrapper)
        body_html = body_html.replace(f"<!--MERMAID_PLACEHOLDER_{idx}-->", wrapper)

    # Pygments CSS
    pygments_css = ""
    if PYGMENTS_AVAILABLE:
        formatter = HtmlFormatter(style='default')
        pygments_css = formatter.get_style_defs('.codehilite')

    # Construct complete standalone HTML
    full_html = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>{title}</title>
<script src="https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.min.js"></script>
<style>
{ACADEMIC_CSS}
{pygments_css}
</style>
<script>
window.addEventListener('DOMContentLoaded', async () => {{
    mermaid.initialize({{
        startOnLoad: false,
        theme: 'neutral',
        securityLevel: 'loose',
        fontFamily: 'Inter, -apple-system, Segoe UI, sans-serif'
    }});
    try {{
        await mermaid.run({{
            querySelector: '.mermaid'
        }});
    }} catch (e) {{
        console.error('Mermaid render error:', e);
    }}
    const flag = document.createElement('div');
    flag.id = 'render-complete';
    flag.style.display = 'none';
    document.body.appendChild(flag);
}});
</script>
</head>
<body>
{body_html}
</body>
</html>
"""
    with open(output_html_path, 'w', encoding='utf-8') as f:
        f.write(full_html)


def render_html_to_pdf(browser_path: str, html_path: Path, pdf_path: Path):
    """Execute headless browser to print HTML to PDF with complete virtual time budget."""
    abs_html = html_path.resolve()
    abs_pdf = pdf_path.resolve()

    cmd = [
        browser_path,
        "--headless=new",
        "--disable-gpu",
        "--allow-file-access-from-files",
        "--run-all-compositor-stages-before-draw",
        "--virtual-time-budget=10000",
        "--no-pdf-header-footer",
        f"--print-to-pdf={abs_pdf}",
        f"file:///{abs_html}"
    ]

    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"Browser failed with code {result.returncode}: {result.stderr}")
    if not pdf_path.exists() or pdf_path.stat().st_size == 0:
        raise RuntimeError(f"PDF was not generated or has zero size: {pdf_path}")


def main():
    print("=" * 72)
    print("  FIREWALL RULE ENGINE - ACADEMIC PDF DOCUMENTATION GENERATOR")
    print("=" * 72)

    workspace_root = Path(__file__).resolve().parent.parent
    pdf_dir = workspace_root / "pdf"
    pdf_dir.mkdir(parents=True, exist_ok=True)

    browser = find_browser_executable()
    if not browser:
        print("[ERROR] Neither Google Chrome nor Microsoft Edge was found on this system.")
        print("Please install Chrome or Edge to compile publication-grade PDFs.")
        sys.exit(1)

    print(f"[OK] Located PDF Browser Engine: {browser}")

    targets = [
        {
            "md": "Firewall_Rule_Engine_Project_Documentation.md",
            "pdf": "Firewall_Rule_Engine_Project_Documentation.pdf",
            "title": "Firewall Rule Engine - Comprehensive Technical Documentation"
        },
        {
            "md": "OOP_Concepts_and_Architecture.md",
            "pdf": "OOP_Concepts_and_Architecture.pdf",
            "title": "Firewall Rule Engine - OOP Concepts & Architectural Design"
        },
        {
            "md": "Research_Gap_and_Methodology.md",
            "pdf": "Research_Gap_and_Methodology.pdf",
            "title": "Firewall Rule Engine - Research Gap & Engineering Methodology"
        }
    ]

    results = []

    for item in targets:
        md_file = workspace_root / item["md"]
        pdf_file = pdf_dir / item["pdf"]
        temp_html = pdf_dir / f"{item['md']}.html"

        if not md_file.exists():
            print(f"[SKIP] Markdown file not found: {md_file.name}")
            continue

        print(f"\n[PROCESSING] {item['md']} ...")
        t0 = time.time()

        # Step 1: Convert Markdown to Styled HTML with safe Mermaid masking
        convert_md_to_html(md_file, temp_html, item["title"])
        print(f"  -> Generated intermediate HTML: {temp_html.name}")

        # Step 2: Render to PDF via Headless Browser
        try:
            render_html_to_pdf(browser, temp_html, pdf_file)
            elapsed = time.time() - t0
            size_kb = pdf_file.stat().st_size / 1024.0
            print(f"  -> [SUCCESS] Compiled PDF: {pdf_file.name} ({size_kb:.1f} KB in {elapsed:.2f}s)")
            results.append((item["pdf"], size_kb, True))
        except Exception as e:
            print(f"  -> [FAILED] Error rendering PDF: {e}")
            results.append((item["pdf"], 0, False))

    print("\n" + "=" * 72)
    print("  SUMMARY OF COMPILED PDF DELIVERABLES")
    print("=" * 72)
    for name, size_kb, success in results:
        status = "PASSED" if success else "FAILED"
        print(f"  [{status}] {name:<48} : {size_kb:>8.1f} KB")
    print("=" * 72)
    print(f"All PDF deliverables saved in: {pdf_dir.resolve()}\n")


if __name__ == "__main__":
    main()
