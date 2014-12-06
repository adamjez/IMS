all: dokumentace.pdf

%.eps: %.png
	convert $< $@
	pdftops -eps $@

dokumentace.dvi: dokumentace.tex
	latex dokumentace.tex

dokumentace.ps: dokumentace.dvi
	dvips dokumentace.dvi

dokumentace.pdf: dokumentace.ps
	ps2pdf dokumentace.ps

.PHONY: clean
clean:
	rm -fr *.aux *.log *.tex# *.ps *.dvi *.out *.pdf