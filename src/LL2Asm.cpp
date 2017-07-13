#include "Asm.h"
#include "Glsl.h"
#include <map>
#include <set>

void LL2Asm(Shader *shader, unsigned char *&target, size_t &length) {
  int curid = 0;
  std::vector<AsmStmt *> stmts, stwm;
  std::map<AsmStmt *, int> ids;
  std::map<int, int> lastIndex;
  for (std::pair<std::string, std::vector<Function *> > fs : shader->functions) {
    for (Function *f : fs.second) {
      if (f->calls == 0) continue;
      for (AsmBlock *ab : f->asmblocks) {
        for (AsmStmt *st : ab->stmts) {
          ids[st] = curid;
          curid++;
          stmts.push_back(st);
        }
      }
    }
  }
  for (AsmStmt *st : stmts) {
    int id = ids[st];
    lastIndex[id] = id;
    AsmOp *ao = dynamic_cast<AsmOp *>(st);
    AsmCall *ac = dynamic_cast<AsmCall *>(st);
    AsmStore *as = dynamic_cast<AsmStore *>(st);
    AsmCondJmp *acj = dynamic_cast<AsmCondJmp *>(st);
    if (ao) {
      for (AsmStmt *stmt : ao->inputs) {
        lastIndex[ids[stmt]] = id;
      }
    } else if (ac) {
      for (AsmStmt *stmt : ac->args) {
        lastIndex[ids[stmt]] = id;
      }
    } else if (as) {
      lastIndex[ids[as->value]] = id;
    } else if (acj) {
      if (acj->condition) {
        lastIndex[ids[acj->condition]] = id;
      }
    }
  }

  std::set<int> emptyRegs;
  std::map<int, int> curUse;
  std::map<int, int> regFor;
  for (int i = 0; i < 16; i++) {
    emptyRegs.insert(i);
  }
  for (AsmStmt *st : stmts) {
    int id = ids[st];
    printf("%zu empty\n", emptyRegs.size());
    if (emptyRegs.size() == 0) {
      printf("Can't find empty register! Ouch!\n");
      return;
    }

    for (std::map<int, int>::iterator it = curUse.begin(); it != curUse.end(); ++it) {
      if (lastIndex[it->second] <= id) {
        emptyRegs.insert(it->first);
      }
    }

    int reg = *emptyRegs.begin();
    emptyRegs.erase(reg);
    curUse[reg] = id;
    regFor[id] = reg;
    
    ConstLoadExpr *cle = dynamic_cast<ConstLoadExpr *>(st);
    AsmOp *ao = dynamic_cast<AsmOp *>(st);
    AsmCall *ac = dynamic_cast<AsmCall *>(st);
    AsmStore *as = dynamic_cast<AsmStore *>(st);
    AsmCondJmp *acj = dynamic_cast<AsmCondJmp *>(st);
    VarLoadExpr *vle = dynamic_cast<VarLoadExpr *>(st);
    if (cle) {
      printf("  MOVAPS r%d, %p\n", regFor[ids[st]], cle->val);
    } else if (ao) {
      printf("  %s r%d", ao->opcodeName.c_str(), regFor[ids[st]]);
      for (AsmStmt *stmt : ao->inputs) {
        printf(", r%d", regFor[ids[stmt]]);
      }
      printf("\n");
    } else if (ac) {
      int i = 0;
      for (AsmStmt *stmt : ac->args) {
        if (regFor[ids[stmt]] != i) {
          int r = regFor[ids[stmt]];
          int e = curUse[i];
          curUse[i] = ids[stmt];
          curUse[r] = e;
          regFor[ids[stmt]] = i;
          regFor[e] = r;
          printf("  XCHG r%d, r%d\n", i, regFor[ids[stmt]]);
        }
        i++;
      }
      if (ac->args.empty()) {
        emptyRegs.erase(0);
      }
      curUse[0] = id;
      printf("  CALL %p\n", ac->target);
    } else if (as) {
      printf("  MOVAPS %p, r%d\n", as->var, regFor[ids[as->value]]);
    } else if (acj) {
      if (acj->condition) {
        printf("  TEST r%d\n", regFor[ids[acj->condition]]);
        printf("  JZ %p\n", acj->target);
      } else {
        printf("  JMP %p\n", acj->target);
      }
    } else if (vle) {
      printf("  MOVAPS r%d, %p\n", regFor[ids[st]], vle->var);
    }

    stwm.push_back(st);

    for (int reg : emptyRegs) {
      if (curUse.find(reg) != curUse.end()) {
        regFor.erase(curUse[reg]);
        curUse.erase(reg);
      }
    }
  }
}


