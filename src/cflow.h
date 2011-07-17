#ifndef CFLOW_H
#define CFLOW_H

#include "parser.h"
#include "vector.h"

#define CHAR 12
#define FORK_EXPR 21
#define FORK_XOR 23
#define FORK_AND 25
#define FORK_SUM 27
#define FORK_TRMS 29
#define NOT 31
#define IDENTITY 33
#define CONSTANT 34
#define NUMBER 26

#define PLUS 13
#define MULT 6
#define DIV 9

#define DECLARATION 6
#define DECLARATION_TOO 7
#define ASSIGNMENT 8
#define ATE 9
#define DRANK 10
#define RETURN 20

enum Command{Add,Sub,Mod,Mul,Div,And,Or,Xor,Mov,Not,Dec,Inc};

typedef struct varNode
{
    char* name;
    int type;
    int reg;
    int assigned;
    struct varNode * next;
} Var;

typedef struct varTable
{
    Var* first;
    Var* current;
    int size;
} VTable;

typedef struct CFGNode
{
    int dest;
    int src;
    int srcReg;    // Is source another register or just a value (Bool)
    int varAssign; // Is an assignment of a variable
    enum Command cmd;
    struct CFGNode* next;
    Vector * liveIn;
    Vector * liveOut;
} Node;

typedef struct CFGTable
{
    Node * first;
    Node * current;
    int size;
    int maxReg;
} CFG;


CFG * makeCFG(struct TokenStruct *Token, int optims, int * error);
void printCFG(CFG * cfg);

#endif
