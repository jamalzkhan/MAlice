#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "cflow.h"
#include "assembly.h"



void printTree(struct TokenStruct *Token, int indents)
{
    int i;
    for(i = 0; i < indents; i++)
    {
        printf("  ");
    }
    printf("Rule: %ls | Symbol: %ls | Data: %ls\n", Grammar.RuleArray[Token->ReductionRule].Description, Grammar.SymbolArray[Token->Symbol].Name, Token->Data);
    for(i = 0; i < Grammar.RuleArray[Token->ReductionRule].SymbolsCount; i++)
    {
        printTree(Token->Tokens[i],indents+1);
    }
}


int compile(int debug, int printSource, int printAssembly, int optimisation, char *fname)
{
    wchar_t *InputBuf;
    struct TokenStruct *Token;
    struct ContextStruct Context;
    int Result;

    /* Load the inputfile into memory. */
    InputBuf = LoadInputFile(fname);
    if (InputBuf == NULL) exit(1);
    
    if(printSource)
        printf("Input MAlice program:\n-- %s --\n%ls---\n\n",fname,InputBuf);

    /* Run the Parser. */
    Result = Parse(InputBuf,wcslen(InputBuf),TRIMREDUCTIONS,DEBUG,&Token);
    /* Interpret the results. */
    if (Result != PARSEACCEPT) {
        ShowErrorMessage(Token,Result);
        
        printf("\n[EXIT] Syntax Error\n");
        return 255;
    } else {
        /* Initialize the Context. */
        Context.Debug = DEBUG;
        Context.Indent = 0;
        Context.ReturnValue = NULL;

        /* Start execution by calling the subroutine of the first Token on
           the TokenStack. It's the "Start Symbol" that is defined in the
           grammar. */
        RuleJumpTable[Token->ReductionRule](Token,&Context);
    }
    
    //printTree(Token,0);
    
    int error = debug; // Used as debug flag and error reporting during CFG gen
    CFG * cfg = makeCFG(Token, optimisation, &error);
    if(error || cfg == 0)
    {
        printf("\n[EXIT] Compilation Error\n");
        return 255;
    }
    char* asmfname;
    asmfname = asmName(fname);
    
    genCode(cfg, asmfname);
    if(printAssembly)
    {
        printf("Generated assembly code:\n-- %s --\n",asmfname);
        printFile(asmfname);
        printf("---\n\n");
    }
    free(asmfname);

    char* exefname;
    exefname = exeName(fname);
    char fc[128];
    char sc[128];

    sprintf(fc ,"nasm -f elf32 %s.s -o %s.o", exefname, exefname);
    sprintf(sc ,"gcc -m32 %s.o -o %s", exefname, exefname);

    char * firstCommand = (char *)malloc(sizeof(char) * strlen(fc));
    char * secondCommand = (char *)malloc(sizeof(char) * strlen(sc));
    strcpy(firstCommand, fc);
    strcpy(secondCommand, sc);

    system(firstCommand);
    system(secondCommand);
    
    /* Cleanup. */
    DeleteTokens(Token);
    free(InputBuf);
    return 0;
}


int main(int argc, char *argv[])
{
  
    char * fname = "ex.alice";

  int debug = 0;
    int printSource = 0;
    int printAssembly = 0;
    int optimisation = 2;
    //int i;
/*
    for (i = 1; i < argc; i++)
    {
        if (!(strcmp(argv[i],"-s") && strcmp(argv[i],"-source"))) printSource = 1;
        else if (!(strcmp(argv[i],"-a") && strcmp(argv[i],"-assembly"))) printAssembly = 1;
        else if (!(strcmp(argv[i],"-d") && strcmp(argv[i],"-debug"))) debug = 1;
        else if (!(strcmp(argv[i],"-o"))) optimisation = 1;
        else if (!(strncmp(argv[i],"-o",2))) optimisation = atoi(&argv[i][2]);
        else
        {
	    if (i == 1 && strstr(argv[1],".alice") != NULL) fname = argv[1];
            else 
            {
                printf("Unrecognized flag %s\n", argv[i]);
                return 1;
            }
        }
    }*/
    int error;
    if (argc > 1) fname = argv[1];
    error = compile(debug, printSource, printAssembly, optimisation, fname);

    return error;
}
