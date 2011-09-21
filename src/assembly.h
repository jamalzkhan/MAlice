#ifndef ASSEMBLY_H
#define ASSEMBLY_H


#include "vector.h"
#include "cflow.h"

char* asmName(char* fname);
char* exeName(char* aname);
void printFile(char* fname);
void genCode(CFG * cfg, char* fname);

#endif
