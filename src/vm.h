#ifndef MY_LOX_VM_H
#define MY_LOX_VM_H
#include "array.h"
#include "map.h"

void init_data();

uint32_t write_constant(value_t *data);
uint32_t write_code(uint8_t op_code);
uint32_t write_data4(uint32_t data);
void write_jump(uint32_t offset);

value_t *read_constant(uint32_t offset);
uint8_t read_code(uint32_t offset);
uint32_t read_data4(uint32_t offset);
bool has_more(uint32_t offset);

uint32_t get_next();
uint8_t back_code();

value_t *peek_value();

void run_code();

#endif
