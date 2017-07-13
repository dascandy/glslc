%{

#include <iostream>
#include "parser.hpp"
#include <string>
using namespace std;
#define YY_DECL extern "C" int yylex()
#define YY_USER_ACTION { yytokenoffs += yyleng; }
extern std::string yyfilename;
size_t yytokenoffs;
#define TOKEN yylval.s.v = yytext; yylval.s.def = DefLocation {yyfilename, yytokenoffs, (size_t)yylineno }
%}
%option yylineno

%%

\n                          { yytokenoffs = 0; }
[ \t]          ;
\/\/.*                      ;
\/\*[\n.]*\*\/              ;
out                         { TOKEN; return OUT; }
in                          { TOKEN; return IN; }
uniform                     { TOKEN; return UNIFORM; }
inout                       { TOKEN; return INOUT; }
layout                      { TOKEN; return LAYOUT; }
location                    { TOKEN; return LOCATION; }
return                      { TOKEN; return RETURN; }
discard                     { TOKEN; return DISCARD; }
if                          { TOKEN; return IF; }
for                         { TOKEN; return FOR; }
while                       { TOKEN; return WHILE; }
else                        { TOKEN; return ELSE; }
const                       { TOKEN; return CONST; }
#version                    { TOKEN; return VERSION; }
#extension                  { TOKEN; return EXTENSION; }
[a-zA-Z_][a-zA-Z0-9_]*      { TOKEN; return STRING; }
:                           { TOKEN; return COLON; }
;                           { TOKEN; return SEMICOLON; }
,                           { TOKEN; return COMMA; }
\(                          { TOKEN; return BRACKET_OPEN; }
\)                          { TOKEN; return BRACKET_CLOSE; }
\[                          { TOKEN; return SQBRACKET_OPEN; }
\]                          { TOKEN; return SQBRACKET_CLOSE; }
\{                          { TOKEN; return ACC_OPEN; }
\}                          { TOKEN; return ACC_CLOSE; }
\.                          { TOKEN; return DOT; }
-                           { TOKEN; return MINUS; }
\*                          { TOKEN; return TIMES; }
\+                          { TOKEN; return PLUS; }
\+\+                        { TOKEN; return INCR; }
--                          { TOKEN; return DECR; }

[0-9]+\.([eE][+-]?[0-9]+)?(lf|LF|[fF])?        { TOKEN; return NUMFLOAT; }
[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?(lf|LF|[fF])?  { TOKEN; return NUMFLOAT; }
\.[0-9]+([eE][+-]?[0-9]+)?(lf|LF|[fF])?        { TOKEN; return NUMFLOAT; }
[0-9]+[eE][+-]?[0-9]+(lf|LF|[fF])?            { TOKEN; return NUMFLOAT; }

[1-9][0-9]*                 { TOKEN; return NUMDEC; }
0[0-7]*                     { TOKEN; return NUMOCT; }
0[xX][0-9A-Fa-f]+           { TOKEN; return NUMHEX; }

[1-9][0-9]*[uU]             { TOKEN; return NUMDECU; }
0[0-7]*[uU]                 { TOKEN; return NUMOCTU; }
0[xX][0-9A-Fa-f]+[uU]       { TOKEN; return NUMHEXU; }

\/                          { TOKEN; return DIVIDE; }
=                           { TOKEN; return EQUALS; }
\?                          { TOKEN; return QUESTIONMARK; }
\+=                         { TOKEN; return PLUSEQUALS; }
-=                          { TOKEN; return MINUSEQUALS; }
\*=                         { TOKEN; return TIMESEQUALS; }
\/=                         { TOKEN; return DIVIDEEQUALS; }
\<                          { TOKEN; return LESSTHAN; }
\>                          { TOKEN; return GREATHERTHAN; }
\<=                         { TOKEN; return LESSEQUAL; }
\>=                         { TOKEN; return GREATEREQUAL; }
==                          { TOKEN; return DOUBLEEQUAL; } 
\!=                         { TOKEN; return NOTEQUAL; }
\!                          { TOKEN; return EXCLAMATIONMARK; }
&&                          { TOKEN; return BOOL_AND; }
\|\|                        { TOKEN; return BOOL_OR; }

%%


