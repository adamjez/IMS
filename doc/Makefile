all: dokumentace.pdf

dokumentace.pdf: dokumentace.tex
	pdflatex dokumentace.tex

.PHONY: clean
clean:
	rm -fr *.aux *.toc *.log *.tex# *.ps *.dvi *.out *.pdf
