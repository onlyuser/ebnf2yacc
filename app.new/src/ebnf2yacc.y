// ebnf2yacc
// -- A kleene closure preprocessor for yacc
// Copyright (C) 2011 Jerry Chen <mailto:onlyuser@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

//%output="ebnf2yacc.tab.c"
%name-prefix="_EBNF2YACC_"

%{

#include "ebnf2yacc.h"
#include "node/XLangNodeIFace.h" // node::NodeIdentIFace
#include "ebnf2yacc.tab.h" // ID_XXX (yacc generated)
#include "XLangAlloc.h" // Allocator
#include "XLangSystem.h" // system::add_sighandler
#include "mvc/XLangMVCView.h" // mvc::MVCView
#include "mvc/XLangMVCModel.h" // mvc::MVCModel
#include "XLangTreeContext.h" // TreeContext
#include "XLangType.h" // uint32_t
#include "XLangString.h" // xl::read_file
#include "TreeRewriter.h" // ebnf_to_bnf
#include "EBNFPrinter.h" // EBNFPrinter
#include <stdio.h> // size_t
#include <stdarg.h> // va_start
#include <string.h> // strlen
#include <string> // std::string
#include <sstream> // std::stringstream
#include <iostream> // std::cout
#include <stdlib.h> // EXIT_SUCCESS
#include <getopt.h> // getopt_long
#include <assert.h> // assert

#define MAKE_TERM(lexer_id, ...)   xl::mvc::MVCModel::make_term(&pc->tree_context(), lexer_id, ##__VA_ARGS__)
#define MAKE_SYMBOL(...)           xl::mvc::MVCModel::make_symbol(&pc->tree_context(), ##__VA_ARGS__)
#define ERROR_LEXER_ID_NOT_FOUND   "missing lexer id handler, most likely you forgot to register one"
#define ERROR_LEXER_NAME_NOT_FOUND "missing lexer name handler, most likely you forgot to register one"
#define EOL                        xl::node::SymbolNode::eol();

// report error
void _e2y(error)(YYLTYPE* loc, ParserContext* pc, yyscan_t scanner, const char* s)
{
    if(loc)
    {
        std::stringstream ss;
        int line_start = 0;
        int line_end = 0;
        for(int i = pc->scanner_context().m_pos; i >= 0; i--)
        {
            if(pc->scanner_context().m_buf[i] == '\n')
            {
                line_start = i+1;
                break;
            }
        }
        for(int j = pc->scanner_context().m_pos; j < pc->scanner_context().m_length; j++)
        {
            if(pc->scanner_context().m_buf[j] == '\n')
            {
                line_end = j;
                break;
            }
        }
        std::string line(pc->scanner_context().m_buf, line_start, line_end-line_start);
        ss << line << std::endl;
        ss << std::string(loc->first_column-1, '-') <<
                std::string(loc->last_column - loc->first_column + 1, '^') << std::endl <<
                loc->first_line << ":c" << loc->first_column << " to " <<
                loc->last_line << ":c" << loc->last_column << std::endl;
        error_messages() << ss.str();
    }
    error_messages() << s;
}
void _e2y(error)(const char* s)
{
    _e2y(error)(NULL, NULL, NULL, s);
}

// get resource
std::stringstream &error_messages()
{
    static std::stringstream _error_messages;
    return _error_messages;
}
std::string id_to_name(uint32_t lexer_id)
{
    static const char* _id_to_name[] = {
        "int",
        "float",
        "string",
        "char",
        "ident"
        };
    int index = static_cast<int>(lexer_id)-ID_BASE-1;
    if(index >= 0 && index < static_cast<int>(sizeof(_id_to_name)/sizeof(*_id_to_name)))
        return _id_to_name[index];
    switch(lexer_id)
    {
        case ID_GRAMMAR:           return "grammar";
        case ID_DEFINITIONS:       return "definitions";
        case ID_DEFINITION:        return "definition";
        case ID_DEF_EQ:            return "def_eq";
        case ID_DEF_BRACE:         return "def_brace";
        case ID_DEF_PROTO_BLOCK:   return "def_proto_block";
        case ID_UNION_BLOCK:       return "union_block";
        case ID_UNION_MEMBERS:     return "union_members";
        case ID_UNION_MEMBER:      return "union_member";
        case ID_UNION_TERMS:       return "union_terms";
        case ID_UNION_TERM:        return "union_term";
        case ID_DEF_SYMBOLS:       return "def_symbols";
        case ID_DEF_SYMBOL:        return "def_symbol";
        case ID_RULES:             return "rules";
        case ID_RULE:              return "rule";
        case ID_RULE_ALTS:         return "rule_alts";
        case ID_RULE_ALT:          return "rule_alt";
        case ID_RULE_ACTION_BLOCK: return "rule_action_block";
        case ID_RULE_TERMS:        return "rule_terms";
        case ID_CODE:              return "code";
        case '+':                  return "+";
        case '*':                  return "*";
        case '?':                  return "?";
        case '(':                  return "(";
    }
    throw ERROR_LEXER_ID_NOT_FOUND;
    return "";
}
uint32_t name_to_id(std::string name)
{
    if(name == "int")               return ID_INT;
    if(name == "float")             return ID_FLOAT;
    if(name == "string")            return ID_STRING;
    if(name == "char")              return ID_CHAR;
    if(name == "ident")             return ID_IDENT;
    if(name == "grammar")           return ID_GRAMMAR;
    if(name == "definitions")       return ID_DEFINITIONS;
    if(name == "definition")        return ID_DEFINITION;
    if(name == "def_eq")            return ID_DEF_EQ;
    if(name == "def_brace")         return ID_DEF_BRACE;
    if(name == "def_proto_block")   return ID_DEF_PROTO_BLOCK;
    if(name == "union_block")       return ID_UNION_BLOCK;
    if(name == "union_members")     return ID_UNION_MEMBERS;
    if(name == "union_member")      return ID_UNION_MEMBER;
    if(name == "union_terms")       return ID_UNION_TERMS;
    if(name == "union_term")        return ID_UNION_TERM;
    if(name == "def_symbols")       return ID_DEF_SYMBOLS;
    if(name == "def_symbol")        return ID_DEF_SYMBOL;
    if(name == "rules")             return ID_RULES;
    if(name == "rule")              return ID_RULE;
    if(name == "rule_alts")         return ID_RULE_ALTS;
    if(name == "rule_alt")          return ID_RULE_ALT;
    if(name == "rule_action_block") return ID_RULE_ACTION_BLOCK;
    if(name == "rule_terms")        return ID_RULE_TERMS;
    if(name == "code")              return ID_CODE;
    if(name == "+")                 return '+';
    if(name == "*")                 return '*';
    if(name == "?")                 return '?';
    if(name == "(")                 return '(';
    throw ERROR_LEXER_NAME_NOT_FOUND;
    return 0;
}
ParserContext* &parser_context()
{
    static ParserContext* pc = NULL;
    return pc;
}

%}

// 'pure_parser' tells bison to use no global variables and create a
// reentrant parser (NOTE: deprecated, use "%define api.pure" instead).
%define api.pure
%parse-param {ParserContext* pc}
%parse-param {yyscan_t scanner}
%lex-param {scanner}

// show detailed parse errors
%error-verbose

// record where each token occurs in input
%locations

%nonassoc ID_BASE

%token<int_value>    ID_INT
%token<float_value>  ID_FLOAT
%token<string_value> ID_STRING
%token<char_value>   ID_CHAR
%token<ident_value>  ID_IDENT
%type<symbol_value>  grammar definitions definition
        def_proto_block union_block union_members union_member union_terms union_term
        def_symbols def_symbol rules rule rule_alts rule_alt rule_action_block rule_terms rule_term code

%nonassoc ID_GRAMMAR ID_DEFINITIONS ID_DEFINITION ID_DEF_EQ ID_DEF_BRACE
        ID_DEF_PROTO_BLOCK ID_UNION_BLOCK ID_UNION_MEMBERS ID_UNION_MEMBER ID_UNION_TERMS ID_UNION_TERM
        ID_DEF_SYMBOLS ID_DEF_SYMBOL ID_RULES ID_RULE ID_RULE_ALTS ID_RULE_ALT ID_RULE_ACTION_BLOCK ID_RULE_TERMS ID_FENCE ID_CODE
%nonassoc ':'
%nonassoc '|' '(' ';'
%nonassoc '+' '*' '?'

%%

root:
      grammar { pc->tree_context().root() = $1; YYACCEPT; }
    | error   { yyclearin; /* yyerrok; YYABORT; */ }
    ;

grammar:
      definitions ID_FENCE rules ID_FENCE code {
                $$ = MAKE_SYMBOL(ID_GRAMMAR, @$, 3, $1, $3, $5);
            }
    ;

//=============================================================================
// DEFINITIONS SECTION

definitions:
      /* empty */            { $$ = EOL; }
    | definitions definition { $$ = MAKE_SYMBOL(ID_DEFINITIONS, @$, 2, $1, $2); }
    ;

definition:
      '%' ID_IDENT                     { $$ = MAKE_SYMBOL(ID_DEFINITION, @$, 1, MAKE_TERM(ID_IDENT, @$, $2)); }
    | '%' ID_IDENT def_symbols         { $$ = MAKE_SYMBOL(ID_DEFINITION, @$, 2, MAKE_TERM(ID_IDENT, @$, $2), $3); }
    | '%' ID_IDENT '{' union_block '}' { $$ = MAKE_SYMBOL(ID_DEFINITION, @$, 2, MAKE_TERM(ID_IDENT, @$, $2), $4); }
    | '%' ID_IDENT '=' ID_STRING {
                $$ = MAKE_SYMBOL(ID_DEF_EQ, @$, 2,
                        MAKE_TERM(ID_IDENT, @$, $2),
                        MAKE_TERM(ID_STRING, @$, $4));
            }
    | '%' ID_IDENT '<' ID_IDENT '>' def_symbols {
                $$ = MAKE_SYMBOL(ID_DEF_BRACE, @$, 3,
                        MAKE_TERM(ID_IDENT, @$, $2),
                        MAKE_TERM(ID_IDENT, @$, $4),
                        $6);
            }
    | def_proto_block { $$ = $1; }
    ;

def_symbols:
      def_symbol             { $$ = MAKE_SYMBOL(ID_DEF_SYMBOLS, @$, 1, $1); }
    | def_symbols def_symbol { $$ = MAKE_SYMBOL(ID_DEF_SYMBOLS, @$, 2, $1, $2); }
    ;

def_symbol:
      ID_IDENT { $$ = MAKE_SYMBOL(ID_DEF_SYMBOL, @$, 1, MAKE_TERM(ID_IDENT, @$, $1)); }
    | ID_CHAR  { $$ = MAKE_SYMBOL(ID_DEF_SYMBOL, @$, 1, MAKE_TERM(ID_CHAR, @$, $1)); }
    ;

union_block:
      union_members { $$ = MAKE_SYMBOL(ID_UNION_BLOCK, @$, 1, $1); }
    ;

union_members:
      /* empty */                { $$ = EOL; }
    | union_members union_member { $$ = MAKE_SYMBOL(ID_UNION_MEMBERS, @$, 2, $1, $2); }
    ;

union_member:
      union_terms ';' { $$ = MAKE_SYMBOL(ID_UNION_MEMBER, @$, 1, $1); }
    ;

union_terms:
      /* empty */            { $$ = EOL; }
    | union_terms union_term { $$ = MAKE_SYMBOL(ID_UNION_TERMS, @$, 2, $1, $2); }
    ;

union_term:
      ID_STRING {
                $$ = MAKE_SYMBOL(ID_UNION_TERM, @$, 1,
                        MAKE_TERM(ID_STRING, @$, $1));
            }
    ;

def_proto_block:
      ID_STRING {
                $$ = $1->size() ? MAKE_SYMBOL(ID_DEF_PROTO_BLOCK, @$, 1,
                        MAKE_TERM(ID_STRING, @$, $1)) : NULL;
            }
    ;

//=============================================================================
// RULES SECTION

rules:
      /* empty */ { $$ = EOL; }
    | rules rule  { $$ = MAKE_SYMBOL(ID_RULES, @$, 2, $1, $2); }
    ;

rule:
      ID_IDENT ':' rule_alts ';' {
                $$ = MAKE_SYMBOL(ID_RULE, @$, 2, MAKE_TERM(ID_IDENT, @$, $1), $3);
            }
    ;

rule_alts:
      rule_alt               { $$ = MAKE_SYMBOL(ID_RULE_ALTS, @$, 1, $1); }
    | rule_alts '|' rule_alt { $$ = MAKE_SYMBOL(ID_RULE_ALTS, @$, 2, $1, $3); }
    ;

rule_alt:
      rule_terms rule_action_block { $$ = MAKE_SYMBOL(ID_RULE_ALT, @$, 2, $1, $2); }
    ;

rule_action_block:
      /* empty */ { $$ = NULL; }
    | ID_STRING {
                $$ = $1->size() ? MAKE_SYMBOL(ID_RULE_ACTION_BLOCK, @$, 1,
                        MAKE_TERM(ID_STRING, @$, $1)) : NULL;
            }
    ;

rule_terms:
      /* empty */          { $$ = EOL; }
    | rule_terms rule_term { $$ = MAKE_SYMBOL(ID_RULE_TERMS, @$, 2, $1, $2); }
    ;

rule_term:
//      ID_INT            { $$ = MAKE_TERM(ID_INT, @$, $1); } // NOTE: not needed
//    | ID_FLOAT          { $$ = MAKE_TERM(ID_FLOAT, @$, $1); } // NOTE: not needed
//    | ID_STRING         { $$ = MAKE_TERM(ID_STRING, @$, $1); } // NOTE: causes shift-reduce conflict
      ID_CHAR           { $$ = MAKE_TERM(ID_CHAR, @$, $1); }
    | ID_IDENT          { $$ = MAKE_TERM(ID_IDENT, @$, $1); }
    | rule_term '+'     { $$ = MAKE_SYMBOL('+', @$, 1, $1); }
    | rule_term '*'     { $$ = MAKE_SYMBOL('*', @$, 1, $1); }
    | rule_term '?'     { $$ = MAKE_SYMBOL('?', @$, 1, $1); }
    | '(' rule_alts ')' { $$ = MAKE_SYMBOL('(', @$, 1, $2); }
    ;

//=============================================================================
// CODE SECTION

code:
      ID_STRING {
                //std::cerr << $1 << std::endl;
                //throw;
                $$ = $1->size() ? MAKE_SYMBOL(ID_CODE, @$, 1,
                        MAKE_TERM(ID_STRING, @$, $1)) : NULL;
            }
    ;

%%

ScannerContext::ScannerContext(const char* buf)
    : m_scanner(NULL), m_buf(buf), m_pos(0), m_length(strlen(buf)),
      m_line(1), m_column(1), m_prev_column(1)
{}

xl::node::NodeIdentIFace* make_ast(xl::Allocator &alloc, const char* s)
{
    parser_context() = new (PNEW(alloc, , ParserContext)) ParserContext(alloc, s);
    yyscan_t scanner = parser_context()->scanner_context().m_scanner;
    _e2y(lex_init)(&scanner);
    _e2y(set_extra)(parser_context(), scanner);
    int error_code = _e2y(parse)(parser_context(), scanner); // parser entry point
    _e2y(lex_destroy)(scanner);
    return (!error_code && error_messages().str().empty()) ? parser_context()->tree_context().root() : NULL;
}

void display_usage(bool verbose)
{
    std::cout << "Usage: XLang [-i|-f] OPTION [-e] [-m]" << std::endl;
    if(verbose)
    {
        std::cout << "Parses input and prints a syntax tree to standard out" << std::endl
                << std::endl
                << "Input control:" << std::endl
                << "  -i, --in-xml FILENAME (de-serialize from xml)" << std::endl
                << "  -f, --in-file FILENAME" << std::endl
                << std::endl
                << "Output control:" << std::endl
                << "  -y, --yacc" << std::endl
                << "  -e, --expand-ebnf" << std::endl
                << "  -l, --lisp" << std::endl
                << "  -x, --xml" << std::endl
                << "  -g, --graph" << std::endl
                << "  -d, --dot" << std::endl
                << "  -m, --memory" << std::endl
                << "  -h, --help" << std::endl;
    }
    else
        std::cout << "Try `XLang --help\' for more information." << std::endl;
}

struct options_t
{
    typedef enum
    {
        MODE_NONE,
        MODE_YACC,
        MODE_LISP,
        MODE_XML,
        MODE_GRAPH,
        MODE_DOT,
        MODE_HELP
    } mode_e;

    mode_e mode;
    std::string in_file;
    std::string in_xml;
    bool dump_memory;
    bool expand_ebnf;

    options_t()
        : mode(MODE_NONE), dump_memory(false), expand_ebnf(false)
    {}
};

bool extract_options_from_args(options_t* options, int argc, char** argv)
{
    if(!options)
        return false;
    int opt = 0;
    int longIndex = 0;
    static const char *optString = "i:f:yelxgdmh?";
    static const struct option longOpts[] = {
                { "in-xml",      required_argument, NULL, 'i' },
                { "in-file",     required_argument, NULL, 'f' },
                { "yacc",        no_argument,       NULL, 'y' },
                { "expand-ebnf", no_argument,       NULL, 'e' },
                { "lisp",        no_argument,       NULL, 'l' },
                { "xml",         no_argument,       NULL, 'x' },
                { "graph",       no_argument,       NULL, 'g' },
                { "dot",         no_argument,       NULL, 'd' },
                { "memory",      no_argument,       NULL, 'm' },
                { "help",        no_argument,       NULL, 'h' },
                { NULL,          no_argument,       NULL, 0 }
            };
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while(opt != -1)
    {
        switch(opt)
        {
            case 'i': options->in_xml = optarg; break;
            case 'f': options->in_file = optarg; break;
            case 'y': options->mode = options_t::MODE_YACC; break;
            case 'e': options->expand_ebnf = true; break;
            case 'l': options->mode = options_t::MODE_LISP; break;
            case 'x': options->mode = options_t::MODE_XML; break;
            case 'g': options->mode = options_t::MODE_GRAPH; break;
            case 'd': options->mode = options_t::MODE_DOT; break;
            case 'm': options->dump_memory = true; break;
            case 'h':
            case '?': options->mode = options_t::MODE_HELP; break;
            case 0: // reserved
            default:
                break;
        }
        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }
    return options->mode != options_t::MODE_NONE || options->dump_memory;
}

bool import_ast(options_t &options, xl::Allocator &alloc, xl::node::NodeIdentIFace* &ast)
{
    if(options.in_xml != "")
    {
        ast = xl::mvc::MVCModel::make_ast(
                new (PNEW(alloc, xl::, TreeContext)) xl::TreeContext(alloc),
                options.in_xml);
        if(!ast)
        {
            std::cout << "de-serialize from xml fail!" << std::endl;
            return false;
        }
    }
    else
    {
        std::string s;
        if(!xl::read_file(options.in_file, s))
            return false;
        ast = make_ast(alloc, s.c_str());
        if(!ast)
        {
            std::cout << error_messages().str().c_str() << std::endl;
            return false;
        }
    }
    return true;
}

void export_ast(options_t &options, xl::node::NodeIdentIFace* ast)
{
    if(options.expand_ebnf)
    {
        EBNFPrinter v(&parser_context()->tree_context());
        rewrite_tree_until_stable(ast, &v);
    }
    switch(options.mode)
    {
        case options_t::MODE_YACC:
            {
                EBNFPrinter v(&parser_context()->tree_context());
                v.dispatch_visit(ast);
            }
            break;
        case options_t::MODE_LISP:  xl::mvc::MVCView::print_lisp(ast); break;
        case options_t::MODE_XML:   xl::mvc::MVCView::print_xml(ast); break;
        case options_t::MODE_GRAPH: xl::mvc::MVCView::print_graph(ast); break;
        case options_t::MODE_DOT:   xl::mvc::MVCView::print_dot(ast); break;
        default:
            break;
    }
}

bool apply_options(options_t &options)
{
    try
    {
        if(options.mode == options_t::MODE_HELP)
        {
            display_usage(true);
            return true;
        }
        xl::Allocator alloc(__FILE__);
        xl::node::NodeIdentIFace* ast = NULL;
        if(!import_ast(options, alloc, ast))
            return false;
        export_ast(options, ast);
        if(options.dump_memory)
            alloc.dump(std::string(1, '\t'));
    }
    catch(const char* s)
    {
        std::cout << "ERROR: " << s << std::endl;
        return false;
    }
    return true;
}

void add_signal_handlers()
{
    xl::system::add_sighandler(SIGABRT, xl::system::backtrace_sighandler);
    xl::system::add_sighandler(SIGINT,  xl::system::backtrace_sighandler);
    xl::system::add_sighandler(SIGSEGV, xl::system::backtrace_sighandler);
    xl::system::add_sighandler(SIGFPE,  xl::system::backtrace_sighandler);
    xl::system::add_sighandler(SIGBUS,  xl::system::backtrace_sighandler);
    xl::system::add_sighandler(SIGILL,  xl::system::backtrace_sighandler);
}

int main(int argc, char** argv)
{
    add_signal_handlers();
    options_t options;
    if(!extract_options_from_args(&options, argc, argv))
    {
        display_usage(false);
        return EXIT_FAILURE;
    }
    if(!apply_options(options))
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
