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
":"         { return yy::parser::make_COLON(); }
"="         { return yy::parser::make_EQ(); }
"\n"        { return yy::parser::make_LF(); }
"$"         { return yy::parser::make_DOLLAR(); }
"{"         { return yy::parser::make_LBRACE(); }
"}"         { return yy::parser::make_RBRACE(); }

{comment}   {}
{blank}     { return yy::parser::make_BLANK(yytext); }
{str}       { return yy::parser::make_STR(yytext); }
<<EOF>>     { return yy::parser::make_YYEOF(); }
%%

namespace cps::pc_compat {

    void PcLoader::scan_begin(std::istream &istream) const {
        yyinput = &istream;
    }

}

