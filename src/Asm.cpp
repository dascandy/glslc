#include "Asm.h"
#include "Glsl.h"

void AsmBlock::Add(AsmStmt *stmt) 
{
  stmts.push_back(stmt);
}

ConstLoadExpr::ConstLoadExpr(ConstVal *val)
: val(val) {
}

VarLoadExpr::VarLoadExpr(Variable *var)
: var(var) {
}

AsmOp::AsmOp(const std::string &opcodeName)
: opcodeName(opcodeName)
{
}

AsmOp::AsmOp(const std::string &opcodeName, AsmStmt *arg1)
: opcodeName(opcodeName)
{
  inputs.push_back(arg1);  
}

AsmOp::AsmOp(const std::string &opcodeName, AsmStmt *arg1, AsmStmt *arg2)
: opcodeName(opcodeName)
{
  inputs.push_back(arg1);  
  inputs.push_back(arg2);  
}

AsmCall::AsmCall(AsmBlock *target, std::vector<AsmStmt *> args)
: target(target)
, args(args)
{

}

AsmStore::AsmStore(Variable *var, AsmStmt *value)
: var(var)
, value(value)
{

}

AsmCondJmp::AsmCondJmp(AsmStmt *condition, AsmBlock *target)
: condition(condition)
, target(target)
{

}

/*
   in: xmm0 <- expression, xmm1 <- expression2

   addps xmm0, xmm1

   out: xmm0 -> thisexpr
*/

/*
   in: xmm0 <- expr1, xmm1 <- expr2

   call func

   out: xmm0 -> rv, xmm1 -> garbage, xmm2 -> garbage
*/



