#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "common.h"

stack0_t *new_stack(uint32_t size){
    stack0_t *stack= malloc(sizeof(stack0_t));
    stack->data= malloc(size);
    stack->size=size;
    stack->top=0;
    return stack;
}

void stack_push(stack0_t *stack,void *data,uint32_t size){
    if(stack->top+size>stack->size){
        panic("stack push overflow size %d",size);
    }
    memcpy(stack->data+stack->top,data,size);
    stack->top+=size;
}

void *stack_pop(stack0_t *stack,uint32_t size){
    if(stack->top<size){
        panic("stack pop overflow size %d",size);
    }
    stack->top-=size;
    return stack->data+stack->top;
}

void *stack_peek(stack0_t *stack,uint32_t size){
    if(stack->top<size){
        panic("stack peek overflow size %d",size);
    }
    return stack->data+stack->top-size;
}

void *stack_at(stack0_t *stack,uint32_t offset){
    return stack->data+offset;
}

void stack_push4(stack0_t *stack,uint32_t data){
    stack_push(stack,&data,4);
}

uint32_t stack_pop4(stack0_t *stack){
    uint32_t *data= stack_pop(stack,4);
    return *data;
}

uint32_t stack_at4(stack0_t *stack,uint32_t offset){
    uint32_t *data= stack_at(stack,offset);
    return *data;
}