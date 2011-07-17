#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembly.h"

//esp-(4*(no of reg-6))

char* getRegister(int regNumber)
{
      char reg[24];
      switch(regNumber)
      {
          case 1:
               sprintf(reg,"eax");
               break;
          case 0:
          case 2:
               sprintf(reg,"ebx");
               break;
          case 3:
               sprintf(reg,"ecx");
               break;
          case 4:
               sprintf(reg,"edx");
               break;
          case 5:
               sprintf(reg,"edi");
               break;
          //case 6:
          //     sprintf(reg,"esi");
          //     break;
          default:
	      sprintf(reg,"[ebp-%d]",((regNumber - 5)) * 4 );
	      //sprintf(reg,"TR%d",regNumber);
              break;              
      }
      char* ret = malloc(sizeof(char)*strlen(reg));
      strcpy(ret,reg);
      return ret;
}

void writeNode(Node * node, FILE* file)
{
     char src[24];
     char dest[24];
     int usingEsi = 0;
     
     if(node->dest > 5 && node->srcReg == 1 && node->src > 5)
     {
          fprintf(file, "mov esi, %s\n", getRegister(node->src));
          usingEsi = 1;
     }
     
     switch (node->cmd)
     {
         case Add:
              fprintf(file, "add ");
              break;
         case Sub:
              fprintf(file, "sub ");
              break;
         case Mod:
              fprintf(file, "mov [memD] , edx\n");
              fprintf(file, "mov [memA] , eax\n");
              
              sprintf(src, "%s", getRegister(node->src));
              sprintf(dest, "%s", getRegister(node->dest));
              
	      //The source was eax
	      if(node->src == 1)
                  sprintf(src, "dword [memA]");
	      //The destination was edx
              if(node->dest == 4)
                  sprintf(dest, "dword [memD]");
	      //The source was edx
              if(node->src == 4)
                  sprintf(src, "dword [memD]");
	      //The destination was eax
              if(node->dest == 1)
                  sprintf(dest, "dword [memA]");
                  
              fprintf(file, "mov edx , 0\n");
              fprintf(file, "mov eax , %s\n", dest);

              if(node->src > 5)
                  fprintf(file, "div dword %s\n", src);
              else
                  fprintf(file, "div %s\n", src);

              fprintf(file, "mov %s , edx\n", dest);
              fprintf(file, "mov eax , [memA]\n");
              fprintf(file, "mov edx , [memD]\n");
              return;
              break;
         case Mul:
	      fprintf(file, "mov [memD] , edx\n");
              fprintf(file, "mov [memA] , eax\n");
              
              sprintf(src, "%s", getRegister(node->src));
              sprintf(dest, "%s", getRegister(node->dest));
              
	      //The source was eax
	      if(node->src == 1)
                  sprintf(src, "dword [memA]");
	      //The destination was edx
              if(node->dest == 4)
                  sprintf(dest, "dword [memD]");
	      //The source was edx
              if(node->src == 4)
                  sprintf(src, "dword [memD]");
	      //The destination was eax
              if(node->dest == 1)
                  sprintf(dest, "dword [memA]");
                  
              fprintf(file, "mov eax , %s\n", dest);

              if(node->src > 5)
                  fprintf(file, "imul dword %s\n", src);
              else
                  fprintf(file, "imul %s\n", src);

              fprintf(file, "mov %s , eax\n", dest);
              fprintf(file, "mov eax , [memA]\n");
              fprintf(file, "mov edx , [memD]\n");
              return;
              break;
         case Div:
              fprintf(file, "mov [memD] , edx\n");
              fprintf(file, "mov [memA] , eax\n");
              
              sprintf(src, "%s", getRegister(node->src));
              sprintf(dest, "%s", getRegister(node->dest));
              
	      //The source was eax
	      if(node->src == 1)
                  sprintf(src, "dword [memA]");
	      //The destination was edx
              if(node->dest == 4)
                  sprintf(dest, "dword [memD]");
	      //The source was edx
              if(node->src == 4)
                  sprintf(src, "dword [memD]");
	      //The destination was eax
              if(node->dest == 1)
                  sprintf(dest, "dword [memA]");
                  
              fprintf(file, "mov edx , 0\n");
              fprintf(file, "mov eax , %s\n", dest);

              if(node->src > 5)
                  fprintf(file, "div dword %s\n", src);
              else
                  fprintf(file, "div %s\n", src);

              fprintf(file, "mov %s , eax\n", dest);
              fprintf(file, "mov eax , [memA]\n");
              fprintf(file, "mov edx , [memD]\n");
              return;
              break;
         case And:
              fprintf(file, "and ");
              break;
         case Or:
              fprintf(file, "or  ");
              break;
         case Xor:
              fprintf(file, "xor ");
              break;
         case Mov:
              fprintf(file, "mov ");
              break;
         case Not:
              fprintf(file, "not ");
              break;
         case Dec:
              fprintf(file, "dec ");
              break;
         case Inc: 
              fprintf(file, "inc ");
              break;
         default:
              printf("Something's gone wrong...\n");
              break;     
     }

     if(node->dest > 5)
         fprintf(file,"dword %s ", getRegister(node->dest));
     else
         fprintf(file, "%s ",getRegister(node->dest));

     if (node->srcReg == 1 && !usingEsi) fprintf(file, ", %s\n", getRegister(node->src));
     else if (node->srcReg == 1 && usingEsi) fprintf(file, ", esi\n");
     else if (node->srcReg == 0) fprintf(file, ", %d\n", node->src);
     else fprintf(file, "\n");
}

char* asmName(char* aname)
{
    char fname[48];
    char* t;
    strcpy(fname, aname);
    t = strchr(fname, '.');
    strcpy(t, ".s");
    char* rname = malloc(strlen(fname) * sizeof(char));
    strcpy(rname, fname);
    return rname;
}

char* exeName(char* aname)
{
    char fname[48];
    char* t;
    strcpy(fname, aname);
    t = strchr(fname, '.');
    strcpy(t, "\0");
    char* rname = malloc(strlen(fname) * sizeof(char));
    strcpy(rname, fname);
    return rname;
}

void printFile(char* fname)
{
    FILE *file; 
    if ((file = fopen(fname,"r")) == NULL)
    {
        printf("Error in opening file.\n");
        return;          
    }
    char temp[256];
    fgets(temp, 1024, file);
    while(!feof(file))
    {
        printf("%s",temp);
        fgets(temp, 1024, file);
    }
    fclose(file);
}

void genCode(CFG * cfg, char* fname)
{
     FILE *file; 
     if ((file = fopen(fname,"w")) == NULL) {
         printf("Error in writing file.\n");
         return;          
     }
    
     fprintf(file,"section .data ;declare section\n");
     fprintf(file,"memA DD 0\n");
     fprintf(file,"memD DD 0\n");
     fprintf(file,"section .text\n");
     fprintf(file,"global main\nmain:\n");
     
     int i = (cfg->maxReg - 5)*4;
     if(i > 0)
     {
	 //Using the stack 
         fprintf(file, "push ebp\n");
	 fprintf(file, "mov esp , ebp\n");
	 
	 fprintf(file, "sub esp , %d\n", i);
     }
	 

     Node * node = cfg->first;
     while(node != 0)
     {
         writeNode(node, file);
         node = node->next;    
     }
     
     if(i > 0)
     {
         //Putting the stack back where it was!
	 fprintf(file,"mov ebp , esp\n");
	 fprintf(file,"pop ebp\n");
	 //End of stack support
     }

     fprintf(file,"exit_program:\nmov eax, 1 ;Linux system call to return\n");
     fprintf(file," int 0x80\n");
     
     fclose(file);  
}
