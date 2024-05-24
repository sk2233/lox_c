#ifndef MY_LOX_PARSER_H
#define MY_LOX_PARSER_H
#include "type.h"
#include "array.h"

#define KEY_WORD_SIZE 16

typedef enum {
    TK_LEFT, TK_RIGHT, // ( )
    TK_LEFT2, TK_RIGHT2, // { }
    TK_ADD, TK_SUB, TK_MUL, TK_DIV, // + - * /
    TK_COMMA, TK_DOT, TK_SEMI,  // , . ;
    TK_NOT, // -
    TK_ASSIGN, // =
    TK_GT, TK_GE, TK_LT, TK_LE, TK_EQ, TK_NE,  // > >= < <= == !=
    TK_ID, TK_STR, TK_NUM, TK_NIL, // id str num nil
    TK_IF, TK_ELSE, // if else
    TK_AND, TK_OR, // and or
    TK_TRUE, TK_FALSE, // true false
    TK_CLASS, TK_FUNC, TK_RETURN, // class func return
    TK_SUPER, TK_THIS, // super this
    TK_FOR, TK_WHILE, // for while
    TK_PRINT, // print
    TK_VAR, // var
    TK_EOF //  eof
} token_type;

typedef struct {
    token_type type;
    char *start;
    uint32_t len;
} token_t;

typedef struct {
    token_type type;
    char name[8];
}key_word_t;

array_t *parse_token(char *source);

#endif
