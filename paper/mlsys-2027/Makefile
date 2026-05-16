# MLSys 2027 paper — build pipeline
#
# Targets:
#   make           Build main.pdf (pdflatex + bibtex + pdflatex x2)
#   make clean     Remove build intermediates and PDF
#   make watch     Continuous build via latexmk (requires latexmk)
#
# Tested on TeX Live 2022+. Venue style file (mlsys2027.sty) will be
# swapped in once the call for papers drops; until then we use the
# generic `article` document class declared in main.tex.

TARGET    = main
TEX       = pdflatex
BIB       = bibtex
TEXFLAGS  = -interaction=nonstopmode -halt-on-error

.PHONY: all clean watch

all: $(TARGET).pdf

$(TARGET).pdf: $(TARGET).tex refs.bib
	$(TEX) $(TEXFLAGS) $(TARGET)
	$(BIB) $(TARGET)
	$(TEX) $(TEXFLAGS) $(TARGET)
	$(TEX) $(TEXFLAGS) $(TARGET)

watch:
	latexmk -pdf -pvc -interaction=nonstopmode $(TARGET).tex

clean:
	rm -f $(TARGET).aux $(TARGET).bbl $(TARGET).blg \
	      $(TARGET).fdb_latexmk $(TARGET).fls \
	      $(TARGET).log $(TARGET).out \
	      $(TARGET).synctex.gz $(TARGET).toc \
	      $(TARGET).pdf
