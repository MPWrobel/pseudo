#ifndef eval_h
#define eval_h

#include "parse.h"

typedef struct {
	enum {
		VAL_NONE,
		VAL_PROC,
		VAL_INTEGER,
		VAL_STRING,
	} type;
	union {
		ProcStatement *procedure;
		int            integer;
		char          *string;
	};
} Value;

typedef struct {
	char *key;
	Value value;
} ValueItem;

typedef struct {
	Statement   *program;
	ValueItem  **stack;
	MemoryBlock *arena;
} Evaluator;

Evaluator *CreateEvaluator(Statement *);
void       DestroyEvaluator(Evaluator *);

void  Eval(Evaluator *, Statement *);
Value EvalExpression(Evaluator *, Expression *);
Value GetValue(Evaluator *, char *);

#endif /* !eval_h */
