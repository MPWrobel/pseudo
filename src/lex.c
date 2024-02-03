#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_ds.h"

#include "lex.h"
#include "utils.h"

static char *TypeToString(TokenType);

char *TokenNames[] = {
	[TOK_EOF]        = "EOF",
	[TOK_COMMENT]    = "COMMENT",
	[TOK_L_PAREN]    = "L_PAREN",
	[TOK_R_PAREN]    = "R_PAREN",
	[TOK_L_BRACE]    = "L_BRACE",
	[TOK_R_BRACE]    = "R_BRACE",
	[TOK_COMMA]      = "COMMA",
	[TOK_SEMICOLON]  = "SEMICOLON",
	[TOK_ASSIGN]     = "ASSIGN",
	[TOK_PLUS]       = "PLUS",
	[TOK_MINUS]      = "MINUS",
	[TOK_STAR]       = "STAR",
	[TOK_SLASH]      = "SLASH",
	[TOK_MOD]        = "MOD",
	[TOK_EQUAL]      = "EQUAL",
	[TOK_UNEQUAL]    = "UNEQUAL",
	[TOK_LESSER]     = "LESSER",
	[TOK_GREATER]    = "GREATER",
	[TOK_LESSER_EQ]  = "LESSER_EQ",
	[TOK_GREATER_EQ] = "GREATER_EQ",
	[TOK_NOT]        = "NOT",
	[TOK_AND]        = "AND",
	[TOK_OR]         = "OR",
	[TOK_IF]         = "IF",
	[TOK_ELSE]       = "ELSE",
	[TOK_FOR]        = "FOR",
	[TOK_BREAK]      = "BREAK",
	[TOK_CONTINUE]   = "CONTINUE",
	[TOK_RETURN]     = "RETURN",
	[TOK_PROC]       = "PROC",
	[TOK_LET]        = "LET",
	[TOK_IDENTIFIER] = "IDENTIFIER",
	[TOK_STRING]     = "STRING",
	[TOK_INTEGER]    = "INTEGER",
};

TokenType
GetTokenType(char *string)
{
	switch (string[0]) {
	case '(':
		if (strlen(string) == 1) return TOK_L_PAREN;
	case ')':
		if (strlen(string) == 1) return TOK_R_PAREN;
	case '{':
		if (strlen(string) == 1) return TOK_L_BRACE;
	case '}':
		if (strlen(string) == 1) return TOK_R_BRACE;
	case ',':
		if (strlen(string) == 1) return TOK_COMMA;
	case ';':
		if (strlen(string) == 1) return TOK_SEMICOLON;
	case '+':
		if (strlen(string) == 1) return TOK_PLUS;
	case '-':
		if (strlen(string) == 1) return TOK_MINUS;
	case '*':
		if (strlen(string) == 1) return TOK_STAR;
	case '/':
		if (strlen(string) == 1) return TOK_SLASH;

	case '=':
		if (strcmp(string, "=") == 0) return TOK_ASSIGN;
		if (strcmp(string, "==") == 0) return TOK_EQUAL;
	case '!':
		if (strcmp(string, "!") == 0) return TOK_NOT;
		if (strcmp(string, "!=") == 0) return TOK_UNEQUAL;
	case '<':
		if (strcmp(string, "<") == 0) return TOK_LESSER;
		if (strcmp(string, "<=") == 0) return TOK_LESSER_EQ;
	case '>':
		if (strcmp(string, ">") == 0) return TOK_GREATER;
		if (strcmp(string, ">=") == 0) return TOK_GREATER_EQ;

	case 'a':
		if (strcmp(string, "and") == 0) return TOK_AND;
	case 'b':
		if (strcmp(string, "break") == 0) return TOK_BREAK;
	case 'c':
		if (strcmp(string, "continue") == 0) return TOK_CONTINUE;
	case 'e':
		if (strcmp(string, "else") == 0) return TOK_ELSE;
	case 'f':
		if (strcmp(string, "for") == 0) return TOK_FOR;
	case 'i':
		if (strcmp(string, "if") == 0) return TOK_IF;
	case 'l':
		if (strcmp(string, "let") == 0) return TOK_LET;
	case 'o':
		if (strcmp(string, "or") == 0) return TOK_OR;
	case 'p':
		if (strcmp(string, "proc") == 0) return TOK_PROC;
	case 'r':
		if (strcmp(string, "return") == 0) return TOK_RETURN;

	default: return TOK_EOF;
	}
}

Lexer *
CreateLexer(char *input)
{
	MemoryBlock *arena = CreateArena();

	Lexer *lexer = ArenaAlloc(arena, sizeof(Lexer));

	*lexer = (Lexer){.arena = arena, .input = input, .row = 1, .column = 1};

	if ((lexer->current = input[lexer->position++])) {
		lexer->peek = input[lexer->position++];
	}

	return lexer;
}

void
DestroyLexer(Lexer *lexer)
{
	DestroyArena(lexer->arena);
}

void
ReadChar(Lexer *lexer)
{
	lexer->column++;

	lexer->current = lexer->peek;
	if (lexer->current) lexer->peek = lexer->input[lexer->position++];
}

Token *
NextToken(Lexer *lexer)
{
	Token *token = ArenaAlloc(lexer->arena, sizeof(Token));
	char  *word  = NULL;

	*token = (Token){0};

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
	} else if (lexer->current == '#') {
		token->type = TOK_COMMENT;
		while (lexer->current != '\0' && lexer->current != '\n') {
			arrpush(word, lexer->current);
			ReadChar(lexer);
		}
		arrpush(word, '\0');
	} else if (isalpha(lexer->current)) {
		do {
			arrpush(word, lexer->current);
			ReadChar(lexer);
		} while (isalnum(lexer->current));
		arrpush(word, '\0');

		token->type = GetTokenType(word);
		token->type = token->type ? token->type : TOK_IDENTIFIER;
	} else if (isdigit(lexer->current)) {
		token->type = TOK_INTEGER;

		do {
			arrpush(word, lexer->current);
			ReadChar(lexer);
		} while (isdigit(lexer->current));
		arrpush(word, '\0');
	} else if (lexer->current == '"') {
		token->type = TOK_STRING;

		ReadChar(lexer);
		while (lexer->current != '"') {
			if (lexer->current == '\0') {
				token->type = TOK_EOF;
				while (arrlen(word)) arrpop(word);
				arrpush(word, 'E');
				arrpush(word, 'O');
				arrpush(word, 'F');
				arrpush(word, '\0');
				fprintf(stderr, "Unterminated string at line %d\n", lexer->row);
				break;
			}

			arrpush(word, lexer->current);
			ReadChar(lexer);
		}

		arrpush(word, '\0');

		ReadChar(lexer);
	} else {
		arrpush(word, lexer->current);
		arrpush(word, lexer->peek);
		arrpush(word, '\0');

		token->type = GetTokenType(word);

		arrpop(word);
		arrpop(word);
		arrpush(word, '\0');

		if (token->type) ReadChar(lexer);
		else {
			token->type = GetTokenType(word);

			if (token->type == TOK_EOF && lexer->current) {
				fprintf(stderr, "Unexpected character '%c' (0x%X) at line %d\n",
				        lexer->current, lexer->current, lexer->row);
			}
		}

		ReadChar(lexer);
	}

	token->value = ArenaAlloc(lexer->arena, arrlen(word));
	strncpy(token->value, word, arrlen(word));
	arrfree(word);

	return token;
}

char *
TokenString(Token *token)
{
	static char string[64];
	if (token->type == TOK_STRING) {
		snprintf(string, sizeof(string), "%s(\"%s\")", TokenNames[token->type], token->value);
	} else {
		snprintf(string, sizeof(string), "%s(%s)", TokenNames[token->type], token->value);
	}
	return string;
}

void
PrintToken(Token *token)
{
	if (token->type == TOK_STRING) {
		Print("%s(\"%s\")", TokenNames[token->type], token->value);
	} else {
		Print("%s(%s)", TokenNames[token->type], token->value);
	}
}
