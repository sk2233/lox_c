#ifndef MY_LOX_TYPE_H
#define MY_LOX_TYPE_H
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    OP_NOP,  // 空操作指令
    OP_RETURN, // 无操作数
    OP_CONSTANT,   // 一个uint32操作数
    OP_NOT, // 数字 加 -   布尔值 加 !
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ, // OP_NE = OP_NOT OP_EQ
    OP_GT, // OP_GE = OP_NOT OP_LT
    OP_LT, // OP_LE = OP_NOT OP_GT
    OP_PRINT,
    OP_POP,
    OP_ASSIGN,
    OP_GET,
    OP_SET,
    OP_BEGIN_SCOPE,
    OP_END_SCOPE,
    OP_JUMP,
    OP_JUMP_FALSE,
    OP_JUMP_PEEK_FALSE,
    OP_JUMP_PEEK_TRUE,
    OP_AND,
    OP_OR,
    OP_FUNC, // 定义函数
    OP_SWAP, // 交换栈顶的两个数据
    OP_CALL,
    OP_CLASS,
    OP_GET_FIELD,
    OP_SET_FIELD,
    OP_METHOD,
    OP_SUPER_METHOD,
} op_code;

typedef enum {
    VT_NIL,
    VT_NUM,
    VT_BOOL,
    VT_STR, // 字符串 内容存储在 指针中
    VT_FUNC, // 函数  存储在指针中
    VT_NATIVE, // 本地方法类型
    VT_CLASS, // 类型
    VT_INSTANCE, // 实例
    VT_PTR,  // 指针类型，只用于内部使用
}value_type;

typedef struct {
    value_type type;
    union {
        bool bool0;
        int32_t num;
        void *ptr;
    };
}value_t;

#endif
