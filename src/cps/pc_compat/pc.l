/*
 * SPDX-License-Identifier: MIT
 * Copyright © 2024 Haowen Liu
 */

%{
#include <string>
#include <istream>
#include "cps/pc_compat/pc_loader.hpp"
#include "cps/pc_compat/pc.parser.hpp"
%}

/* To debug scanner, set debug as an option */
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

/* "Name"              { return yy::parser::make_NAME(); }
"Version"           { return yy::parser::make_VERSION(); }
"Description"       { return yy::parser::make_DESCRIPTION(); }
"URL"               { return yy::parser::make_URL(); }
"Cflags"            { return yy::parser::make_CFLAGS(); }
"Cflags.private"    { return yy::parser::make_CFLAGS_P(); }
"Libs"              { return yy::parser::make_LIBS(); }
"Libs.private"      { return yy::parser::make_LIBS_P(); } */

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

