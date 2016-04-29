PDFLATEX= pdflatex
LATEX = latex
TARGET= code-design

.SUFFIXES:  .pdf .tex .ps  .dvi 

all: $(TARGET).pdf

.tex.dvi: 
	$(LATEX)  $<

.dvi.ps: 
	$(PS) -o $@ $<

.tex.pdf:
	$(PDFLATEX)  $<


clean:
	rm -f $(TARGET).pdf
