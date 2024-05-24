#include "array.h"
#include <string.h>
#include <stdlib.h>

array_t *new_array(uint32_t size){
    array_t *res= malloc(sizeof(array_t));
    res->data= malloc(size);
    res->capacity=size;
    res->count=0;
    return res;
}

uint32_t array_write(array_t *array, void *data,uint32_t size){
    if(array->count+size>array->capacity){
        array->capacity*=2;
        uint8_t *temp=malloc(array->capacity);
        memcpy(temp, array->data, array->count);
        free(array->data);
        array->data=temp;
    }
    memcpy(array->data+array->count,data,size);
    array->count+=size;
    return array->count-size;
}

void array_write_at(array_t *array,uint32_t offset,void *data,uint32_t size){
    memcpy(array->data+offset,data,size);
}

void *array_at(array_t *array,uint32_t offset){
    return array->data+offset;
}

void array_free(array_t *array){
    free(array->data);
    free(array);
}

uint32_t array_write1(array_t *array,uint8_t data){
    return array_write(array,&data,1);
}

uint32_t array_write4(array_t *array,uint32_t data){
    return array_write(array,&data,4);
}

uint8_t array_at1(array_t *array,uint32_t offset){
    uint8_t *data= array_at(array,offset);
    return *data;
}

uint32_t array_at4(array_t *array,uint32_t offset){
    uint32_t *data= array_at(array,offset);
    return *data;
}