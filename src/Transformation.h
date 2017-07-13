#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <stddef.h>

class Shader;

void HL2ML(Shader *shader);
void ML2LL(Shader *shader);
void LL2Asm(Shader *shader, unsigned char *&target, size_t &length);

void ConstantExtraction(Shader *shader);

void InlineMethods(Shader *shader);
void PatchUpMain(Shader *shader);


#endif


