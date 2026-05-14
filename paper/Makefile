# Workshop paper build — Phase 7a M3 (A-stage of A→B staging).
#
# Targets:
#   make paper      → build neurips-workshop-2026.pdf
#   make watch      → run latexmk in continuous-build mode
#   make clean      → remove build artefacts (keep PDF)
#   make distclean  → remove build artefacts AND the PDF
#
# Requirements: pdflatex + bibtex (TeX Live ≥ 2023 tested).
# Optional: latexmk for continuous build (`make watch`).

MAIN := neurips-workshop-2026
TEX  := $(MAIN).tex
BIB  := refs.bib
PDF  := $(MAIN).pdf

.PHONY: paper watch clean distclean

paper: $(PDF)

$(PDF): $(TEX) $(BIB)
	pdflatex -interaction=nonstopmode $(MAIN)
	bibtex   $(MAIN)
	pdflatex -interaction=nonstopmode $(MAIN)
	pdflatex -interaction=nonstopmode $(MAIN)

watch:
	latexmk -pdf -pvc -interaction=nonstopmode $(MAIN)

clean:
	rm -f $(MAIN).{aux,bbl,blg,fdb_latexmk,fls,log,out,synctex.gz,toc}

distclean: clean
	rm -f $(PDF)
