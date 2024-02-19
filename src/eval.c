#include <stdio.h>
#include <stdlib.h>

#include "eval.h"
#include "stb_ds.h"

ValueItem **stack;

Evaluator *
CreateEvaluator(Statement *program)
{
	MemoryBlock *arena = CreateArena();

	Evaluator *eval = ArenaAlloc(arena, sizeof(Evaluator));
	*eval = (Evaluator){.arena = arena, .program = program, .stack = NULL};

	arrpush(eval->stack, NULL);
	shdefault(eval->stack[0], (Value){.type = VAL_NONE});

	return eval;
}

void
DestroyEvaluator(Evaluator *eval)
{
	shfree(eval->stack[0]);
	arrfree(eval->stack);
	DestroyArena(eval->arena);
}

void
Eval(Evaluator *eval, Statement *statements)
{
	int top = arrlen(eval->stack) - 1;

	puts("\n\n==== EVAL ====");
	int i;
	for (i = 0; statements[i].type != STAT_INVALID; i++) {
		// PrintStatement(&statements[i]);
		switch (statements[i].type) {
		case STAT_LET: {
			LetStatement statement  = statements[i].let;
			char        *identifier = statement.identifier->value;
			Value        value      = EvalExpression(eval, statement.value);
			shput(eval->stack[top], identifier, value);
		} break;
		case STAT_PROC: {
			ProcStatement *statement = &statements[i].proc;
			Value          value = {.type = VAL_PROC, .procedure = statement};
			shput(eval->stack[top], statement->identifier->value, value);
		} break;
		case STAT_RETURN: {
			Value value = EvalExpression(eval, statements->return_.value);
			shput(eval->stack[top], "_return_val", value);
			return;
		}
		case STAT_EXPR:
			EvalExpression(eval, statements[i].expression.expression);
			break;
		default: break;
		}
	}

	if (top == 0) printf("z: %d\n", GetValue(eval, "z").integer);
}

Value
GetValue(Evaluator *eval, char *identifier)
{
	Value value;

	int frame;
	for (frame = arrlen(eval->stack) - 1; frame >= 0; frame--) {
		value = shget(eval->stack[frame], identifier);
		if (value.type != VAL_NONE) break;
	}

	if (value.type == VAL_NONE) {
		fprintf(stderr, "Undeclared identifier: %s\n", identifier);
		exit(300);
	}

	return value;
}

Value
EvalExpression(Evaluator *eval, Expression *expression)
{
	Value value;

	switch (expression->type) {
	case EXPR_IDENTIFIER:
		value = GetValue(eval, expression->identifier.value->value);
		break;
	case EXPR_LITERAL: {
		Token *token = expression->literal.value;
		if (token->type == TOK_INTEGER) {
			value.type    = VAL_INTEGER;
			value.integer = atoi(token->value);
		} else if (token->type == TOK_STRING) {
			value.type   = VAL_STRING;
			value.string = token->value;
		} else exit(300);
	} break;
	case EXPR_INFIX: {
		InfixExpression infix = expression->infix;

		value.type = VAL_INTEGER;
		int value1 = EvalExpression(eval, infix.value1).integer;
		int value2 = EvalExpression(eval, infix.value2).integer;

		if (infix.operator->type == TOK_PLUS) {
			value.integer = value1 + value2;
		} else if (infix.operator->type == TOK_MINUS) {
			value.integer = value1 - value2;
		} else if (infix.operator->type == TOK_STAR) {
			value.integer = value1 * value2;
		} else if (infix.operator->type == TOK_SLASH) {
			value.integer = value1 / value2;
		}
	} break;
	case EXPR_CALL: {
		CallExpression call       = expression->call;
		char          *identifier = call.procedure->value;
		Value          proc       = GetValue(eval, identifier);

		if (call.arity != proc.procedure->arity) {
			fprintf(stderr, "Artity mismatch\n");
			exit(300);
		}

		int top = arrlen(eval->stack);
		arrpush(eval->stack, NULL);
		shdefault(eval->stack[top], (Value){.type = VAL_NONE});

		int i;
		for (i = 0; i < call.arity; i++) {
			char *argument = proc.procedure->arguments[i]->value;
			Value value    = EvalExpression(eval, call.arguments[i]);
			printf("%s = %d\n", argument, value.integer);
			shput(eval->stack[top], argument, value);
		}

		Eval(eval, proc.procedure->body->statements);
		value = shget(eval->stack[top], "_return_val");

		shfree(eval->stack[top]);
		arrpop(eval->stack);
	} break;
	default: exit(300);
	}

	return value;
}
