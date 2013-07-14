/* ebnf2yacc
 * -- A kleene closure preprocessor for yacc
 * Copyright (C) 2011 Jerry Chen <mailto:onlyuser@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

%option outfile="lex.ebnf2yacc.c"
%option prefix="_EBNF2YACC_"

%{

#include "ebnf2yacc.h"
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "ebnf2yacc.tab.h" // ID_XXX (yacc generated)
#include "XLangTreeContext.h" // TreeContext
#include "XLangString.h" // xl::unescape
#include <stdlib.h> // atoi
#include <string> // std::string
#include <sstream> // std::stringstream

std::stringstream &buf()
{
    static std::stringstream _buf;
    return _buf;
}
void reset_buf()
{
    std::stringstream &ss = buf();
    ss.str("");
    ss.clear();
}

#define PUSH_STATE   yy_push_state
#define POP_STATE    yy_pop_state()
#define LVAL         _XLANG_lval
#define TREE_CONTEXT (*tree_context())

// does nothing, returns void like yy_pop_state
#define NO_OP [](){}

// temporary workaround for flex bug: yy_top_state crashes on INITIAL state - ID: 3462982
// http://sourceforge.net/tracker/?func=detail&aid=3462982&group_id=97492&atid=618177
#define TOP_STATE (                                     \
        PUSH_STATE(INITIAL),                            \
        ((yy_start_stack_ptr>1) ? POP_STATE : NO_OP()), \
        yy_top_state())

%}

%option noyywrap nounput
%option stack
%x ST_COMMENT_BLOCK ST_COMMENT_LINE ST_STRING ST_CHAR
%x ST_UNION_BLOCK ST_PROTO_BLOCK ST_ACTION_BLOCK ST_CODE
%s ST_RULES

ws               [ \t\n]+
any              .|[\n]
lit_char_def     [<>=]
lit_char_rules   [:|()+*?;]
lit_char_percent [%][^%{}]
digit            [0-9]
alpha            [a-zA-Z]
id               {alpha}|[_]
lit_ident        {id}+({digit}|{id})*
lit_ident_def    {id}+({digit}|{id}|[-])*
lit_ident_c      {id}+({digit}|{id}|[:<>,*])*
lit_int          {digit}+
lit_float        {digit}*[.]{digit}+

%%

{ws}        ;

 /*==========================================================================*/
 /* COMMENTS */

"/*"        {
                PUSH_STATE(ST_COMMENT_BLOCK);
            }
<ST_COMMENT_BLOCK>"*/" {
                POP_STATE;
            }
<ST_COMMENT_BLOCK>{any} ;

"//"        {
                PUSH_STATE(ST_COMMENT_LINE);
            }
<ST_COMMENT_LINE>"\n" {
                POP_STATE;
            }
<ST_COMMENT_LINE>. ;

 /*==========================================================================*/
 /* QUOTES */

[\"]        {
                PUSH_STATE(ST_STRING);
            }
<ST_STRING>[^\"]* {
                LVAL.string_value = TREE_CONTEXT.alloc_string(xl::unescape(yytext));
                return ID_STRING;
            }
<ST_STRING>[\"] {
                POP_STATE;
            }

[\']        {
                PUSH_STATE(ST_CHAR);
            }
<ST_CHAR>[^\']* {
                LVAL.char_value = xl::unescape(yytext)[0];
                return ID_CHAR;
            }
<ST_CHAR>[\'] {
                POP_STATE;
            }

 /*==========================================================================*/
 /* DEFINITIONS SECTION */

<INITIAL>[\{] {
                PUSH_STATE(ST_UNION_BLOCK);
                return *yytext;
            }
<ST_UNION_BLOCK>{lit_ident_c} {
                LVAL.string_value = TREE_CONTEXT.alloc_string(yytext);
                return ID_STRING;
            }
<ST_UNION_BLOCK>[;] {
                return *yytext;
            }
<ST_UNION_BLOCK>"/*" {
                PUSH_STATE(ST_COMMENT_BLOCK);
            }
<ST_UNION_BLOCK>"//" {
                PUSH_STATE(ST_COMMENT_LINE);
            }
<ST_UNION_BLOCK>[\}] {
                POP_STATE;
                return *yytext;
            }
<ST_UNION_BLOCK>{any} ;

<INITIAL>"%{" {
                if(TOP_STATE == INITIAL)
                    reset_buf();
                PUSH_STATE(ST_PROTO_BLOCK);
            }
<ST_PROTO_BLOCK>"%}" {
                POP_STATE;
                if(TOP_STATE == INITIAL)
                {
                    LVAL.string_value = TREE_CONTEXT.alloc_string(buf().str());
                    return ID_STRING;
                }
            }
<ST_PROTO_BLOCK>{any} {
                buf() << yytext;
            }

<INITIAL>{lit_char_def} {
                return *yytext;
            }
<INITIAL>{lit_char_percent} {
                yyless(1);
                return *yytext;
            }
<INITIAL>{lit_ident_def} {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_IDENT;
            }
<INITIAL>"%%" {
                BEGIN ST_RULES;
                return ID_FENCE;
            }
<INITIAL>{any} ;

 /*==========================================================================*/
 /* RULES SECTION */

<ST_RULES>[\{] {
                if(TOP_STATE == INITIAL)
                    reset_buf();
                PUSH_STATE(ST_ACTION_BLOCK);
            }
<ST_ACTION_BLOCK>[\{] {
                PUSH_STATE(ST_ACTION_BLOCK);
                REJECT; // fall-through to {any} handler
            }
<ST_ACTION_BLOCK>[\}] {
                POP_STATE;
                if(TOP_STATE == INITIAL)
                {
                    LVAL.string_value = TREE_CONTEXT.alloc_string(buf().str());
                    return ID_STRING;
                }
                else
                    REJECT; // fall-through to {any} handler
            }
<ST_ACTION_BLOCK>{any} {
                buf() << yytext;
            }

<ST_RULES>"%%" {
                reset_buf();
                BEGIN ST_CODE;
                return ID_FENCE;
            }

 /*==========================================================================*/
 /* CODE SECTION */

<ST_CODE>{any} {
                buf() << yytext;
            }
<ST_CODE><<EOF>> {
                LVAL.string_value = TREE_CONTEXT.alloc_string(buf().str());
                return ID_STRING;
            }

 /*==========================================================================*/
 /* LITERALS */

<ST_RULES>{lit_ident}|"%prec" {
                LVAL.ident_value = TREE_CONTEXT.alloc_unique_string(yytext);
                return ID_IDENT;
            }

<ST_RULES>{lit_int} {
                LVAL.int_value = atoi(yytext);
                return ID_INT;
            }

<ST_RULES>{lit_float} {
                LVAL.float_value = atof(yytext);
                return ID_FLOAT;
            }

<ST_RULES>{lit_char_rules} {
                return *yytext;
            }

<ST_RULES>{any} {
                char buf[40];
                sprintf(buf, "unknown character: \'%c\'\n", *yytext);
                _XLANG_error(buf);
                YY_FLUSH_BUFFER;
                BEGIN INITIAL;
            }

 /*==========================================================================*/
 /* OTHER */

.           ;

%%