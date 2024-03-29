#ifndef parse_h
#define parse_h

#include <stdbool.h>

#include "arena.h"
#include "lex.h"

typedef enum {
	PREC_MIN,
	PREC_COMPARISON,
	PREC_SUM,
	PREC_PRODUCT,
	PREC_PREFIX,
	PREC_CALL
} Precedence;

typedef struct {
	Lexer       *lexer;
	Token       *current;
	Token       *peek;
	MemoryBlock *arena;
} Parser;

typedef struct {
	Token              *procedure;
	char                arity;
	struct Expression **arguments;
} CallExpression;

typedef struct {
	Token *value;
} IdentifierExpression;

typedef struct {
	Token *value;
} LiteralExpression;

typedef struct {
	Token             *operator;
	struct Expression *value;
} PrefixExpression;

typedef struct {
	Token             *operator;
	struct Expression *value1, *value2;
} InfixExpression;

typedef struct Expression {
	enum {
		EXPR_INVALID,
		EXPR_CALL,
		EXPR_IDENTIFIER,
		EXPR_LITERAL,
		EXPR_INFIX,
		EXPR_PREFIX
	} type;
	union {
		CallExpression       call;
		IdentifierExpression identifier;
		LiteralExpression    literal;
		PrefixExpression     prefix;
		InfixExpression      infix;
	};
} Expression;

typedef struct ExpressionStatement {
	Expression *expression;
} ExpressionStatement;

typedef struct BlockStatement {
	Token            *start;
	int               count;
	struct Statement *statements;
} BlockStatement;

typedef struct LetStatement {
	Token      *identifier;
	Expression *value;
} LetStatement;

typedef struct ReturnStatement {
	Token      *start;
	Expression *value;
} ReturnStatement;

typedef struct ProcStatement {
	Token          *identifier;
	char            arity;
	Token         **arguments;
	BlockStatement *body;
} ProcStatement;

typedef struct Statement {
	enum {
		STAT_INVALID,
		STAT_EXPR,
		STAT_BLOCK,
		STAT_LET,
		STAT_RETURN,
		STAT_PROC
	} type;
	union {
		ExpressionStatement expression;
		ProcStatement       proc;
		LetStatement        let;
		ReturnStatement     return_;
		BlockStatement      block;
	};
} Statement;

Parser *CreateParser(Lexer *);
void    DestroyParser(Parser *);

void ReadToken(Parser *);
void ExpectToken(Parser *, TokenType);

void PrintStatement(Statement *);
void PrintExpression(Expression *);

Statement     *Parse(Parser *);
Statement      ParseStatement(Parser *);
Statement      ParseExpressionStatement(Parser *);
Statement      ParseLetStatement(Parser *);
Statement      ParseReturnStatement(Parser *);
Statement      ParseProcStatement(Parser *);
BlockStatement ParseBlockStatement(Parser *);

typedef Expression ExpressionParser(Parser *);

Expression *ParseExpression(Parser *, Precedence);
Expression  ParseLiteralExpression(Parser *);
Expression  ParseStringLiteral(Parser *);
Expression  ParseIntegerLiteral(Parser *);

#endif /* !parse_h */
