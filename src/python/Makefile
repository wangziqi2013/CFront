
all: slr-gen lr-parse

slr-gen:
	python ./syntax.py --slr ./krc-lr.syntax --dump-file=./krc-lr.table --sub-syntax

lr-gen:
	python ./syntax.py --lr1 ./krc-lr.syntax --dump-file=./krc-lr.table --sub-syntax

lalr-gen:
	python ./syntax.py --lalr ./krc-lr.syntax --dump-file=./krc-lr.table --sub-syntax

lr-parse:
	python ./syntax.py --lr ./krc-lr.table --token-file=./lex_test.c
