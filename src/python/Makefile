
all: slr-gen lr-parse

slr-gen:
	python ./syntax.py --slr ./krc-lr.syntax --dump-file=./krc-lr.table

lr-parse:
	python ./syntax.py --lr ./krc-lr.table --token-file=./lex_test.c
