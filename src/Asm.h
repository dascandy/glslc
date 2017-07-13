#ifndef ASM_H
#define ASM_H

#include "Glsl.h"

class AsmStmt {
public:
  virtual ~AsmStmt() {}
};

class AsmBlock {
public:
  void Add(AsmStmt *stmt);
  std::vector<AsmStmt *> stmts;
};

class ConstLoadExpr : public AsmStmt {
public:
  ConstLoadExpr(ConstVal *val);
  ConstVal *val;
};

class VarLoadExpr : public AsmStmt {
  public:
  VarLoadExpr(Variable *var);
  Variable *var;
};

class AsmOp : public AsmStmt {
public:
  AsmOp(const std::string &opcodeName);
  AsmOp(const std::string &opcodeName, AsmStmt *arg1);
  AsmOp(const std::string &opcodeName, AsmStmt *arg1, AsmStmt *arg2);
  std::string opcodeName;
  std::vector<AsmStmt*> inputs;
};

class AsmCall : public AsmStmt {
public:
  AsmCall(AsmBlock *target, std::vector<AsmStmt *> args);
  AsmBlock *target;
  std::vector<AsmStmt *> args;
};

class AsmStore : public AsmStmt {
public:
  AsmStore(Variable *var, AsmStmt *value);
  Variable *var;
  AsmStmt *value;
};

class AsmCondJmp : public AsmStmt {
public:
  AsmCondJmp(AsmStmt *condition, AsmBlock *target);
  AsmStmt *condition;
  AsmBlock *target;
};
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

#endif


