
all: slr-gen lr-parse

slr-gen:
	python ./syntax.py --slr ./krc-lr.syntax --dump-file=./krc-lr.table

lr-gen:
	python ./syntax.py --lr1 ./krc-lr.syntax --dump-file=./krc-lr.table

lalr-gen:
	python ./syntax.py --lalr ./krc-lr.syntax --dump-file=./krc-lr.tabl

earley-parse:
	python ./syntax.py --earley ./krc-lr.syntax --token-file=./lex_test.c

lr-parse:
	python ./syntax.py --lr ./krc-lr.table --token-file=./lex_test.c
