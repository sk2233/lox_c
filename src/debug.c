#include <printf.h>
#include "debug.h"
#include "vm.h"
#include "value.h"

void disassemble_code(){
    uint32_t offset=0;
    while (has_more(offset)){
        offset= disassemble_instruction(offset);
    }
}

uint32_t simple_instruction(const char *name, uint32_t offset) {
    printf("0x%08x %s\n",offset,name);
    return offset+1;
}

// 只能展示有一个操作数的指令
uint32_t stack_instruction(char *name, uint32_t offset) {
    value_t *val= peek_value();
    printf("0x%08x %s ",offset,name);
    print_value(val);
    printf("\n");
    return offset+1;
}

uint32_t constant_instruction(uint32_t offset) {
    uint32_t addr= read_data4(offset+1);
    value_t *val= read_constant(addr);
    printf("0x%08x CONSTANT 0x%08x = ",offset,addr);
    print_value(val);
    printf("\n");
    return offset+5;
}

uint32_t jump_instruction(char *name,uint32_t offset) {
    uint32_t ip= read_data4(offset+1);
    printf("0x%08x %s 0x%08x\n",offset,name,ip);
    return offset+5;
}

uint32_t disassemble_instruction(uint32_t offset){
    uint8_t op_code = read_code(offset);
    switch (op_code) {
        case OP_RETURN:
            return stack_instruction("OP_RETURN",offset);
        case OP_CONSTANT:
            return constant_instruction(offset);
        case OP_NOT:
            return stack_instruction("OP_NOT",offset);
        case OP_ADD:
            return simple_instruction("OP_ADD",offset);
        case OP_SUB:
            return simple_instruction("OP_SUB",offset);
        case OP_MUL:
            return simple_instruction("OP_MUL",offset);
        case OP_DIV:
            return simple_instruction("OP_DIV",offset);
        case OP_NOP:
            return simple_instruction("OP_NOP",offset);
        case OP_GT:
            return simple_instruction("OP_GT",offset);
        case OP_LT:
            return simple_instruction("OP_LT",offset);
        case OP_EQ:
            return simple_instruction("OP_EQ",offset);
        case OP_PRINT:
            return stack_instruction("OP_PRINT",offset);
        case OP_POP:
            return stack_instruction("OP_POP",offset);
        case OP_ASSIGN:
            return simple_instruction("OP_ASSIGN",offset);
        case OP_GET:
            return stack_instruction("OP_GET",offset);
        case OP_SET:
            return simple_instruction("OP_SET",offset);
        case OP_BEGIN_SCOPE:
            return simple_instruction("OP_BEGIN_SCOPE",offset);
        case OP_END_SCOPE:
            return simple_instruction("OP_END_SCOPE",offset);
        case OP_JUMP_FALSE:
            return jump_instruction("OP_JUMP_FALSE",offset);
        case OP_JUMP:
            return jump_instruction("OP_JUMP",offset);
        case OP_JUMP_PEEK_TRUE:
            return jump_instruction("OP_JUMP_PEEK_TRUE",offset);
        case OP_JUMP_PEEK_FALSE:
            return jump_instruction("OP_JUMP_PEEK_FALSE",offset);
        case OP_AND:
            return simple_instruction("OP_AND",offset);
        case OP_OR:
            return simple_instruction("OP_OR",offset);
        case OP_SWAP:
            return simple_instruction("OP_SWAP",offset);
        case OP_CALL:
            return simple_instruction("OP_CALL",offset);
        case OP_CLASS:
            return simple_instruction("OP_CLASS",offset);
        case OP_FUNC:
            return simple_instruction("OP_FUNC",offset);
        case OP_GET_FIELD:
            return simple_instruction("OP_GET_FIELD",offset);
        case OP_SET_FIELD:
            return simple_instruction("OP_SET_FIELD",offset);
        case OP_METHOD:
            return simple_instruction("OP_METHOD",offset);
        case OP_SUPER_METHOD:
            return stack_instruction("OP_SUPER_METHOD",offset);
        default:
            printf("unknown opcode %d\n", op_code);
            return offset+1;
    }
}
