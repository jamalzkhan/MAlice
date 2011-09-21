#include <stdlib.h>
#include "vector.h"

Vector * vNew()
{
    Vector * v = malloc(sizeof(Vector));
    v->first = 0;
    v->size = 0;
    return v;
}

void vFreeN(Vector_n * n)
{
    if(n == 0) return;
    if(n->next)
        vFreeN(n->next);
    free(n);
}

void vFree(Vector * v)
{
    if(v == 0) return;
    vFreeN(v->first);
    free(v);
}

int vAdd(Vector * v, int d)
{
    if(v == 0) // Null pointer
        return -1;
        
    Vector_n * node = malloc(sizeof(Vector_n));
    node->val = d;
    node->val2 = 0;
    node->next = 0;
    if(v->size == 0)
    {
        v->first = node;
    }
    else
    {
        Vector_n * t = v->first;
        int i;
        for(i = 0; i < v->size; i++)
        {
            if(t->val == d)
            {
                free(node);
                return 0;
            }
            if(t->next == 0)
            {
                t->next = node;
                break;
            }
            t = t->next;
        }
    }
    v->size++;
    return 1;
}

int vAddEdge(Vector * v, int d, int d2)
{
    if(v == 0) // Null pointer
        return -1;
        
    Vector_n * node = malloc(sizeof(Vector_n));
    node->val = d;
    node->val2 = d2;
    node->next = 0;
    if(v->size == 0)
    {
        v->first = node;
    }
    else
    {
        Vector_n * t = v->first;
        while(t != 0 && t->next != 0)
        {
            if(d == t->val && d2 == t->val2)
            {
                free(node);
                return 0;
            }
            t = t->next;
        }
/*
        if(d == t->val)
        {
            free(node);
            return 0;
        }
*/
        t->next = node;
    }
    v->size++;
    return 1;
}

int vDel(Vector * v, int d)
{
    if(v == 0) // Null pointer
        return -1;
        
    Vector_n * t = v->first;
    Vector_n * n;
    
    if(t->val == d)
    {
        v->first = t->next;
        free(t);
        v->size--;
        return 1;
    }
    
    while(t != 0)
    {
        if(t->next != 0 && t->next->val == d)
        {
            n = t->next->next;
            free(t->next);
            t->next = n;
            v->size--;
            return 1;
        }
        t = t->next;
    }
    return 0;
}

int vGet(Vector * v, int index)
{
    if(v == 0) // Null pointer
        return -1;
    if(index >= v->size) // Index out of bounds
        return -1;
    
    Vector_n * t = v->first;
    int i;
    for(i = 0; i < v->size; i++)
    {
        if(i == index)
        {
            return t->val;
        }
        t = t->next;
    }
    
    return -1; //Something went wrong...
}
