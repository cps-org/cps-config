// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

%skeleton "lalr1.cc"

%define api.token.raw

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include "cps/utils.hpp"
    #include "cps/pc_compat/pc_base.hpp"
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
    COLON       ":"
    LF          "\n"
    EQ          "="
    LT          "<"
    LE          "<="
    NE          "!="
    GT          ">"
    GE          ">="
    DOLLAR      "$"
    LBRACE      "{"
    RBRACE      "}"
    COMMA       ","
    REQUIRES    "Requires"
    REQUIRES_P  "Requires.private"
    CONFLICTS   "Conflicts"
    PROVIDES    "Provides"
;

%token <std::string> STR "str"
%token <std::string> BLANK "blank"
%nterm <std::string> literal
%nterm <std::string> variable
%nterm <std::string> name
%nterm <cps::pc_compat::VersionOperation> version_op_token
%nterm <cps::pc_compat::VersionOperation> version_op
%nterm <cps::pc_compat::PackageRequirement> package_requirement
%nterm <std::vector<cps::pc_compat::PackageRequirement>> package_requirements
%nterm <std::variant<std::string, std::vector<cps::pc_compat::PackageRequirement>>> literal_property

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

// In this grammar, leading whitespace is handled by this rule.
// Everything else only takes care of trailing whitespace.
line:
    "\n"
  | statement "\n"
  | "blank" statement "\n";

// Statement is a meaningful line, not including line feed
statement:
    property
  | assignment;

// property is a line that sets a property to a value
property:
    "Requires" colon package_requirements { loader.properties.emplace("Requires", $3); }
  | "Requires.private" colon package_requirements { loader.properties.emplace("Requires.private", $3); }
  | "Conflicts" colon package_requirements { loader.properties.emplace("Conflicts", $3); }
  | "Provides" colon package_requirements { loader.properties.emplace("Provides", $3); }
  | name colon literal_property { loader.properties.emplace($1, $3); }

// version_op_token captures all the valid tokens for version comparison
version_op_token:
    "<" { $$ = cps::pc_compat::VersionOperation::lt; }
  | "<=" { $$ = cps::pc_compat::VersionOperation::le; }
  | "=" { $$ = cps::pc_compat::VersionOperation::eq; }
  | "!=" { $$ = cps::pc_compat::VersionOperation::ne; }
  | ">" { $$ = cps::pc_compat::VersionOperation::gt; }
  | ">=" { $$ = cps::pc_compat::VersionOperation::ge; };

// version_op handles trailing space for version comparisons
version_op:
    version_op_token
  | version_op_token "blank" { $$ = $1; };

// package_requirement parses a package name, optionally followed by some version requirement
package_requirement:
    name version_op "str" {
        $$ = cps::pc_compat::PackageRequirement {
            .package = $1,
            .operation = $2,
            .version = $3,
        };
    }
  | package_requirement "blank" { $$ = $1; };

// package_requirements is a comma separated list of package_requirement
package_requirements:
    package_requirement { $$ = std::vector{$1}; }
  | package_requirements comma package_requirement {
        $1.emplace_back($3);
        $$ = $1;
    }
  | package_requirements comma "str" {
        $1.emplace_back(cps::pc_compat::PackageRequirement {
            .package = $3,
            .operation = std::nullopt,
            .version = std::nullopt,
        });
        $$ = $1;
    };

// assignment is a line that sets a variable to a value
assignment:
    name "=" literal { loader.variables.emplace($1, cps::utils::trim($3)); };

// name handles surrounding spaces for a variable or property name
name:
    "str"
  | "str" "blank" { $$ = $1; };

// literal_property constructs a variant with the trimmed literal value
literal_property:
    literal { $$ = cps::pc_compat::PcPropertyValue{std::in_place_type<std::string>, cps::utils::trim($1)}; };

// Literal is a literal string. This could contain trailing whitespace so trim the result before using.
literal:
    ":" { $$ = ":"; }
  | "str" { $$ = $1; }
  | variable { $$ = $1; }
  | literal ":" { $$ = $1 + ":"; }
  | literal "str" { $$ = $1 + $2; }
  | literal variable { $$ = $1 + $2; }
  | literal "blank" { $$ = $1 + $2; };

variable:
    "$" "{" "str" "}" { $$ = loader.variables[$3]; }

// colon and comma handles trailing whitespace
colon:
    ":"
  | ":" "blank";
comma:
    ","
  | "," "blank";
%%

void
yy::parser::error (const std::string& m)
{
    std::cerr << "Error: " << m << '\n';
}
