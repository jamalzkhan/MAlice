#ifndef VECTOR_H
#define VECTOR_H

typedef struct vectorNode
{
    int val;
    int val2;
    struct vectorNode * next;
} Vector_n;

typedef struct vectorStruct
{
    struct vectorNode * first;
    int size;
} Vector;

Vector* vNew();
void nFree(Vector * v);
void vFree(Vector * v);
int vAdd(Vector * v, int d);
int vAddEdge(Vector * v, int d, int d2);
int vDel(Vector * v, int d);
int vGet(Vector * v, int index);

#endif
