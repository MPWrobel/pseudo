#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "lex.h"
#include "parse.h"
#include "stb_ds.h"
#include "utils.h"

Precedence TokenPrecedence[] = {
	[TOK_PLUS]       = PREC_SUM,
	[TOK_MINUS]      = PREC_SUM,
	[TOK_STAR]       = PREC_PRODUCT,
	[TOK_SLASH]      = PREC_PRODUCT,
	// [TOK_MOD] = PREC_PRODUCT,
	[TOK_IDENTIFIER] = PREC_CALL,
};

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

void
ReadToken(Parser *parser)
{
	parser->current = parser->peek;
	parser->peek    = NextToken(parser->lexer);

	if (parser->current) {
		printf(">\t");
		Print(TokenString(parser->current));
	}
}

void
ExpectToken(Parser *parser, TokenType type)
{
	ReadToken(parser);
	if (parser->current->type == type) return;

	Token token = {.type = type};
	fprintf(stderr, "Unexpected token: %s\n", TokenString(parser->current));
	fprintf(stderr, "Expected: %s\n", TokenString(&token));
	fprintf(stderr, "At line %d\n", parser->current->row);
	exit(200);
}

Statement *
Parse(Parser *parser)
{
	Statement *program;

	ReadToken(parser);

	Statement *statements = NULL;
	for (;;) {
		Statement statement = ParseStatement(parser);
		if (!statement.type) break;

		arrpush(statements, statement);
		PrintStatement(&statement);
	}

	arrpush(statements, (Statement){.type = STAT_INVALID});
	program = ArenaAlloc(parser->arena, arrlen(statements) * sizeof(Statement));
	memcpy(program, statements, arrlen(statements) * sizeof(Statement));
	arrfree(statements);

	return program;

}

Statement
ParseStatement(Parser *parser)
{
	switch (parser->peek->type) {
	case TOK_PROC: return ParseProcStatement(parser);
	case TOK_LET: return ParseLetStatement(parser);
	case TOK_RETURN: return ParseReturnStatement(parser);
	case TOK_IDENTIFIER:
	case TOK_MINUS:
	case TOK_INTEGER: return ParseExpressionStatement(parser);
	default: return (Statement){0};
	}
}

BlockStatement
ParseBlockStatement(Parser *parser)
{
	BlockStatement block;
	Statement *statements = NULL;

	ExpectToken(parser, TOK_L_BRACE);
	while (parser->peek->type != TOK_R_BRACE) {
		Statement statement = ParseStatement(parser);
		if (!statement.type) break;

		arrpush(statements, statement);
		PrintStatement(&statement);
	}
	ExpectToken(parser, TOK_R_BRACE);

	arrpush(statements, (Statement){.type = STAT_INVALID});
	block.count      = arrlen(statements);
	block.statements = ArenaAlloc(parser->arena, block.count * sizeof(Statement));
	memcpy(block.statements, statements, block.count * sizeof(Statement));
	arrfree(statements);

	return block;
}

Statement
ParseProcStatement(Parser *parser)
{
	ExpectToken(parser, TOK_PROC);
	ProcStatement statement;

	ExpectToken(parser, TOK_IDENTIFIER);
	statement.identifier = parser->current;

	ExpectToken(parser, TOK_L_PAREN);
	Token **arguments = NULL;

	if (parser->peek->type != TOK_R_PAREN) {
		ExpectToken(parser, TOK_IDENTIFIER);
		arrpush(arguments, parser->current);
		while (parser->peek->type == TOK_COMMA) {
			ReadToken(parser);
			ExpectToken(parser, TOK_IDENTIFIER);
			arrpush(arguments, parser->current);
		}
	}
	
	statement.arity = arrlen(arguments);
	if (statement.arity > 0) {
		statement.arguments = ArenaAlloc(parser->arena, statement.arity * sizeof(Token *));
		memcpy(statement.arguments, arguments, statement.arity * sizeof(Token *));
	}
	arrfree(arguments);

	ExpectToken(parser, TOK_R_PAREN);

	statement.body  = ArenaAlloc(parser->arena, sizeof(BlockStatement));
	*statement.body = ParseBlockStatement(parser);

	return (Statement){.type = STAT_PROC, .proc = statement};
}

Statement
ParseExpressionStatement(Parser *parser)
{
	Expression *expression = ParseExpression(parser, PREC_MIN);

	ExpectToken(parser, TOK_SEMICOLON);

	return (Statement){.type = STAT_EXPR, .expression.expression = expression};
}

CallExpression
ParseCallExpression(Parser *parser)
{
	CallExpression expression = {.procedure = parser->current};

	Expression **arguments = NULL;
	ExpectToken(parser, TOK_L_PAREN);

	if (parser->peek->type != TOK_R_PAREN) {
		arrpush(arguments, ParseExpression(parser, PREC_MIN));
		while (parser->peek->type == TOK_COMMA) {
			ReadToken(parser);
			arrpush(arguments, ParseExpression(parser, PREC_MIN));
		}
	}
	
	expression.arity = arrlen(arguments);
	if (expression.arity > 0) {
		expression.arguments = ArenaAlloc(parser->arena, expression.arity * sizeof(Expression *));
		memcpy(expression.arguments, arguments, expression.arity * sizeof(Expression *));
	}
	arrfree(arguments);

	ExpectToken(parser, TOK_R_PAREN);

	return expression;
}

Expression *
ParseExpression(Parser *parser, Precedence precedence)
{
	ReadToken(parser);

	Expression *left;

	if (parser->current->type == TOK_L_PAREN) {
		left = ParseExpression(parser, PREC_MIN);
		ExpectToken(parser, TOK_R_PAREN);
	} else left = ArenaAlloc(parser->arena, sizeof(Expression));

	switch (parser->current->type) {
	case TOK_MINUS:
	case TOK_NOT:
		left->type = EXPR_PREFIX;
		left->prefix =
			(PrefixExpression){.operator= parser->current,
		                       .value = ParseExpression(parser, PREC_PREFIX)};
		break;
	case TOK_IDENTIFIER:
		if (parser->peek->type == TOK_L_PAREN) {
			left->type = EXPR_CALL;
			left->call = ParseCallExpression(parser);
		} else {
			left->type       = EXPR_IDENTIFIER;
			left->identifier = (IdentifierExpression){.value = parser->current};
		}
		break;
	case TOK_STRING:
	case TOK_INTEGER:
		left->type    = EXPR_LITERAL;
		left->literal = (LiteralExpression){.value = parser->current};
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
	while (parser->peek->type != TOK_SEMICOLON) {
		if (precedence >= TokenPrecedence[parser->peek->type]) break;
		ReadToken(parser);
		infix.value1 = left;
		infix.operator= parser->current;
		infix.value2 =
			ParseExpression(parser, TokenPrecedence[parser->current->type]);
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

	ExpectToken(parser, TOK_LET);

	ExpectToken(parser, TOK_IDENTIFIER);
	statement.identifier = parser->current;

	ExpectToken(parser, TOK_ASSIGN);

	Expression *value = ParseExpression(parser, PREC_MIN);
	if (value->type) statement.value = value;
	else return (Statement){0};

	ExpectToken(parser, TOK_SEMICOLON);

	return (Statement){.type = STAT_LET, .let = statement};
}

Statement
ParseReturnStatement(Parser *parser)
{
	ReturnStatement statement = {.start = parser->current};

	ExpectToken(parser, TOK_RETURN);

	Expression *value = ParseExpression(parser, PREC_MIN);
	if (value->type) statement.value = value;
	else return (Statement){0};

	ExpectToken(parser, TOK_SEMICOLON);

	return (Statement){.type = STAT_RETURN, .return_ = statement};
}

void
PrintExpression(Expression *expression)
{
	switch (expression->type) {
	case EXPR_CALL:
		Print("CALL_EXPRESSION:");
		BeginIndent();

		Print("Procedure: %s", TokenString(expression->call.procedure));
		Print("Arguments:");
		BeginIndent();
		int i;
		for (i = 0; i < expression->call.arity; i++) {
			PrintExpression(expression->call.arguments[i]);
		}
		EndIndent();

		EndIndent();
		break;
	case EXPR_IDENTIFIER:
		Print("IDENTIFIER_EXPRESSION:");
		BeginIndent();

		Print("Value: %s", TokenString(expression->identifier.value));

		EndIndent();
		break;
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
	case STAT_PROC:
		Print("PROC_STATEMENT:");
		BeginIndent();

		Print("Identifier: %s", TokenString(statement->proc.identifier));

		Print("Arity: %d", statement->proc.arity);

		Print("Arguments:");
		BeginIndent();
		int i;
		for (i = 0; i < statement->proc.arity; i++) {
			Print(TokenString(statement->proc.arguments[i]));
		}
		EndIndent();

		Print("Body:");
		BeginIndent();
		for (i = 0; i < statement->proc.body->count; i++) {
			PrintStatement(&statement->proc.body->statements[i]);
		}
		EndIndent();

		EndIndent();
		break;
	case STAT_LET:
		Print("LET_STATEMENT:");
		BeginIndent();

		Print("Identifier: %s", TokenString(statement->let.identifier));

		Print("Value:");
		BeginIndent();
		PrintExpression(statement->let.value);
		EndIndent();

		EndIndent();
		break;
	case STAT_RETURN:
		Print("RETURN_STATEMENT:");
		BeginIndent();

		Print("Value:");
		BeginIndent();
		PrintExpression(statement->let.value);
		EndIndent();

		EndIndent();
		break;
	case STAT_EXPR:
		Print("EXPRESSION_STATEMENT:");
		BeginIndent();

		Print("Expression:");
		BeginIndent();
		PrintExpression(statement->expression.expression);
		EndIndent();

		EndIndent();
		break;
	default: break;
	}
}
