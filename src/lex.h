#ifndef lex_h
#define lex_h

/* clang-format off */
typedef enum {
	TOK_EOF,
	/* DELIMITERS */
	/* Pairs */
	TOK_L_PAREN, TOK_R_PAREN,
	TOK_L_BRACE, TOK_R_BRACE,
	/* Other */
	TOK_COMMA, TOK_SEMICOLON,
	/* OPERATORS */
	/* Assignment */
	TOK_ASSIGN,
	/* Arithmetic */
	TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_MOD,
	/* Relational */
	TOK_EQUAL, TOK_UNEQUAL,
	TOK_LESSER, TOK_GREATER,
	TOK_LESSER_EQ, TOK_GREATER_EQ,
	/* Logical */
	TOK_NOT, TOK_AND, TOK_OR,
	/* KEYWORDS */
	/* Control Flow */
	TOK_IF, TOK_ELSE,
	TOK_FOR, TOK_BREAK, TOK_CONTINUE,
	TOK_RETURN,
	/* Definitions */
	TOK_PROC, TOK_LET,
	/* IDENTIFIERS */
	TOK_IDENTIFIER,
	/* LITERALS */
	TOK_STRING, TOK_NUMBER,
} TokenType;
/* clang-format on */

typedef struct {
	TokenType type;
	char     *value;
} Token;

typedef struct {
	char     *key;
	TokenType value;
} Symbol;

typedef struct {
	char   *input;
	Token  *output;
	Symbol *symbols;
	char    current;
	char    peek;
	int     position;
	int     row;
	int     column;
} Lexer;

Lexer *CreateLexer(char *);
void   DestroyLexer(Lexer *);
Token *Lex(Lexer *);
Token  NextToken(Lexer *);
void   ReadChar(Lexer *);

Token CreateToken(TokenType, char *);
void  DestroyToken(Token);
void  PrintToken(Token);

#endif /* !lex_h */
