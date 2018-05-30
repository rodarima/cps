PDFS=report.pdf

all: $(PDFS)

%.pdf: %.tex
	latexmk -pdf -latexoption="-shell-escape" $<
	@kill -HUP $$(pidof mupdf)

clean:
	rm -f ${PDFS} *.{aux,log}
