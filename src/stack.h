#ifndef MY_LOX_STACK_H
#define MY_LOX_STACK_H
#include "type.h"

typedef struct {
    void *data;
    uint32_t top,size;
}stack0_t;

stack0_t *new_stack(uint32_t size);
void stack_push(stack0_t *stack,void *data,uint32_t size);
void *stack_pop(stack0_t *stack,uint32_t size);
void *stack_peek(stack0_t *stack,uint32_t size);
void *stack_at(stack0_t *stack,uint32_t offset);

void stack_push4(stack0_t *stack,uint32_t data);
uint32_t stack_pop4(stack0_t *stack);
uint32_t stack_at4(stack0_t *stack,uint32_t offset);

#endif
