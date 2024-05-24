#include <printf.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

void panic(char *format, ...){
    printf("panic happen\n");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(-1);
}

void *clone(void *data,uint32_t size){
    void *res= malloc(size);
    memcpy(res,data,size);
    return res;
}