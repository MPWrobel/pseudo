#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_ds.h"

#include "lex.h"

static char *TypeToString(TokenType);

char *TokenNames[] = {
	[TOK_EOF]        = "TOK_EOF",
	[TOK_L_PAREN]    = "TOK_L_PAREN",
	[TOK_R_PAREN]    = "TOK_R_PAREN",
	[TOK_L_BRACE]    = "TOK_L_BRACE",
	[TOK_R_BRACE]    = "TOK_R_BRACE",
	[TOK_COMMA]      = "TOK_COMMA",
	[TOK_SEMICOLON]  = "TOK_SEMICOLON",
	[TOK_ASSIGN]     = "TOK_ASSIGN",
	[TOK_PLUS]       = "TOK_PLUS",
	[TOK_MINUS]      = "TOK_MINUS",
	[TOK_STAR]       = "TOK_STAR",
	[TOK_SLASH]      = "TOK_SLASH",
	[TOK_MOD]        = "TOK_MOD",
	[TOK_EQUAL]      = "TOK_EQUAL",
	[TOK_UNEQUAL]    = "TOK_UNEQUAL",
	[TOK_LESSER]     = "TOK_LESSER",
	[TOK_GREATER]    = "TOK_GREATER",
	[TOK_LESSER_EQ]  = "TOK_LESSER_EQ",
	[TOK_GREATER_EQ] = "TOK_GREATER_EQ",
	[TOK_NOT]        = "TOK_NOT",
	[TOK_AND]        = "TOK_AND",
	[TOK_OR]         = "TOK_OR",
	[TOK_IF]         = "TOK_IF",
	[TOK_ELSE]       = "TOK_ELSE",
	[TOK_FOR]        = "TOK_FOR",
	[TOK_BREAK]      = "TOK_BREAK",
	[TOK_CONTINUE]   = "TOK_CONTINUE",
	[TOK_RETURN]     = "TOK_RETURN",
	[TOK_PROC]       = "TOK_PROC",
	[TOK_LET]        = "TOK_LET",
	[TOK_IDENTIFIER] = "TOK_IDENTIFIER",
	[TOK_STRING]     = "TOK_STRING",
	[TOK_NUMBER]     = "TOK_NUMBER",
};

Symbol Symbols[] = {
	{"(",        TOK_L_PAREN   },
    {")",        TOK_R_PAREN   },
    {"{",        TOK_L_BRACE   },
	{"}",        TOK_R_BRACE   },
    {",",        TOK_COMMA     },
    {";",        TOK_SEMICOLON },
	{"+",        TOK_PLUS      },
    {"-",        TOK_MINUS     },
    {"*",        TOK_STAR      },
	{"/",        TOK_SLASH     },

	{"=",        TOK_ASSIGN    },
    {"==",       TOK_EQUAL     },
    {"!",        TOK_NOT       },
	{"!=",       TOK_UNEQUAL   },
    {"<",        TOK_LESSER    },
    {"<=",       TOK_LESSER_EQ },
	{">",        TOK_GREATER   },
    {">=",       TOK_GREATER_EQ},

	{"and",      TOK_AND       },
    {"break",    TOK_BREAK     },
    {"continue", TOK_CONTINUE  },
	{"else",     TOK_ELSE      },
    {"for",      TOK_FOR       },
    {"if",       TOK_IF        },
	{"let",      TOK_LET       },
    {"or",       TOK_OR        },
    {"proc",     TOK_PROC      },
	{"return",   TOK_RETURN    },

	{NULL,       TOK_EOF       }
};

Token *
Lex(Lexer *lexer)
{
	Token *output = NULL;

	Token token;
	while ((token = NextToken(lexer)).type) arrpush(output, token);
	arrpush(output, token);

	return output;
}

Lexer *
CreateLexer(char *input)
{
	Lexer *lexer = malloc(sizeof(Lexer));

	*lexer = (Lexer){.input = input, .row = 1, .column = 1};

	if ((lexer->current = input[lexer->position++])) {
		lexer->peek = input[lexer->position++];
	}

	Symbol *symbol;
	shdefault(lexer->symbols, TOK_EOF);
	for (symbol = Symbols; symbol->key; symbol++) {
		shputs(lexer->symbols, *symbol);
	}

	return lexer;
}

void
DestroyLexer(Lexer *lexer)
{
	shfree(lexer->symbols);
	free(lexer);
}

void
ReadChar(Lexer *lexer)
{
	lexer->column++;

	lexer->current = lexer->peek;
	if (lexer->current) lexer->peek = lexer->input[lexer->position++];
}

Token
NextToken(Lexer *lexer)
{
	Token token;
	TokenType type = TOK_EOF;
	char *word     = NULL;

	while (isspace(lexer->current)) {
		if (lexer->current == '\n' || lexer->current == '\r') {
			lexer->column = 1;
			lexer->row++;
		}
		ReadChar(lexer);
	}

	if (lexer->current == '\0') {
		arrpush(word, 'E');
		arrpush(word, 'O');
		arrpush(word, 'F');
		arrpush(word, '\0');
	} else if (isalpha(lexer->current)) {
		arrpush(word, lexer->current);
		while (isalnum(lexer->peek)) {
			ReadChar(lexer);
			arrpush(word, lexer->current);
		}
		arrpush(word, '\0');

		type = shget(lexer->symbols, word);
		type = type ? type : TOK_IDENTIFIER;
	} else if (isdigit(lexer->current)) {
		type = TOK_NUMBER;

		arrpush(word, lexer->current);
		while (isdigit(lexer->peek)) {
			ReadChar(lexer);
			arrpush(word, lexer->current);
		}
		arrpush(word, '\0');
	} else if (lexer->current == '"') {
		type = TOK_STRING;

		ReadChar(lexer);
		while (lexer->current != '"') {
			if (lexer->current == '\0') {
				fprintf(stderr, "Unterminated string at line %d\n", lexer->row);
				break;
			}

			arrpush(word, lexer->current);
			ReadChar(lexer);
		}

		arrpush(word, '\0');
	} else {
		arrpush(word, lexer->current);
		arrpush(word, lexer->peek);
		arrpush(word, '\0');

		type = shget(lexer->symbols, word);

		arrpop(word);
		arrpop(word);
		arrpush(word, '\0');

		if (type) ReadChar(lexer);
		else {
			type = shget(lexer->symbols, word);

			if (type == TOK_EOF && lexer->current) {
				fprintf(stderr, "Unexpected character '%c' (0x%X) at line %d\n",
				        lexer->current, lexer->current, lexer->row);
			}
		}
	}

	ReadChar(lexer);
	token = CreateToken(type, word);
	arrfree(word);
	return token;
}

Token
CreateToken(TokenType type, char *value)
{
	return (Token){.type = type, .value = strdup(value)};
}

void
DestroyToken(Token token)
{
	free(token.value);
}

void
PrintToken(Token token)
{
	if (token.type == TOK_STRING) {
		printf("%s:\t\"%s\"\n", TokenNames[token.type], token.value);
	} else {
		printf("%s:\t%s\n", TokenNames[token.type], token.value);
	}
}
