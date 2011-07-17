#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cflow.h"

/* ---------------------------------------------------------------------------*/
// Translating parse tree to CFG

void newVar(VTable * table, char* name, int type, int reg)
{
    Var * v = malloc(sizeof(Var));
    v->name = malloc(strlen(name) * sizeof(char));
    strcpy(v->name, name);
    v->type = type;
    v->reg  = reg;
    v->assigned = 0;
    v->next = 0;
    if(table->size == 0)
    {
        table->first = v;
        table->size = 1;
    }
    else
    {
        table->current->next = v;
        table->size++;
    }
    table->current = v;
}

Var* isInTable(VTable* table, char* name)
{
    if(table->first == 0)
    {
        return 0;
    }
    Var * vn = table->first;
    while(vn != 0)
    {
        if(strcmp(vn->name, name) == 0)
        {
            return vn;
        }
        vn = vn->next;
    }
    return 0;
}

void addNode(CFG* cfg, enum Command cmd, int dest, int src, int srcReg, int varAssign)
{
    Node * newNode = malloc(sizeof(Node));
    newNode->cmd = cmd;
    newNode->dest = dest;
    newNode->src = src;
    newNode->srcReg = srcReg;
    newNode->varAssign = varAssign;
    newNode->liveIn = 0;
    newNode->liveOut = 0;
    newNode->next = 0;
    
    if (cfg->first == 0)
    {
        cfg->first = newNode;
        cfg->size = 1;                  
    }
    else
    {
        cfg->current->next = newNode;
        cfg->size++;   
    }
    cfg->current = newNode;
}

void parseExp(struct TokenStruct * Token, CFG * cfg, VTable * table, int complex, int targetReg, int * error)
{
    //printf("Parsing %ls\n", Grammar.RuleArray[Token->ReductionRule].Description);
    char tmp[256];
    if(targetReg > cfg->maxReg)
        cfg->maxReg = targetReg;

    switch(Token->ReductionRule) {
    case CHAR:
    {
        int val;
        sprintf(tmp, "%ls", Token->Tokens[0]->Data);
        sscanf(tmp, "'%c'", (char *)&val);
        //printf("mov T%d %d (%ls)\n", targetReg, val, Token->Tokens[0]->Data);
        addNode(cfg, Mov, targetReg, val, 0, 0);
        break;
    }
    case FORK_EXPR:
    {
        parseExp(Token->Tokens[0], cfg, table, 1, targetReg, error);
        parseExp(Token->Tokens[2], cfg, table, 1, targetReg + 1, error);
        //printf("or T%d T%d\n", targetReg, targetReg + 1);
        addNode(cfg, Or, targetReg, targetReg+1, 1, 0);
        break;
    }
    case FORK_XOR:
    {
        parseExp(Token->Tokens[0], cfg, table, 1, targetReg, error);
        parseExp(Token->Tokens[2], cfg, table, 1, targetReg + 1, error);
        //printf("xor T%d T%d\n", targetReg, targetReg + 1);
        addNode(cfg, Xor, targetReg, targetReg+1, 1, 0);
        break;
    }
    case FORK_AND:
    {
        parseExp(Token->Tokens[0], cfg, table, 1, targetReg, error);
        parseExp(Token->Tokens[2], cfg, table, 1, targetReg + 1, error);
        //printf("and T%d T%d\n", targetReg, targetReg + 1);
        addNode(cfg, And, targetReg, targetReg+1, 1, 0);
        break;
    }
    case FORK_SUM:
    {
        parseExp(Token->Tokens[0], cfg, table, 1, targetReg, error);
        parseExp(Token->Tokens[2], cfg, table, 1, targetReg + 1, error);
        if (Token->Tokens[1]->Tokens[0]->Symbol == PLUS) {
            //printf("add T%d T%d\n", targetReg, targetReg + 1);
            addNode(cfg, Add, targetReg, targetReg+1, 1, 0);
        }
        else {
            //printf("sub T%d T%d\n", targetReg, targetReg + 1);
            addNode(cfg, Sub, targetReg, targetReg+1, 1, 0);
        }
        break;
    }
    case FORK_TRMS:
    {
        parseExp(Token->Tokens[0], cfg, table, 1, targetReg, error);
        parseExp(Token->Tokens[2], cfg, table, 1, targetReg + 1, error);
        if (Token->Tokens[1]->Tokens[0]->Symbol == MULT) {
            //printf("mul T%d T%d\n", targetReg, targetReg + 1);
            addNode(cfg, Mul, targetReg, targetReg+1, 1, 0);
        }
        else if (Token->Tokens[1]->Tokens[0]->Symbol == DIV)  {
            //printf("div T%d T%d\n", targetReg, targetReg + 1);
            addNode(cfg, Div, targetReg, targetReg+1, 1, 0);
        }
        else {
            //printf("mod T%d T%d\n", targetReg, targetReg + 1);
            addNode(cfg, Mod, targetReg, targetReg+1, 1, 0);
        }
        break;
    }
    case NOT:
    {
        parseExp(Token->Tokens[1], cfg, table, complex, targetReg, error);
        //printf("not T%d\n", targetReg);
        addNode(cfg, Not, targetReg, 0, -1, 0);
        break;
    }
    case IDENTITY:
    {
        sprintf(tmp, "%ls", Token->Tokens[0]->Data);
        Var* v = isInTable(table, tmp);
        if(v != 0) // Variable exists in var table
        {
            if(v->type || (!complex && !v->type))
            {
                //printf("mov T%d T%d\n", targetReg, v->reg);
                if(!v->assigned)
                    printf("Warning at line %ld column %ld.\n  Variable '%s' has not been assigned a value yet.\n", Token->Line, Token->Column, v->name);
                addNode(cfg, Mov, targetReg, v->reg, 1, 0);
            }
            else
            {
                 printf("Error at line %ld column %ld.\n  Variable '%s' is a letter, cannot cast it to a number.\n", Token->Line, Token->Column, v->name);
                 *error = 1;
                 return;
            }
        }
        else
        {
            printf("Error at line %ld column %ld.\n  Variable '%ls' has not been declared.\n", Token->Line, Token->Column, Token->Tokens[0]->Data);
            *error = 1;
            return;
        }
        break;
    }
    case CONSTANT:
    {
        int val;
        sprintf(tmp, "%ls", Token->Tokens[0]->Data);
        sscanf(tmp, "%d", &val);
        //printf("mov T%d %d\n", targetReg, val);
        addNode(cfg, Mov, targetReg, val, 0, 0);
        break;
    }
    default:
        parseExp(Token->Tokens[0], cfg, table, complex, targetReg, error);
        break;
    }

}

void parseTree(struct TokenStruct * Token, CFG * cfg, VTable * table, int * regCount, int * error)
{
    int i;
    char tmp[256];
    //printf("Parsing %ls\n", Grammar.RuleArray[Token->ReductionRule].Description);
    switch (Token->ReductionRule)
    {
    case DECLARATION:
    case DECLARATION_TOO:
    {
        sprintf(tmp,"%ls",Token->Tokens[0]->Data);
        if(isInTable(table, tmp) == 0)
        {
            newVar(table, tmp, Token->Tokens[3]->Tokens[0]->Symbol == NUMBER, *regCount+1);
    	    (*regCount)++;
        }
        else
        {
            printf("Error at line %ld column %ld.\n  Variable '%ls' has already been declared.\n", Token->Line, Token->Column, Token->Tokens[0]->Data);
            *error = 1;
        }
        break;
    }
    case ASSIGNMENT:
    {
        sprintf(tmp, "%ls", Token->Tokens[0]->Data);
        Var* v = isInTable(table, tmp);
        if(v != 0) // Variable exists in var table
        {
            parseExp(Token->Tokens[2], cfg, table, v->type, *regCount+1, error);
            v->assigned = 1;
            //printf("mov T%d T%d\n", v->reg, *regCount+1);
            addNode(cfg, Mov, v->reg, *regCount+1, 1, 1);
        }
        else
        {
            printf("Error at line %ld column %ld.\n  Variable '%ls' has not been declared.\n", Token->Line, Token->Column, Token->Tokens[0]->Data);
            *error = 1;
        }
        break;
    }
    case DRANK:
    {
        sprintf(tmp, "%ls", Token->Tokens[0]->Data);
         Var* v = isInTable(table, tmp);
         if(v != 0)
         {
             if(v->type)
             {
                 //printf("dec T%d\n", v->reg);
                 if(!v->assigned)
                    printf("Warning at line %ld column %ld.\n  Variable '%s' has not been assigned a value yet.\n", Token->Line, Token->Column, v->name);
                 addNode(cfg, Dec, v->reg, 0, -1, 0);
             }
             else
             {
                 printf("Error at line %ld column %ld.\n  Illegal operation on letter '%s'.\n", Token->Line, Token->Column, v->name);
                 *error = 1;
             }
         }
         else
         {
             printf("Error at line %ld column %ld.\n  Variable '%ls' has not been declared.\n", Token->Line, Token->Column, Token->Tokens[0]->Data);
             *error = 1;
         }
         break;
    }
    case ATE:
    {
        sprintf(tmp, "%ls", Token->Tokens[0]->Data);
         Var* v = isInTable(table, tmp);
         if(v != 0)
         {
             if(v->type)
             {
                 //printf("inc T%d\n", v->reg);
                 if(!v->assigned)
                    printf("Warning at line %ld column %ld.\n  Variable '%s' has not been assigned a value yet.\n", Token->Line, Token->Column, v->name);
                 addNode(cfg, Inc, v->reg, 0, -1, 0);       
             }
             else
             {
                 printf("Error at line %ld column %ld.\n  Illegal operation on letter '%s'.\n", Token->Line, Token->Column, v->name);
                 *error = 1;
             }
         }
         else
         {
             printf("Error at line %ld column %ld.\n  Variable '%ls' has not been declared.\n", Token->Line, Token->Column, Token->Tokens[0]->Data);
             *error = 1;
         }
         break;
    }
    case RETURN:
    {
        parseExp(Token->Tokens[2], cfg, table, 0, *regCount+1, error);
        //printf("mov T0 T%d\n", *regCount+1);
        addNode(cfg, Mov, 0, *regCount+1, 1, 0);
        break;
    }
    }
    
    if(*regCount > cfg->maxReg)
	cfg->maxReg = *regCount;
    
    for(i = 0; i < Grammar.RuleArray[Token->ReductionRule].SymbolsCount; i++)
    {
        parseTree(Token->Tokens[i], cfg, table, regCount, error);
    }
}

/* ---------------------------------------------------------------------------*/
// Optimisation of CFG

unsigned int pow2n(int n)
{
    unsigned int p = 1;
    int i;
    for(i = 0; i < n; i++)
    {
        p *= 2;
    }
    return p;
}

int optimiseTrans(CFG * cfg)
{
    int maxReg = 0;
    Node * l1 = cfg->first;
    Node * l2 = l1->next;
    
    while(l2 != 0)
    {
        if(l2->cmd != Not && l2->cmd != Dec && l2->cmd != Inc)
        {
            if(l1->cmd == Mov && (l2->cmd != Div && l2->cmd != Mod && l2->cmd != Mul) && 
               l2->srcReg == 1 && l1->dest == l2->src && !l1->varAssign)
            {
                l1->cmd = l2->cmd;
                l1->dest = l2->dest;
                l1->next = l2->next;
                l1->varAssign = l2->varAssign;
                free(l2);
                cfg->size--;
                l2 = l1->next;
                continue;
            }
        }
        l1 = l2;
        l2 = l2->next;
    }
    
    unsigned int used = 0;
    l1 = cfg->first;
    while(l1 != 0)
    {
        used = used | (unsigned int) pow2n(l1->dest);
        if(l1->srcReg == 1)
            used = used | (unsigned int) pow2n(l1->src);
        l1 = l1->next;
    }
    
    int i;
    unsigned int comp = 0;
    for(i=31; i>0; i--)
    {
        comp = (unsigned int) pow2n(i);
        if((comp & used) == 0)
        {
            l1 = cfg->first;
            while(l1 != 0)
            {
                if(i < l1->dest)
                    l1->dest -=1;
                if(l1->srcReg == 1 && i < l1->src)
                    l1->src -=1;
                l1 = l1->next;
            }
        }
    }

    l1 = cfg->first;
    while(l1 != 0)
    {
        if(l1->dest > maxReg)
            maxReg = l1->dest;
        l1 = l1->next;
    }
    return maxReg;
}

int calcMax(CFG * cfg)
{
    int size = 0;
    int maxReg = 0;
    Node * l = cfg->first;
    while(l != 0)
    {
        size++;
        if(l->dest > maxReg)
            maxReg = l->dest;
        l = l->next;
    }
    cfg->size = size;
    cfg->maxReg = maxReg;
    return maxReg;
}

int calcLives(CFG * cfg)
{
    int i = 0;
    int d, n, maxReg;
    Node * nodes[cfg->size];
    nodes[i] = cfg->first;
    if(nodes[i]->liveIn) vFree(nodes[i]->liveIn);
    if(nodes[i]->liveOut) vFree(nodes[i]->liveOut);
    nodes[i]->liveIn = vNew();
    nodes[i]->liveOut = vNew();
    
    maxReg = 0;
    while(nodes[i]->next != 0)
    {
        if(maxReg < nodes[i]->dest)
            maxReg = nodes[i]->dest;
        if(nodes[i]->srcReg == 1 && maxReg < nodes[i]->src)
            maxReg = nodes[i]->src;
            
        nodes[i+1] = nodes[i]->next;
        i++;
        
        if(nodes[i]->liveIn) vFree(nodes[i]->liveIn);
        if(nodes[i]->liveOut) vFree(nodes[i]->liveOut);
        nodes[i]->liveIn = vNew();
        nodes[i]->liveOut = vNew();
    }
    
    int change, temp;
    do
    {
        change = 0;
        for(i = cfg->size-1; i >= 0; i--)
        //for(i = 0; i < cfg->size; i++)
        {
            // LiveIn
            if(nodes[i]->cmd != Mov)
            {
                temp = (vAdd(nodes[i]->liveIn, nodes[i]->dest) == 1);
                change = change || temp;
            }
            
            if(nodes[i]->srcReg == 1)
            {
                temp = (vAdd(nodes[i]->liveIn, nodes[i]->src) == 1);
                change = change || temp;
            }
            
            for(n = 0; (d = vGet(nodes[i]->liveOut, n)) > -1; n++)
            {
                if(nodes[i]->cmd == Mov && nodes[i]->dest == d)
                    continue;
                temp = (vAdd(nodes[i]->liveIn, d) == 1);
                change = change || temp;
            }
            
            if(i+1 >= cfg->size)
                continue;
                
            // LiveOut
            for(n = 0; (d = vGet(nodes[i+1]->liveIn, n)) > -1; n++)
            {
                temp = (vAdd(nodes[i]->liveOut, d) == 1);
                change = change || temp;
            }
        }
    } while(change);
    
    return maxReg;
}



short int ** initGraph(int size)
{
    short int ** g;
    int i;
    g = malloc(size*sizeof(short int *));
    for(i = 0; i < size; i++)
        g[i] = malloc(size*sizeof(short int));
    return g;
}

short int ** calcInterference(CFG * cfg, int totalRegs)
{
    int regs = totalRegs;
    int d, e, i, j;
    
    //short int graph[regs][regs];
    short int ** iGraph = initGraph(regs);
    
    for(i = 0; i < (regs) * (regs); i++)
    {
        if(i/regs == i%regs)
            iGraph[i/regs][i%regs] = 1;
        else
            iGraph[i/regs][i%regs] = 0;
    }
    
    struct CFGNode * l = cfg->first;
    while(l != 0)
    {
        for(i = 0; (d = vGet(l->liveOut, i)) > -1; i++)
        {
            for(j = 0; (e = vGet(l->liveOut, j)) > -1; j++)
            {
                iGraph[d][e] = 1;
                iGraph[e][d] = 1;
            }
        }
        l = l->next;
    }
    return iGraph;
}

void printInterference(short int ** iGraph, int totalRegs)
{
    int i, d, size = totalRegs;
    for(i = -1; i < size; i++)
    {
        if(i < 0)
            printf("    ");
        else
            printf("T%d: ",i);
        
        for(d = 0; d < size; d++)
        {
            if(i < 0)
                printf("T%d ",d);
            else
                printf("%2d ",iGraph[i][d]);
        }
        printf("\n");
    }
}

void applyTranslations(CFG * cfg, int * translations)
{
     int i;
     Node * current = cfg->first;
     for (i = 0; i < cfg->size; i++)
     {
         //printf("Line %d:\n", i);
         //printf("Translated %d to %d in destinations\n", current->dest, translations[current->dest]);
         current->dest = translations[current->dest];
         if (current->srcReg == 1)
         {
             //printf("Translated %d to %d in sources\n", current->src, translations[current->dest]);
             current->src = translations[current->src];
         }
         current = current->next;
     }
}

int findNextMove(int current, int start, short int ** iGraph, int size, int * translations, int prev)
{
   int i;
   short int found = -1;
   for (i = start; i < size; i++)
   {
       if (i != current && i != prev && iGraph[current][i])
       {
           found = 1;   
           break; 
       } 
   }
     
   if (found == 1)
       return i;

   return found;
}

int findMostConflicting(int temp1, int temp2, Vector * conflicts)
{
    int conflict1 = 0;
    int conflict2 = 0;
    int i;
    Vector_n * current = conflicts->first;
    for (i = 0; i < conflicts->size; i++)
    {
        if (current->val == temp1 || current->val2 == temp1) conflict1++;
        if (current->val == temp2 || current->val2 == temp2) conflict2++;
        current = current->next;
    }
    
    if (conflict1 >= conflict2) return 1;
    else return 0;
}

int nextAvailableRegister(int current, short int ** iGraph, int * translations, int size)
{
    int candidateReg = 1;
    int i, change;
    change = 1;
    while (change)
    {
        change = 0;
        for (i = 1; i < size; i++)
        {
            if(iGraph[current][i] && candidateReg == translations[i])
            {
                candidateReg++;
                change = 1;
                break;
            }
        }
    }
    
    return candidateReg;
}


void resolveConflicts(short int ** iGraph, int * translations, Vector * conflicts, int size)
{
     int i;
     int confSize = conflicts->size;
     int mostConflicting;
     Vector_n * current = conflicts->first;
     for (i = 0; i < confSize; i++)
     {
         if (translations[current->val] == translations[current->val2])
         {
             mostConflicting = findMostConflicting(current->val, current->val2, conflicts);
             if (mostConflicting){
                  translations[current->val] = nextAvailableRegister(current->val, iGraph, translations, size); 
             }
             else {
                  translations[current->val2] = nextAvailableRegister(current->val2, iGraph, translations, size); 
             }      
         }
         current = current->next;    
     }
}

void colorGraph(int current, short int ** iGraph, int * translations, Vector * conflicts, short int regColor, int size, int prev)
{
    translations[current] = regColor;
    if (regColor == 1) regColor = 2;
    else regColor = 1;
    short int availableMovesLeft = 1;
    int nextMove;
    int start = 1;
    while (availableMovesLeft)
    {
        nextMove = findNextMove(current, start, iGraph, size, translations, prev);
        if (nextMove > -1)
        {
            if (translations[nextMove] > -1)
            {
                vAddEdge(conflicts, current, nextMove);        
            } 
            else {
                 colorGraph(nextMove, iGraph, translations, conflicts, regColor, size, current);
            }
            start = nextMove + 1;   
        } 
        else availableMovesLeft = 0;
    }
}

void reduceCFGRegisters(short int ** iGraph, CFG * cfg, int usedRegs)
{
     int i;
     int * translations = malloc(sizeof(int) * usedRegs);
     Vector * conflicts = malloc(sizeof(Vector));
     for (i = 0; i < usedRegs; i++) translations[i] = -1;
     //colorGraph(1, iGraph, translations, conflicts, 1, usedRegs, 0);

     for (i = 1; i < usedRegs; i++) if (translations[i] == -1) colorGraph(i,iGraph,translations,conflicts,1,usedRegs,0);
     translations[0] = 0;
     //for (i = 0; i < usedRegs; i++) printf("Translation: T%d to R%d\n", i, translations[i]);
     resolveConflicts(iGraph, translations, conflicts, usedRegs);
     applyTranslations(cfg, translations);
}

/* ---------------------------------------------------------------------------*/
// Public functions

CFG * makeCFG(struct TokenStruct *Token, int optims, int * error)
{
    int varRegs = 0;
    int usedRegs = 0;
    int debug = *error;
    *error = 0;
    CFG * cfg = malloc(sizeof(CFG));
    cfg->first = 0;
    cfg->current = 0;
    cfg->size = 0;
    cfg->maxReg = 0;
    VTable * table = malloc(sizeof(VTable));
    table->first = 0;
    table->current = 0;
    table->size = 0;
    
    parseTree(Token, cfg, table, &varRegs, error);
    free(table);
    
    if(*error)
    {
        free(cfg);
        return 0;
    }
    
    if(debug)
    {
        printf("Control flow graph:\n---\n");
        printCFG(cfg);
        printf("---\n\n");
    }
    if(optims > 0)
    {
        usedRegs = optimiseTrans(cfg) + 1;
        cfg->maxReg = usedRegs - 1;
        if(debug)
        {
            printf("Optimised graph (Pass 1):\n---\n");
            printCFG(cfg);
            printf("---\n\n");
        }
    }
    if(optims > 1)
    {
        usedRegs = calcLives(cfg) + 1;
        if(usedRegs != cfg->maxReg + 1)
        {
            printf("Error: Conflicting max register counts...\n");
        }
        short int ** interference = calcInterference(cfg, usedRegs);
        if(debug)
        {
            printf("CFG with live ranges:\n---\n");
            printCFG(cfg);
            printf("---\n\nInterference matrix:\n---\n");
            printInterference(interference, usedRegs);
            printf("---\n\n");
        }
        reduceCFGRegisters(interference, cfg, usedRegs);
        usedRegs = calcLives(cfg) + 1;
        calcMax(cfg);
        if(debug)
        {
            printf("After Graph Coloring:\n---\n");
            printCFG(cfg);
            printf("---\n\n");
        }
    }
    
    return cfg;
}

void printCFG(CFG * cfg)
{
    int d, i;
    char tmp[256];
    Node * l = cfg->first;
    while(l != 0)
    {
        switch(l->cmd)
        {
            case Add: printf("add"); break;
            case Sub: printf("sub"); break;
            case Mod: printf("mod"); break;
            case Mul: printf("mul"); break;
            case Div: printf("div"); break;
            case And: printf("and"); break;
            case Or:  printf("or ");  break;
            case Xor: printf("xor"); break;
            case Mov: printf("mov"); break;
            case Not: printf("not"); break;
            case Dec: printf("dec"); break;
            case Inc: printf("inc"); break;
            default: break;
        }
        if(l->cmd != Not && l->cmd != Dec && l->cmd != Inc)
        {
            printf(" T%d", l->dest);
            if(l->srcReg)
                printf(" T%d", l->src);
            else
                printf(" %d", l->src);
        }
        else
        {
            printf(" T%d\t", l->dest);
        }
        
        if(l->liveIn != 0)
        {
            printf("\tLiveIns = ");
            sprintf(tmp,"{");
            for(i = 0; (d = vGet(l->liveIn, i)) > -1; i++)
            {
                sprintf(tmp,"%s%d",tmp,d);
                if(vGet(l->liveIn, i+1) > -1)
                    sprintf(tmp,"%s,",tmp);
            }
            sprintf(tmp,"%s}",tmp);
            printf("%-12s LiveOuts = {",tmp);
            for(i = 0; (d = vGet(l->liveOut, i)) > -1; i++)
            {
                printf("%d",d);
                if(vGet(l->liveOut, i+1) > -1)
                    printf(",");
            }
            printf("}");
        }
        
        printf("\n");
        l = l->next;
    }
}
