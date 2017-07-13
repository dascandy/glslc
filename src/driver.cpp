#include "parser.hpp"
#include <iostream>
#include "Glsl.h"
#include "Transformation.h"
#include "Asm.h"
using std::cout;
using std::endl;

extern "C" int yylex();
int yyparse();
extern "C" const char *yytext;
extern "C" FILE *yyin;

extern "C" void yyerror(const char *s);
extern "C" const char *print(int token);
extern Shader *shader;
extern int yylineno;

bool compileErrors = false;

std::string yyfilename;
int main(int argc, char **argv) {
  yyfilename = argv[1];
  FILE *input = fopen(argv[1], "r");

  if (!input) {
    CompileError(new DefLocation {argv[1], 0, 0}, "File not found!");
    return -1;
  }

  yyin = input;
  yyparse();
  fclose(input);
  PatchUpMain(shader);
  HL2ML(shader);
  ML2LL(shader);
  unsigned char *buf;
  size_t size;
  LL2Asm(shader, buf, size);

  if (compileErrors) return -1;

  printf("Cvals:\n");

  for (ConstVal *cv : shader->cvars) {
    printf("Cvar at %p: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", cv,
           (unsigned char)cv->value[0],
           (unsigned char)cv->value[1],
           (unsigned char)cv->value[2],
           (unsigned char)cv->value[3],
           (unsigned char)cv->value[4],
           (unsigned char)cv->value[5],
           (unsigned char)cv->value[6],
           (unsigned char)cv->value[7],
           (unsigned char)cv->value[8],
           (unsigned char)cv->value[9],
           (unsigned char)cv->value[10],
           (unsigned char)cv->value[11],
           (unsigned char)cv->value[12],
           (unsigned char)cv->value[13],
           (unsigned char)cv->value[14],
           (unsigned char)cv->value[15]);
  }

  for (std::map<std::string, std::vector<Function *>>::iterator it = shader->functions.begin(); it != shader->functions.end(); ++it) {
    std::vector<Function *> &fs = it->second;

    if (fs.size() == 0) continue;

    for (Function *f : fs) {
      if (f->calls == 0) continue;

      printf("function %s(", f->name.c_str());

      for (std::pair<std::string, Variable *> v : f->scope->variables)
        printf("var %s\n", v.first.c_str());

      for (FuncArg *fa : f->arguments)
        printf("%s|", fa->name.c_str());

      printf(");\n");
      f->body->Print(0);
      printf("basicblocks:\n");
      int tgtN = 1;
      std::map<BasicBlock *, int> name;
      BasicBlock *nextBlock = NULL;

      for (BasicBlock *bb : f->bbs) {
        if (nextBlock &&
            bb != nextBlock) {
          if (name[nextBlock] == 0)
            name[nextBlock] = tgtN++;

          printf("   jmp to %d\n", name[nextBlock]);
        }

        if (bb->refs > 1 ||
            nextBlock != bb) {
          if (name[bb] == 0)
            name[bb] = tgtN++;

          printf("%d:\n", name[bb]);
        }

        for (Rvalue *rv : bb->statements) {
          if (rv->nt == NConditionalJump) {
            if (name[((ConditionalJump *)rv)->target] == 0)
              name[((ConditionalJump *)rv)->target] = tgtN++;

            printf("    when ");
            ((ConditionalJump *)rv)->choice->Print(0);
            printf(" then jump to %d\n", name[((ConditionalJump *)rv)->target]);
          } else {
            printf("    ");
            rv->Print(0);
            printf("\n");
          }
        }

        nextBlock = bb->nextBlock;
      }

      if (nextBlock)
        printf("   jmp to %d\n", name[nextBlock]);

      int curid = 0;
      std::map<AsmStmt *, int> ids;
      printf("\n\nASM blocks:\n");
      for (AsmBlock *ab : f->asmblocks) {
        printf("block %p\n", ab);
        for (AsmStmt *st : ab->stmts) {
          ids.insert(std::make_pair(st, curid));
          ConstLoadExpr *cle = dynamic_cast<ConstLoadExpr *>(st);
          AsmOp *ao = dynamic_cast<AsmOp *>(st);
          AsmCall *ac = dynamic_cast<AsmCall *>(st);
          AsmStore *as = dynamic_cast<AsmStore *>(st);
          AsmCondJmp *acj = dynamic_cast<AsmCondJmp *>(st);
          VarLoadExpr *vle = dynamic_cast<VarLoadExpr *>(st);
          if (cle) {
            printf("  t% 3d: ld %p\n", curid, cle->val);
          } else if (ao) {
            printf("  t% 3d: %s", curid, ao->opcodeName.c_str());
            for (AsmStmt *stmt : ao->inputs) {
              printf(" t%d", ids[stmt]);
            }
            printf("\n");
          } else if (ac) {
            printf("  t% 3d: call %p", curid, ac->target);
            for (AsmStmt *stmt : ac->args) {
              printf(" t%d", ids[stmt]);
            }
            printf("\n");
          } else if (as) {
            printf("  t% 3d: st v%p, t%d\n", curid, as->var, ids[as->value]);
          } else if (acj) {
            if (acj->condition) {
              printf("  t% 3d: jc t%d, %p\n", curid, ids[acj->condition], acj->target);
            } else {
              printf("  t% 3d: jmp %p\n", curid, acj->target);
            }
          } else if (vle) {
            printf("  t% 3d: ld v %p\n", curid, vle->var);
          }
          curid++;
        }
      }
    }
  }

  for (std::map<std::string, Variable *>::iterator it = shader->in.variables.begin(); it != shader->in.variables.end(); ++it)
    printf("in var %s\n", it->first.c_str());

  for (std::map<std::string, Variable *>::iterator it = shader->out.variables.begin(); it != shader->out.variables.end(); ++it)
    printf("out var %s\n", it->first.c_str());

  for (std::map<std::string, Variable *>::iterator it = shader->uniform.variables.begin(); it != shader->uniform.variables.end(); ++it)
    printf("uniform var %s\n", it->first.c_str());
}

void yyerror(const char *msg) {
  DefLocation here {yyfilename, (size_t)yylineno, 0};
  CompileError(&here, "%s\n", msg);
}


