P = pdflatex -halt-on-error

all: principal

codi:
	sh neteja_accents.sh

complet: codi
	$P principal  > /dev/null
	$P principal > /dev/null

evince:
	evince principal.pdf

index:
	makeindex principal.tex

principal:
	$P principal.tex

clean:
	rm *.aux *.bak *.idx *.ilg *.ind *.lof *.lot *.log *.out *.toc *~ *.bbl *.blg 2> /dev/null

