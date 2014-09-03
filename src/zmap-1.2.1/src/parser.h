#define T_AND 257
#define T_OR 258
#define T_NUMBER 259
#define T_FIELD 260
#define T_NOT_EQ 261
#define T_GT_EQ 262
#define T_LT_EQ 263
typedef union {
	int int_literal;
	char *string_literal;
	struct node_struct *expr;
} YYSTYPE;
extern YYSTYPE yylval;
