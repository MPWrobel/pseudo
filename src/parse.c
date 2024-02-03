#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "lex.h"
#include "parse.h"
#include "stb_ds.h"
#include "utils.h"

Precedence TokenPrecedence[] = {
	[TOK_PLUS]  = PREC_SUM,
	[TOK_MINUS] = PREC_SUM,
	[TOK_STAR]  = PREC_PRODUCT,
	[TOK_SLASH] = PREC_PRODUCT,
	// [TOK_MOD] = PREC_PRODUCT,
};

Statement *
Parse(Parser *parser)
{
	Statement *statements = NULL, *program = NULL;
	ReadToken(parser);

	Statement statement;
	do {
		statement = ParseStatement(parser);
		if (!statement.type) continue;
		arrpush(statements, statement);
		PrintStatement(&statement);
	} while (statement.type);

	program = ArenaAlloc(parser->arena, sizeof(Statement) * arrlen(statements));
	memcpy(program, statements, sizeof(Statement) * arrlen(statements));
	arrfree(statements);
	return statements;
}

Statement
ParseStatement(Parser *parser)
{
	ReadToken(parser);
	switch (parser->current->type) {
	case TOK_LET: return ParseLetStatement(parser);
	case TOK_RETURN: return ParseReturnStatement(parser);
	default: return (Statement){0};
	}
}

void
ReadToken(Parser *parser)
{
	parser->current = parser->peek;
	parser->peek    = NextToken(parser->lexer);

	if (parser->current) {
		printf(">\t");
		PrintToken(parser->current);
	}
}

Parser *
CreateParser(Lexer *lexer)
{
	MemoryBlock *arena  = CreateArena();
	Parser      *parser = ArenaAlloc(arena, sizeof(Parser));
	*parser             = (Parser){.arena = arena, .lexer = lexer};
	return parser;
}

void
DestroyParser(Parser *parser)
{
	DestroyArena(parser->arena);
}

bool
ExpectToken(Parser *parser, TokenType type)
{
	ReadToken(parser);
	return parser->current->type == type;
}

Statement
ParseExpressionStatement(Parser *parser)
{
	Expression *expression = ParseExpression(parser, PREC_MIN);

	if (!ExpectToken(parser, TOK_SEMICOLON)) puts("Expected a semicolon");

	return (Statement){.type                            = STAT_EXPR,
	                   .statement.expression.expression = expression};
}

Expression *
ParseExpression(Parser *parser, Precedence precedence)
{
	ReadToken(parser);

	Expression *left = ArenaAlloc(parser->arena, sizeof(Expression));

	PrefixExpression  prefix  = {0};
	LiteralExpression literal = {0};
	switch (parser->current->type) {
	case TOK_MINUS:
	case TOK_NOT:
		prefix.operator= parser->current;
		prefix.value = ParseExpression(parser, PREC_MIN);
		left->type   = EXPR_PREFIX;
		left->prefix = prefix;
		break;
	case TOK_STRING:
	case TOK_INTEGER:
		literal.value = parser->current;
		left->type    = EXPR_LITERAL;
		left->literal = literal;
		break;
	default: break;
	}

	switch (parser->peek->type) {
	case TOK_PLUS:
	case TOK_MINUS:
	case TOK_STAR:
	case TOK_SLASH:
	case TOK_MOD: break;
	default: return left;
	}

	InfixExpression infix = {0};
	while (parser->peek->type != TOK_SEMICOLON && precedence < TokenPrecedence[parser->peek->type]) {
		ReadToken(parser);
		infix.value1   = left;
		infix.operator = parser->current;
		infix.value2   = ParseExpression(parser, TokenPrecedence[parser->current->type]);
		left        = ArenaAlloc(parser->arena, sizeof(Expression));
		left->type  = EXPR_INFIX;
		left->infix = infix;
	}

	return left;
}

Statement
ParseLetStatement(Parser *parser)
{
	LetStatement statement = {0};

	if (ExpectToken(parser, TOK_IDENTIFIER)) {
		statement.identifier = parser->current;
	} else {
		puts("Expected an identifier");
		return (Statement){0};
	}

	if (!ExpectToken(parser, TOK_ASSIGN)) {
		puts("Expected an assignment operator");
		return (Statement){0};
	}

	Expression *value = ParseExpression(parser, PREC_MIN);
	if (value->type) statement.value = value;
	else return (Statement){0};

	if (!ExpectToken(parser, TOK_SEMICOLON)) puts("Expected a semicolon");

	return (Statement){.type = STAT_LET, .statement.let = statement};
}

Statement
ParseReturnStatement(Parser *parser)
{
	ReturnStatement statement = {.start = parser->current};

	Expression *value = ParseExpression(parser, PREC_MIN);
	if (value->type) statement.value = value;
	else return (Statement){0};

	if (!ExpectToken(parser, TOK_SEMICOLON)) puts("Expected a semicolon");

	return (Statement){.type = STAT_RETURN, .statement.return_ = statement};
}

void
PrintExpression(Expression *expression)
{
	switch (expression->type) {
	case EXPR_LITERAL:
		Print("LITERAL_EXPRESSION:");
		BeginIndent();

		Print("Value: %s", TokenString(expression->literal.value));

		EndIndent();
		break;
	case EXPR_PREFIX:
		Print("PREFIX_EXPRESSION:");
		BeginIndent();

		Print("Operator: %s", TokenString(expression->prefix.operator));

		Print("Value:");
		BeginIndent();
		PrintExpression(expression->prefix.value);
		EndIndent();

		EndIndent();
		break;
	case EXPR_INFIX:
		Print("INFIX_EXPRESSION");
		BeginIndent();

		Print("Operator: %s", TokenString(expression->infix.operator));

		Print("Value1:");
		BeginIndent();
		PrintExpression(expression->infix.value1);
		EndIndent();

		Print("Value2:");
		BeginIndent();
		PrintExpression(expression->infix.value2);
		EndIndent();

		EndIndent();
		break;
	default: break;
	}
}

void
PrintStatement(Statement *statement)
{
	switch (statement->type) {
	case STAT_LET:
		Print("LET_STATEMENT:");
		BeginIndent();

		Print("Identifier: %s",
		      TokenString(statement->statement.let.identifier));

		Print("Value:");
		BeginIndent();
		PrintExpression(statement->statement.let.value);
		EndIndent();

		EndIndent();
		break;
	case STAT_RETURN:
		Print("RETURN_STATEMENT:");
		BeginIndent();

		Print("Value:");
		BeginIndent();
		PrintExpression(statement->statement.let.value);
		EndIndent();

		EndIndent();
		break;
	default: break;
	}
}
