#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>
#include <string.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20070509

#define YYEMPTY (-1)
#define yyclearin    (yychar = YYEMPTY)
#define yyerrok      (yyerrflag = 0)
#define YYRECOVERING (yyerrflag != 0)

extern int yyparse(void);

static int yygrowstack(void);
#define YYPREFIX "yy"
#line 2 "src/zmap-1.2.1/src/parser.y"
#include <stdio.h>
#include <string.h>
#include "expression.h"
#include "lexer.h"
#include "filter.h"

void yyerror(const char *str)
{
	fprintf(stderr,"Parse error: %s\n",str);
}
 
int yywrap()
{
	return 1;
}

extern node_t *zfilter;

#line 22 "src/zmap-1.2.1/src/parser.y"
typedef union {
	int int_literal;
	char *string_literal;
	struct node *expr; 
} YYSTYPE;
#line 48 "src/zmap-1.2.1/src/parser.c"
#define T_AND 257
#define T_OR 258
#define T_NUMBER 259
#define T_FIELD 260
#define T_NOT_EQ 261
#define T_GT_EQ 262
#define T_LT_EQ 263
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    4,    4,    4,    4,    1,    1,    2,    2,    2,
    2,    2,    2,    3,    3,
};
short yylen[] = {                                         2,
    1,    3,    3,    3,    1,    1,    1,    3,    3,    3,
    3,    3,    3,    3,    3,
};
short yydefred[] = {                                      0,
    0,    0,    0,    5,    6,    7,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    4,   11,   15,   12,
    9,   10,    8,   14,   13,    3,    0,
};
short yydgoto[] = {                                       3,
    4,    5,    6,    7,
};
short yysindex[] = {                                    -40,
  -40,  -57,    0,    0,    0,    0, -250,  -39, -249, -245,
 -244, -243, -247, -242,  -40,  -40,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -248,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,   18,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    1,
};
short yygindex[] = {                                      0,
    0,    0,    0,    5,
};
#define YYTABLESIZE 259
short yytable[] = {                                       1,
    2,   17,   12,   13,   11,    8,   15,   16,   15,   18,
   19,   23,   24,   20,   21,   22,   25,    1,    0,   26,
   27,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    9,   10,   14,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   15,   16,    2,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    2,
};
short yycheck[] = {                                      40,
    0,   41,   60,   61,   62,    1,  257,  258,  257,  259,
  260,  259,  260,  259,  259,  259,  259,    0,   -1,   15,
   16,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   41,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  261,  262,  263,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  257,  258,  260,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  258,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 263
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'<'","'='","'>'",0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"T_AND",
"T_OR","T_NUMBER","T_FIELD","T_NOT_EQ","T_GT_EQ","T_LT_EQ",
};
char *yyrule[] = {
"$accept : expression",
"expression : filter_expr",
"filter_expr : filter_expr T_OR filter_expr",
"filter_expr : filter_expr T_AND filter_expr",
"filter_expr : '(' filter_expr ')'",
"filter_expr : filter",
"filter : number_filter",
"filter : string_filter",
"number_filter : T_FIELD '=' T_NUMBER",
"number_filter : T_FIELD '>' T_NUMBER",
"number_filter : T_FIELD '<' T_NUMBER",
"number_filter : T_FIELD T_NOT_EQ T_NUMBER",
"number_filter : T_FIELD T_GT_EQ T_NUMBER",
"number_filter : T_FIELD T_LT_EQ T_NUMBER",
"string_filter : T_FIELD '=' T_FIELD",
"string_filter : T_FIELD T_NOT_EQ T_FIELD",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 500

int      yydebug;
int      yynerrs;
int      yyerrflag;
int      yychar;
short   *yyssp;
YYSTYPE *yyvsp;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* variables for the parser stack */
static short   *yyss;
static short   *yysslim;
static YYSTYPE *yyvs;
static int      yystacksize;
#line 143 "src/zmap-1.2.1/src/parser.y"


#line 214 "src/zmap-1.2.1/src/parser.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = yyssp - yyss;
    newss = (yyss != 0)
          ? (short *)realloc(yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    yyss  = newss;
    yyssp = newss + i;
    newvs = (yyvs != 0)
          ? (YYSTYPE *)realloc(yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse(void)
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

#ifdef lint
    goto yyerrlab;
#endif

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yyvsp[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 45 "src/zmap-1.2.1/src/parser.y"
{
		zfilter = yyvsp[0].expr;
	}
break;
case 2:
#line 52 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(OR);
			yyval.expr->left_child = yyvsp[-2].expr;
			yyval.expr->right_child = yyvsp[0].expr;
		}
break;
case 3:
#line 58 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(AND);
			yyval.expr->left_child = yyvsp[-2].expr;
			yyval.expr->right_child = yyvsp[0].expr;
		}
break;
case 4:
#line 64 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = yyvsp[-1].expr;
		}
break;
case 5:
#line 68 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = yyvsp[0].expr;
		}
break;
case 6:
#line 74 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = yyvsp[0].expr;
		}
break;
case 7:
#line 78 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = yyvsp[0].expr;
		}
break;
case 8:
#line 84 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(EQ);
			yyval.expr->left_child = make_field_node(yyvsp[-2].string_literal);
			yyval.expr->right_child = make_int_node(yyvsp[0].int_literal);
		}
break;
case 9:
#line 91 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(GT);
			yyval.expr->left_child = make_field_node(yyvsp[-2].string_literal);
			yyval.expr->right_child = make_int_node(yyvsp[0].int_literal);
		}
break;
case 10:
#line 98 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(LT);
			yyval.expr->left_child = make_field_node(yyvsp[-2].string_literal);
			yyval.expr->right_child = make_int_node(yyvsp[0].int_literal);
		}
break;
case 11:
#line 105 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(NEQ);
			yyval.expr->left_child = make_field_node(yyvsp[-2].string_literal);
			yyval.expr->right_child = make_int_node(yyvsp[0].int_literal);
		}
break;
case 12:
#line 112 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(GT_EQ);
			yyval.expr->left_child = make_field_node(yyvsp[-2].string_literal);
			yyval.expr->right_child = make_int_node(yyvsp[0].int_literal);
		}
break;
case 13:
#line 119 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(LT_EQ);
			yyval.expr->left_child = make_field_node(yyvsp[-2].string_literal);
			yyval.expr->right_child = make_int_node(yyvsp[0].int_literal);
		}
break;
case 14:
#line 128 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(EQ);
			yyval.expr->left_child = make_field_node(yyvsp[-2].string_literal);
			yyval.expr->right_child = make_string_node(yyvsp[0].string_literal);
		}
break;
case 15:
#line 135 "src/zmap-1.2.1/src/parser.y"
{
			yyval.expr = make_op_node(NEQ);
			yyval.expr->left_child = make_field_node(yyvsp[-2].string_literal);
			yyval.expr->right_child = make_string_node(yyvsp[0].string_literal);
		}
break;
#line 505 "src/zmap-1.2.1/src/parser.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    return (1);

yyaccept:
    return (0);
}
