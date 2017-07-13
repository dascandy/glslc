#include "Transformation.h"
#include "Glsl.h"
#include "Asm.h"

static ConstLoadExpr *ConvertImm(Rvalue *rv, AsmBlock *curBlock, std::vector<ConstVal *> &cvars) {
  ConstLoadExpr *rval = NULL;
  ConstVal *newVal;

  switch (rv->nt) {
    case NConstantInt:
      newVal = new ConstVal(*(ImmediateIntExpression *)rv);
      break;

    case NConstantUInt:
      newVal = new ConstVal(*(ImmediateUintExpression *)rv);
      break;

    case NConstantFloat:
      newVal = new ConstVal(*(ImmediateFloatExpression *)rv);
      break;
  }

  for (ConstVal *v : cvars) {
    if (*v == *newVal) {
      rval = new ConstLoadExpr(v);
      break;
    }
  }

  if (!rval) {
    cvars.push_back(newVal);
    rval = new ConstLoadExpr(newVal);
  }

  curBlock->Add(rval);
  return rval;
}

static AsmStmt *RecurseRvalue(Rvalue *rv, AsmBlock *curblock, std::map<BasicBlock *, AsmBlock *> &mapping, std::vector<ConstVal *> &cvars) {
  switch (rv->nt) {
    case NConstantInt:
    case NConstantUInt:
    case NConstantFloat:
      return ConvertImm(rv, curblock, cvars);
    case NFunctionCall: {
      FunctionCall *f = (FunctionCall *)rv;
      std::vector<AsmStmt *> args;
      for (Rvalue *&arg : f->arguments) {
        args.push_back(RecurseRvalue(arg, curblock, mapping, cvars));
      }
      AsmStmt *nv = new AsmCall(mapping[f->func->bbs.front()], args);
      curblock->Add(nv);
      return nv;
    }
    break;

    case NExpression: {
      Expression *e = (Expression *)rv;
      AsmStmt *left = NULL;
      if (e->op != Expression::assign) left = RecurseRvalue(e->left, curblock, mapping, cvars);
      AsmStmt *right = NULL;
      if (e->right) right = RecurseRvalue(e->right, curblock, mapping, cvars);
      AsmStmt *nv = NULL;
      switch(e->op) {
        case Expression::comma:
          return right;
        case Expression::max: {
          AsmOp *op = new AsmOp("MAXPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::min: {
          AsmOp *op = new AsmOp("MINPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::plus: {
          AsmOp *op = new AsmOp("ADDPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::minus: {
          AsmOp *op = new AsmOp("SUBPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::times: {
          AsmOp *op = new AsmOp("MULPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::divide: {
          AsmOp *op = new AsmOp("DIVPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::assign: {
          VarRef *vr = (VarRef*)e->left;
          curblock->Add(new AsmStore(vr->var, right));
          return right;
        }
        case Expression::comp_eq: {
          AsmOp *op = new AsmOp("CMPEQPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::comp_ge: {
          AsmOp *op = new AsmOp("CMPNLTPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::comp_gt: {
          AsmOp *op = new AsmOp("CMPNLEPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::comp_le: {
          AsmOp *op = new AsmOp("CMPLEPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::comp_lt: {
          AsmOp *op = new AsmOp("CMPLTPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::comp_ne: {
          AsmOp *op = new AsmOp("CMPNEQPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::invert: {
          AsmOp *op = new AsmOp("ANDNPS", left, left);
          curblock->Add(op);
          return op;
        }
        case Expression::neg: {
          right = ConvertImm(new ImmediateIntExpression(DefLocation{"Builtin", 0, 0}, "0"), curblock, cvars);
          AsmOp *op = new AsmOp("SUBPS", right, left);
          curblock->Add(op);
          return op;
        }
        case Expression::booland: {
          AsmOp *op = new AsmOp("ANDPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::boolor: {
          AsmOp *op = new AsmOp("ORPS", left, right);
          curblock->Add(op);
          return op;
        }
        case Expression::postincr: {
          right = ConvertImm(new ImmediateIntExpression(DefLocation{"Builtin", 0, 0}, "1"), curblock, cvars);
          VarRef *vr = (VarRef *)e->left;
          AsmOp *op = new AsmOp("ADDPS", left, right);
          curblock->Add(op);
          curblock->Add(new AsmStore(vr->var, op));
          return left;
        }
      }
      curblock->Add(nv);
      return nv;
    }
    break;

    case NShuffle: {
      ShuffleExpression *se = (ShuffleExpression *)rv;
      AsmStmt *arg = RecurseRvalue(se->left, curblock, mapping, cvars);
      AsmStmt *val = new AsmOp("SHUFPS-" + se->shuffle, arg);
      curblock->Add(val);
      return val;
    }
    break;

    case NConditionalJump: {
      ConditionalJump *cj = (ConditionalJump *)rv;
      AsmStmt *condition = RecurseRvalue(cj->choice, curblock, mapping, cvars);
      AsmStmt *nv = new AsmCondJmp(condition, mapping[cj->target]);
      curblock->Add(nv);
      return nv;
    }

    case NVarRef: {
      VarRef *vr = (VarRef *)rv;
      AsmStmt *nv = new VarLoadExpr(vr->var);
      curblock->Add(nv);
      return nv;
    }

  }
}

void ML2LL(Shader *shader) {
  std::map<BasicBlock *, AsmBlock *> mapping;

  for (std::pair<const std::string, std::vector<Function *>> &funcvec : shader->functions) {
    for (Function *func : funcvec.second) {
      for (BasicBlock *bb : func->bbs) {
        mapping[bb] = new AsmBlock();
        func->asmblocks.push_back(mapping[bb]);
      }
    }
  }
  for (std::pair<const std::string, std::vector<Function *>> &funcvec : shader->functions) {
    for (Function *func : funcvec.second) {
      for (BasicBlock *bb : func->bbs) {
        for (Rvalue *rv : bb->statements) {
          RecurseRvalue(rv, mapping[bb], mapping, shader->cvars);
        }
        if (bb->nextBlock) {
          AsmStmt *nv = new AsmCondJmp(NULL, mapping[bb->nextBlock]);
          mapping[bb]->Add(nv);
        } else {
          AsmStmt *nv = new AsmOp("RET");
          mapping[bb]->Add(nv);
        }
      }
    }
  }
}


