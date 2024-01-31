#include <readline/readline.h>
#include <stdio.h>
#include <sysexits.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "lex.h"
#include "parse.h"
#include "util.h"

void LaunchREPL();
void RunFile(char *, char **);

int
main(int argc, char **argv)
{
	if (argc > 1) RunFile(argv[1], argv + 1);
	else LaunchREPL();
}

void
LaunchREPL()
{
	char *line;

	for (;;) {
		line = readline("> ");
		if (!line) break;

		Lexer  *lexer  = CreateLexer(line);

		Token *token;
		while((token = NextToken(lexer))->type) PrintToken(token);

		DestroyLexer(lexer);

		free(line);
	}
}

void
RunFile(char *script, char **argv)
{
	FILE *file;
	char *buf;

	file = fopen(script, "r");
	if (!file) {
		fprintf(stderr, "File \"%s\" does not exits\n", script);
		return;
	}

	buf = ReadFile(file);

	Lexer  *lexer  = CreateLexer(buf);

	Token *token;
	while((token = NextToken(lexer))->type) PrintToken(token);

	DestroyLexer(lexer);

	free(buf);
}
