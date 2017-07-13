%{
#include <cstdio>
#include <iostream>
using namespace std;
#include "parser.hpp"
#include "Glsl.h"

extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
 
extern "C" void yyerror(const char *s);

Shader *shader = NULL;
Scope *curScope = NULL;
std::string type;
extern bool compileErrors;
%}
%code requires {

#ifndef YYSTYPE
#include "Glsl.h"

struct yystype {
  struct { std::string v; DefLocation def; } s;
  struct { Rvalue *v; DefLocation def; } e;
  struct { std::vector<FuncArg*> *v; DefLocation def; } fal;
  struct { FuncArg *v; DefLocation def; } fa;
  struct { std::vector<Rvalue *> *v; DefLocation def; } el;
};

#define YYSTYPE struct yystype
#endif

}

%token <s> IN OUT UNIFORM INOUT VERSION EXTENSION RETURN IF FOR WHILE ELSE LAYOUT LOCATION DISCARD
%token <s> TIMESEQUALS PLUSEQUALS MINUSEQUALS DIVIDEEQUALS STRING CONST
%token <s> SEMICOLON COLON QUESTIONMARK
%token <s> COMMA BRACKET_OPEN BRACKET_CLOSE SQBRACKET_OPEN SQBRACKET_CLOSE ACC_OPEN ACC_CLOSE DOT
%token <s> MINUS TIMES PLUS NUMFLOAT NUMDEC NUMOCT NUMHEX NUMDECU NUMOCTU NUMHEXU DIVIDE EQUALS INCR DECR
%token <s> LESSTHAN GREATHERTHAN LESSEQUAL GREATEREQUAL DOUBLEEQUAL NOTEQUAL 
%token <s> BOOL_AND BOOL_OR EXCLAMATIONMARK

%type <e> expression13 expression12 expression11 expression10 expression9 expression8 expression7 expression6 expression5 expression4 vardecl num layoutspec statement compound_statement vardecls fordecl 
%type <s> typedecl lvalue usage direction 
%type <fal> funcargspec funcargspec1
%type <fa> funcarg
%type <el> funcargs stmtlist 

// The well-known shift-reduce conflict on if-then-else with non-required braces.
%expect 1

%%
glslfile:
  glslfile preproc_decl
| glslfile declaration
|                               {
  shader = new Shader();
  curScope = shader->globalBB.scope;
}
;
preproc_decl:
  VERSION NUMDEC                                { shader->version = atoi($2.v.c_str()); }
| EXTENSION STRING COLON STRING                 { shader->extensions.insert(std::make_pair($2.v, $4.v)); }
;
declaration:
  LAYOUT layoutspec usage typedecl lvalue SEMICOLON { 
    Scope *s; 
    if ($3.v == "in") s = &shader->in; 
    else if ($3.v == "out") s = &shader->out;
    else if ($3.v == "uniform") s = &shader->uniform;
    s->variables.insert(std::make_pair($5.v, new Variable($5.def, $4.v, $5.v, -1)));
    // atoi($2.c_str())))); 
  }
| usage typedecl lvalue SEMICOLON { 
    Scope *s; 
    if ($1.v == "in") s = &shader->in; 
    else if ($1.v == "out") s = &shader->out;
    else if ($1.v == "uniform") s = &shader->uniform;
    s->variables.insert(std::make_pair($3.v, new Variable($3.def, $2.v, $3.v, -1))); 
  }
| typedecl vardecls SEMICOLON { shader->globalBB.statements.push_back($2.v); type = ""; }
| typedecl STRING BRACKET_OPEN { curScope = new Scope(curScope); } funcargspec BRACKET_CLOSE compound_statement { 
    shader->functions[$2.v].push_back(new Function($2.def, $1.v, $2.v, *$5.v, curScope, $7.v));
    curScope = curScope->parent;
  }
;
usage:
  IN        { $$ = $1; }
| OUT       { $$ = $1; }
| UNIFORM   { $$ = $1; }
;
vardecls:
  vardecls COMMA vardecl { 
    if ($1.v && $3.v) { $$.v = new Expression(Expression::comma, $1.v, $3.v); } else if ($1.v) $$.v = $1.v; else $$.v = $3.v; $$.def = $1.def; }
| vardecl { $$.v = $1.v; $$.def = $1.def; }
;
vardecl:
  lvalue EQUALS expression13 {
    Variable *var = new Variable($1.def, type, $1.v, -1);
    curScope->variables.insert(std::make_pair($1.v, var)); 
    $$.v = new Expression(Expression::assign, new VarRef($1.def, var), $3.v);
    $$.def = $1.def;
  }
| lvalue {
    Variable *var = new Variable($1.def, type, $1.v, -1);
    curScope->variables.insert(std::make_pair($1.v, var)); 
    $$.v = NULL;
    $$.def = $1.def;
  }
;
typedecl:
  CONST STRING                                                { $$.v = type = "const " + $2.v; $$.def = $1.def; }
| STRING                                                      { $$.v = type = $1.v; $$.def = $1.def; }
;
funcargspec:
  funcargspec1                                  { $$.v = $1.v; }
|                                               { $$.v = new std::vector<FuncArg*>(); }
;
funcargspec1:
  funcargspec1 COMMA funcarg                    { 
    $$.v = $1.v;
    curScope->variables.insert(std::make_pair($3.v->name, new Variable($3.v->type, $3.v->name, -1)));
    $$.v->push_back($3.v); 
  }
| funcarg                                       { 
    $$.v = new std::vector<FuncArg*>(); 
    if ($1.v->type.type != Type::Tvoid) {
      curScope->variables.insert(std::make_pair($1.v->name, new Variable($1.v->type, $1.v->name, -1)));
      $$.v->push_back($1.v); 
    }
  }
;
funcarg:
  direction typedecl lvalue EQUALS expression12           { $$.v = new FuncArg($2.def, $1.v, $2.v, $3.v, $5.v); $$.def = $2.def; }
| direction typedecl lvalue                               { $$.v = new FuncArg($2.def, $1.v, $2.v, $3.v, NULL); $$.def = $2.def; }
| direction typedecl                                      { $$.v = new FuncArg($2.def, $1.v, $2.v, "", NULL); $$.def = $2.def; } 
;
direction:
  IN           { $$.v = $1.v; }
| OUT          { $$.v = $1.v; }
| INOUT        { $$.v = $1.v; }
|              { $$.v = "in"; }
;
lvalue:
  STRING                                             { $$.v = $1.v; $$.def = $1.def; }
| STRING SQBRACKET_OPEN num SQBRACKET_CLOSE          { $$.v = $1.v; $$.def = $1.def; // TODO: fix up the array stuff
} 
;
layoutspec:
BRACKET_OPEN LOCATION EQUALS num BRACKET_CLOSE       { $$.v = $4.v; $$.def = $1.def; }
;
stmtlist:
stmtlist statement { 
    if ($2.v)
      $1.v->push_back($2.v);
    $$.v = $1.v;
    $$.def = $1.def;  
  }
|                                             { $$.v = new std::vector<Rvalue*>(); $$.def = DefLocation(); } 
;
compound_statement:
  ACC_OPEN { curScope = new Scope(curScope); } stmtlist ACC_CLOSE { $$.v = new ScopeStmt($1.def, curScope, *$3.v); delete $3.v; $$.def = $1.def; }
;
statement:
  IF BRACKET_OPEN expression13 BRACKET_CLOSE statement ELSE statement {
    $$.v = new IfStatement($1.def, $3.v, $5.v, $7.v);
    $$.def = $1.def;
  }
| IF BRACKET_OPEN expression13 BRACKET_CLOSE statement {
    $$.v = new IfStatement($1.def, $3.v, $5.v, NULL);
    $$.def = $1.def;
  }
| FOR { curScope = new Scope(curScope); } BRACKET_OPEN fordecl SEMICOLON expression13 SEMICOLON expression13 BRACKET_CLOSE statement {
    $$.v = new ForLoop($1.def, curScope, $4.v, $6.v, $8.v, $10.v);
    $$.def = $1.def;
  }
| WHILE BRACKET_OPEN expression13 BRACKET_CLOSE statement {
    $$.v = new WhileLoop($1.def, $3.v, $5.v);
    $$.def = $1.def;
  }
| compound_statement { $$.v = $1.v; $$.def = $1.def; }
| typedecl vardecls SEMICOLON { 
  $$.v = $2.v;
  $$.def = $1.def;
  } 
| expression13 SEMICOLON { $$.v = $1.v; $$.def = $1.def; }
| DISCARD SEMICOLON { $$.v = new Discard($1.def); $$.def = $1.def; } 
;
fordecl:
  typedecl vardecls { $$.v = $2.v; $$.def = $1.def; }
| { $$.v = NULL; $$.def = DefLocation(); }
;
expression13:
  RETURN expression13 { $$.v = new ReturnExpression($2.v); $$.def = $1.def; }
| expression12 EQUALS expression13 { $$.v = new Expression(Expression::assign, $1.v,  $3.v); $$.def = $1.def; }
| expression12 MINUSEQUALS expression13 { $$.v = new Expression(Expression::assign, $1.v, new Expression(Expression::minus, $1.v, $3.v)); $$.def = $1.def; }
| expression12 PLUSEQUALS expression13 { $$.v = new Expression(Expression::assign, $1.v, new Expression(Expression::plus, $1.v, $3.v)); $$.def = $1.def; }
| expression12 TIMESEQUALS expression13 { $$.v = new Expression(Expression::assign, $1.v, new Expression(Expression::times, $1.v, $3.v)); $$.def = $1.def; }
| expression12 DIVIDEEQUALS expression13 { $$.v = new Expression(Expression::assign, $1.v, new Expression(Expression::divide, $1.v, $3.v)); $$.def = $1.def; }
| expression12 { $$.v = $1.v; $$.def = $1.def; }
;
expression12:
  expression11 QUESTIONMARK expression12 COLON expression12 { $$.v = new BooleanChoiceExpression($1.v, $3.v, $5.v); $$.def = $1.def; }
| expression11 { $$.v = $1.v; $$.def = $1.def; }
;
expression11:
  expression10 BOOL_AND expression11 { $$.v = new Expression(Expression::booland, $1.v,  $3.v); $$.def = $1.def; }
| expression10 BOOL_OR expression11 { $$.v = new Expression(Expression::boolor, $1.v,  $3.v); $$.def = $1.def; }
| expression10 { $$.v = $1.v; $$.def = $1.def; }
;
expression10:
  expression9 LESSTHAN expression10 { $$.v = new Expression(Expression::comp_lt, $1.v,  $3.v); $$.def = $1.def; }
| expression9 GREATHERTHAN expression10 { $$.v = new Expression(Expression::comp_gt, $1.v,  $3.v); $$.def = $1.def; }
| expression9 LESSEQUAL expression10 { $$.v = new Expression(Expression::comp_le, $1.v,  $3.v); $$.def = $1.def; }
| expression9 GREATEREQUAL expression10 { $$.v = new Expression(Expression::comp_ge, $1.v,  $3.v); $$.def = $1.def; }
| expression9 DOUBLEEQUAL expression10 { $$.v = new Expression(Expression::comp_eq, $1.v,  $3.v); $$.def = $1.def; }
| expression9 NOTEQUAL expression10 { $$.v = new Expression(Expression::comp_ne, $1.v,  $3.v); $$.def = $1.def; }
| EXCLAMATIONMARK expression10 { $$.v = new Expression(Expression::invert, $2.v); $$.def = $1.def; }
| expression9 { $$.v = $1.v; $$.def = $1.def; }
;
expression9:
  expression8 TIMES expression9 { $$.v = new Expression(Expression::times, $1.v,  $3.v); $$.def = $1.def; }
| expression8 DIVIDE expression9 { $$.v = new Expression(Expression::divide, $1.v,  $3.v); $$.def = $1.def; }
| expression8 { $$.v = $1.v; $$.def = $1.def; }
;
expression8:
  expression7 PLUS expression8 { $$.v = new Expression(Expression::plus, $1.v,  $3.v); $$.def = $1.def; }
| expression7 MINUS expression8 { $$.v = new Expression(Expression::minus, $1.v,  $3.v); $$.def = $1.def; }
| expression7 { $$.v = $1.v; $$.def = $1.def; }
;
expression7:
  MINUS expression6 { $$.v = new Expression(Expression::neg, $2.v); $$.def = $1.def; }
| INCR expression6 { $$.v = new Expression(Expression::preincr, $2.v); $$.def = $1.def; }
| DECR expression6 { $$.v = new Expression(Expression::predecr, $2.v); $$.def = $1.def; }
| expression6 INCR { $$.v = new Expression(Expression::postincr, $1.v); $$.def = $1.def; }
| expression6 DECR { $$.v = new Expression(Expression::postdecr, $1.v); $$.def = $1.def; }
| expression6 { $$.v = $1.v; $$.def = $1.def; }
;
expression6:
  expression5 DOT STRING { $$.v = new ShuffleExpression($1.v, $3.v); $$.def = $1.def; }
| expression5 { $$.v = $1.v; $$.def = $1.def; }
;
expression5:
  expression4 SQBRACKET_OPEN expression9 SQBRACKET_CLOSE { $$.v = new Expression(Expression::arrayidx, $1.v,  $3.v); $$.def = $1.def; }
| expression4 { $$.v = $1.v; $$.def = $1.def; }
;
expression4:
  BRACKET_OPEN expression12 BRACKET_CLOSE { $$.v = $2.v; $$.def = $1.def; }
| STRING BRACKET_OPEN funcargs BRACKET_CLOSE { 
    $$.v = FunctionCall::CreateFromOverload($1.def, $1.v, shader->functions[$1.v], *$3.v);
    $$.def = $1.def;
  }
| STRING {
    if (!curScope->getVariable($1.v)) {
      printf("Cannot find variable %s\n", $1.v.c_str());
      curScope->variables[$1.v] = new Variable(Type::Poison, $1.v, -1);
    }
    $$.v = new VarRef($1.def, curScope->getVariable($1.v));
    $$.def = $1.def;
  }
| num {
    $$.v = $1.v;
    $$.def = $1.def;
  }
;
num:
  NUMDEC    { $$.v = new ImmediateIntExpression($1.def, $1.v); $$.def = $1.def; }
| NUMOCT    { $$.v = new ImmediateIntExpression($1.def, $1.v); $$.def = $1.def; }
| NUMHEX    { $$.v = new ImmediateIntExpression($1.def, $1.v); $$.def = $1.def; }
| NUMDECU   { $$.v = new ImmediateUintExpression($1.def, $1.v); $$.def = $1.def; }
| NUMOCTU   { $$.v = new ImmediateUintExpression($1.def, $1.v); $$.def = $1.def; }
| NUMHEXU   { $$.v = new ImmediateUintExpression($1.def, $1.v); $$.def = $1.def; }
| NUMFLOAT  { $$.v = new ImmediateFloatExpression($1.def, $1.v); $$.def = $1.def; }
;
funcargs:
  funcargs COMMA expression9 { $$.v = $1.v; $$.v->push_back($3.v); $$.def = $1.def; }
| expression9 { $$.v = new std::vector<Rvalue*>(); $$.v->push_back($1.v); $$.def = $1.def; }
| { $$.v = new std::vector<Rvalue*>(); $$.def = DefLocation(); }
%%

