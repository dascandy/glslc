#include "Transformation.h"
#include "Glsl.h"

static void AppendConvertRvalue(Rvalue *rv, BasicBlock *&appendToThis, BasicBlock *returnTarget, BasicBlock *discardTarget, BasicBlock *breakTarget, BasicBlock *continueTarget, Variable *retval, std::vector<BasicBlock *> &bbs) {
  switch (rv->nt) {
    case NScopeStmt: {
      ScopeStmt *stmt = (ScopeStmt *)rv;

      for (Rvalue *v : stmt->statements)
        AppendConvertRvalue(v, appendToThis, returnTarget, discardTarget, breakTarget, continueTarget, retval, bbs);
    }
    break;

    case NForLoop: {
      ForLoop *stmt = (ForLoop *)rv;
      BasicBlock *body = new BasicBlock();
      BasicBlock *update = new BasicBlock();
      BasicBlock *after = new BasicBlock();
      bbs.push_back(body);
      BasicBlock *bodyStart = body;
      body->refs += 2;
      update->refs++;
      after->refs += 2;
      appendToThis->statements.push_back(stmt->inits);
      appendToThis->statements.push_back(new ConditionalJump(new Expression(Expression::invert, stmt->condition), after));
      appendToThis->nextBlock = body;
      AppendConvertRvalue(stmt->loopBody, body, returnTarget, discardTarget, after, update, retval, bbs);
      body->nextBlock = update;
      AppendConvertRvalue(stmt->update, update, returnTarget, discardTarget, breakTarget, continueTarget, retval, bbs);
      update->statements.push_back(new ConditionalJump(stmt->condition, bodyStart));
      bbs.push_back(update);
      bbs.push_back(after);
      update->nextBlock = after;
      appendToThis = after;
    }
    break;

    case NWhileLoop: {
      WhileLoop *stmt = (WhileLoop *)rv;
      BasicBlock *body = new BasicBlock();
      BasicBlock *update = new BasicBlock();
      BasicBlock *after = new BasicBlock();
      bbs.push_back(body);
      body->refs += 2;
      update->refs++;
      after->refs += 2;
      appendToThis->statements.push_back(new ConditionalJump(new Expression(Expression::invert, stmt->condition), after));
      update->statements.push_back(new ConditionalJump(stmt->condition, body));
      appendToThis->nextBlock = body;
      AppendConvertRvalue(stmt->loopBody, body, returnTarget, discardTarget, after, update, retval, bbs);
      body->nextBlock = update;
      bbs.push_back(update);
      bbs.push_back(after);
      update->nextBlock = after;
      appendToThis = after;
    }
    break;

    case NIfStmt: {
      IfStatement *stmt = (IfStatement *)rv;

      if (stmt->elseB) {
        BasicBlock *thenB = new BasicBlock();
        BasicBlock *elseB = new BasicBlock();
        BasicBlock *after = new BasicBlock();
        bbs.push_back(thenB);
        appendToThis->statements.push_back(new ConditionalJump(new Expression(Expression::invert, stmt->condition), elseB));
        appendToThis->nextBlock = thenB;
        AppendConvertRvalue(stmt->thenB, thenB, returnTarget, discardTarget, breakTarget, continueTarget, retval, bbs);
        bbs.push_back(elseB);
        AppendConvertRvalue(stmt->elseB, elseB, returnTarget, discardTarget, breakTarget, continueTarget, retval, bbs);
        bbs.push_back(after);
        thenB->nextBlock = after;
        elseB->nextBlock = after;
        appendToThis = after;
        thenB->refs++;
        elseB->refs++;
        after->refs += 2;
      } else {
        BasicBlock *thenB = new BasicBlock();
        BasicBlock *after = new BasicBlock();
        bbs.push_back(thenB);
        appendToThis->statements.push_back(new ConditionalJump(new Expression(Expression::invert, stmt->condition), after));
        appendToThis->nextBlock = thenB;
        AppendConvertRvalue(stmt->thenB, thenB, returnTarget, discardTarget, breakTarget, continueTarget, retval, bbs);
        thenB->nextBlock = after;
        bbs.push_back(after);
        appendToThis = after;
        thenB->refs++;
        after->refs += 2;
      }
    }
    break;

    /*
    case NBreak:
      if (!breakTarget)
        CompileError(rv->loc, "Break without statement to break out of");
      else {
        appendToThis->nextBlock = breakTarget;
        breakTarget->refs++;
      }

      break;
    case NContinue:
      if (!continueTarget)
        CompileError(rv->loc, "Continue without statement to continue");
      else {
        appendToThis->nextBlock = continueTarget;
        continueTarget->refs++;
      }

      break;
    */
    case NReturn: {
      ReturnExpression *rs = (ReturnExpression *)rv;
      // Todo: type check
      appendToThis->statements.push_back(new Expression(Expression::assign, new VarRef(rs->def, retval), rs->rv));
      appendToThis->nextBlock = returnTarget;
      returnTarget->refs++;
    }
    break;

    case NDiscard:
      appendToThis->nextBlock = discardTarget;
      discardTarget->refs++;
      break;

    default:
      appendToThis->statements.push_back(rv);
      break;
  }
}

static void ConvertFunction(Function *function, BasicBlock *discardTarget) {
  BasicBlock *body = new BasicBlock();
  BasicBlock *returnT = new BasicBlock();
  function->bbs.push_back(body);
  body->refs++;
  Variable *var = NULL;

  if (function->type.type != Type::Tvoid) {
    var = new Variable(function->type, "$@", -1);
    function->arguments.push_back(new FuncArg(function->def, "out", var->type, var->name, NULL));
  }

  AppendConvertRvalue(function->body, body, returnT, discardTarget, NULL, NULL, var, function->bbs);

  if (returnT->refs) {
    if (returnT->refs == 1 &&
        function->bbs.back()->nextBlock == returnT) {
      function->bbs.back()->nextBlock = NULL;
      delete returnT;
    } else {
      function->bbs.back()->nextBlock = returnT;
      function->bbs.push_back(returnT);
      returnT->refs++;
    }
  } else
    delete returnT;
}

void HL2ML(Shader *shader) {
  BasicBlock *discard = shader->discard = new BasicBlock();

  for (std::pair<const std::string, std::vector<Function *>> &funcvec : shader->functions) {
    for (Function *func : funcvec.second)
      ConvertFunction(func, discard);
  }
}


