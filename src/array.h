#ifndef MY_LOX_ARRAY_H
#define MY_LOX_ARRAY_H
#include "type.h"

// 动态数组
typedef struct {
    void *data; // 起始位置
    uint32_t count,capacity; // 数量 容量
}array_t;

array_t *new_array(uint32_t size);
uint32_t array_write(array_t *array, void *data,uint32_t size);
void array_write_at(array_t *array,uint32_t offset,void *data,uint32_t size);
void *array_at(array_t *array,uint32_t offset);
void array_free(array_t *array);

uint32_t array_write1(array_t *array,uint8_t data);
uint32_t array_write4(array_t *array,uint32_t data);
uint8_t array_at1(array_t *array,uint32_t offset);
uint32_t array_at4(array_t *array,uint32_t offset);

#endif
