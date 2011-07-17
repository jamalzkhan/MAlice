#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>                  /* wchar_t */
#include <sys/stat.h>

#include "engine.h"                 /* The Kessels engine. */

#define TRIMREDUCTIONS 0            /* 0=off, 1=on */
#define DEBUG          0            /* 0=off, 1=on */

/* Struct for transporting data between rules. Add whatever you need.
   Note: you could also use global variables to store stuff, but using
   a struct like this makes the interpreter thread-safe. */
struct ContextStruct {
  wchar_t *ReturnValue;             /* In this template all rules return a string. */
  int Indent;                       /* For printing debug messages. */
  int Debug;                        /* 0=off, 1=on */
  };
  
/* Forward definition of the RuleJumpTable. It will be filled with a link
   to a subroutine for every rule later on. */
void (*RuleJumpTable[40])(struct TokenStruct *Token, struct ContextStruct *Context);

wchar_t *LoadInputFile(char *FileName);
void ShowErrorMessage(struct TokenStruct *Token, int Result);

#endif
