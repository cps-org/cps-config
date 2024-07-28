// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

%skeleton "lalr1.cc"

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include <string>
    #include <istream>
    #include "cps/utils.hpp"
    namespace cps::pc_compat {
        class PcLoader;
    }
}

// The parsing context.
%param { cps::pc_compat::PcLoader& loader }

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
#include "cps/pc_compat/pc_loader.hpp"
}

%define api.token.prefix {TOK_}
%token
    COLON   ":"
    LF      "\n"
    EQ      "="
    DOLLAR  "$"
    LBRACE  "{"
    RBRACE  "}"
;

%token <std::string> STR "str"
%token <std::string> BLANK "blank"
%nterm <std::string> literal
%nterm <std::string> variable
%nterm <std::string> literal_value
%nterm <std::string> name

%printer { yyo << $$; } <*>;

%%
%start file;

// This rule allows for the last line to not be terminated by '\n'
file:
    lines
  | lines statement;

lines:
    %empty
  | lines line;

line:
    "\n"
  | statement "\n";

// Statement is a meaningful line, not including line feed
statement:
    property
  | assignment;

// property is a line that sets a property to a value
property:
    name ":" literal_value { loader.properties.emplace($1, cps::utils::trim($3)); };

// assignment is a line that sets a variable to a value
assignment:
    name "=" literal_value { loader.variables.emplace($1, cps::utils::trim($3)); };

// name handles surrounding spaces for a variable or property name
name:
    "str"
  | "blank" "str" { $$ = $2; }
  | "str" "blank" { $$ = $1; }
  | "blank" "str" "blank" { $$ = $2; };

// literal_value handles leading spaces.
literal_value:
    literal
  | "blank" literal { $$ = $2; };

// Literal is a literal string. This could contain trailing whitespace so trim the result before using.
literal:
    ":" { $$ = ":"; }
  | "str" { $$ = $1; }
  | variable { $$ = $1; }
  | literal ":" { $$ = $1 + ":"; }
  | literal "str" { $$ = $1 + $2; }
  | literal variable { $$ = $1 + $2; }
  | literal "blank" { $$ = $1 + " "; };

variable:
    "$" "{" "str" "}" { $$ = loader.variables[$3]; }
%%

void
yy::parser::error (const std::string& m)
{
    std::cerr << "Error: " << m << '\n';
}
