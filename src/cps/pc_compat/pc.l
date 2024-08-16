/*
 * SPDX-License-Identifier: MIT
 * Copyright © 2024 Haowen Liu
 */

%option noyywrap

%{
#include <string>
#include <istream>
#include "cps/pc_compat/pc_loader.hpp"
#include "cps/pc_compat/pc.parser.hpp"
%}

/* To debug scanner, set debug as an option */
/* TODO: Add a way to debug scanner without rebuilding */
%option noyywrap nounput noinput batch

%{
std::istream *yyinput = nullptr;

#define YY_INPUT(buf,result,max_size) { \
    yyinput->read(buf, max_size); \
    result = yyinput->gcount(); \
}
%}

str     [^ \t\r\n#:=${}]+
blank   [ \t\r]+
comment #[^\n]*

%%
":"                 { return yy::parser::make_COLON(); }
"="                 { return yy::parser::make_EQ(); }
"<"                 { return yy::parser::make_LT(); }
"<="                { return yy::parser::make_LE(); }
"!="                { return yy::parser::make_NE(); }
">="                { return yy::parser::make_GE(); }
">"                 { return yy::parser::make_GT(); }
"\n"                { return yy::parser::make_LF(); }
"$"                 { return yy::parser::make_DOLLAR(); }
"{"                 { return yy::parser::make_LBRACE(); }
"}"                 { return yy::parser::make_RBRACE(); }
","                 { return yy::parser::make_COMMA(); }
"Requires"          { return yy::parser::make_REQUIRES(); }
"Requires.private"  { return yy::parser::make_REQUIRES_P(); }
"Conflicts"         { return yy::parser::make_CONFLICTS(); }
"Provides"          { return yy::parser::make_PROVIDES(); }

{comment}           {}
{blank}             { return yy::parser::make_BLANK(yytext); }
{str}               { return yy::parser::make_STR(yytext); }
<<EOF>>             { return yy::parser::make_YYEOF(); }
%%

namespace cps::pc_compat {

    void PcLoader::scan_begin(std::istream &istream) const {
        yyinput = &istream;
    }

}

