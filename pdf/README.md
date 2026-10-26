# PDF Documentation Folder

This folder contains the compiled, publication-grade academic PDF files for the **Firewall Rule Engine in Modern C++17**, along with the automated toolchain used to generate them.

---

## 1. Generated PDF Files

| PDF File Name | Source Markdown File | Classification | Description |
| :--- | :--- | :--- | :--- |
| **`Firewall_Rule_Engine_Project_Documentation.pdf`** | [`Firewall_Rule_Engine_Project_Documentation.md`](../Firewall_Rule_Engine_Project_Documentation.md) | **Primary Faculty Submission** | Complete 13-section technical report covering project overview, theoretical background, tripartite gap analysis, system architecture, OOP implementation, algorithms, test verification (15/15 passed), empirical benchmark results, and RFC references. |
| **`OOP_Concepts_and_Architecture.pdf`** | [`OOP_Concepts_and_Architecture.md`](../OOP_Concepts_and_Architecture.md) | **Viva Defense Reference** | Dedicated OOP and architectural deep-dive detailing Encapsulation, Abstraction, Inheritance, Runtime Polymorphism, Move-only Rules, RAII, STL usage, Design Patterns (Specification, Strategy, Observer, Facade), and faculty viva questions & answers. |
| **`Research_Gap_and_Methodology.pdf`** | [`Research_Gap_and_Methodology.md`](../Research_Gap_and_Methodology.md) | **Methodology Report** | Detailed analysis of the Tripartite Gap Model, Gap-to-Solution Mapping Table, 11-phase development lifecycle, and end-to-end packet evaluation pipeline. |

---

## 2. Tools & Code Used to Generate PDFs

The generation toolchain is completely self-contained in this directory:

- **`generate_pdfs.py`**: Automated Python script that:
  1. Parses Markdown documents using `python-markdown` with tables, fenced code, and TOC extensions.
  2. Renders syntax-highlighted C++17 source code using Pygments.
  3. Integrates Mermaid.js (v10) to dynamically compile flowcharts, UML class diagrams, and sequence diagrams into vector SVGs.
  4. Injects publication-grade academic CSS tailored for A4 print layout (clean margins, zebra-striped tables, styled code blocks, callouts, and page-break optimization).
  5. Automates a headless Chromium/Edge browser engine (`--headless=new`, `--virtual-time-budget=10000`, `--print-to-pdf`) to produce crisp, high-resolution vector PDFs.
- **`make_pdfs.bat`**: Double-clickable Windows batch launcher to recompile all PDFs anytime.

---

## 3. How to Re-generate the PDFs

To recompile the PDFs after making any updates to the Markdown documents, run either:

### Option A: Using the Windows Batch File
Double-click `make_pdfs.bat` in File Explorer, or run in terminal:
```cmd
.\pdf\make_pdfs.bat
```

### Option B: Using Python directly
```bash
python pdf/generate_pdfs.py
```

### Prerequisites
- Python 3.10+ with `markdown` (`pip install markdown`)
- Google Chrome or Microsoft Edge installed on Windows (detected automatically)
