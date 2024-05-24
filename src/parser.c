#include <stdlib.h>
#include "parser.h"
#include "array.h"
#include "common.h"

static char *left;
static char *right;
static key_word_t key_words[KEY_WORD_SIZE]={
        {.type=TK_NIL,.name="nil"},
        {.type=TK_IF,.name="if"},
        {.type=TK_ELSE,.name="else"},
        {.type=TK_AND,.name="and"},
        {.type=TK_OR,.name="or"},
        {.type=TK_TRUE,.name="true"},
        {.type=TK_FALSE,.name="false"},
        {.type=TK_CLASS,.name="class"},
        {.type=TK_FUNC,.name="func"},
        {.type=TK_RETURN,.name="return"},
        {.type=TK_SUPER,.name="super"},
//        {.type=TK_THIS,.name="this"},
        {.type=TK_FOR,.name="for"},
        {.type=TK_WHILE,.name="while"},
        {.type=TK_PRINT,.name="print"},
        {.type=TK_VAR,.name="var"},
};

token_t *new_token(token_type type){
    token_t *token= malloc(sizeof(token_t)); // 没有释放
    token->type=type;
    token->start=left;
    token->len=right-left;
    return token;
}

void add_token(array_t *tokens, token_t *token) {
    array_write(tokens,token, sizeof(token_t));
}

char read_char(){
    char res=*right;
    right++;
    return res;
}

char peek_char() {
    return *right;
}

bool match_char(char c) {
    if(*right==c){
        right++;
        return true;
    }
    return false;
}

bool is_digit(char c) {
    return c>='0'&&c<='9';
}

bool is_alpha(char c) {
    return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_';
}

bool match_key_word(token_t *token, key_word_t* key_word) {
    char *t=token->start;
    char *k=key_word->name;
    for (int i = 0; i < token->len; ++i) {
        if(*t++!=*k++){
            return false;
        }
    }
    return *k=='\0';
}

array_t *parse_token(char *source) {
    array_t *tokens= new_array(1024);
    left=source;
    right=source;
    bool run=true;
    while (run){
        char c= read_char();
        switch (c) {
            // 单字节
            case '(':
                add_token(tokens, new_token(TK_LEFT));
                break;
            case ')':
                add_token(tokens, new_token(TK_RIGHT));
                break;
            case '{':
                add_token(tokens, new_token(TK_LEFT2));
                break;
            case '}':
                add_token(tokens, new_token(TK_RIGHT2));
                break;
            case ';':
                add_token(tokens, new_token(TK_SEMI));
                break;
            case ',':
                add_token(tokens, new_token(TK_COMMA));
                break;
            case '.':
                add_token(tokens, new_token(TK_DOT));
                break;
            case '+':
                add_token(tokens, new_token(TK_ADD));
                break;
            case '-':
                add_token(tokens, new_token(TK_SUB));
                break;
            case '*':
                add_token(tokens, new_token(TK_MUL));
                break;
            case '/':
                add_token(tokens, new_token(TK_DIV));
                break;
            case '\0':
                add_token(tokens, new_token(TK_EOF));
                run=false;
                break;
                // 单双字节
            case '!':
                if(match_char('=')){
                    add_token(tokens, new_token(TK_NE));
                } else{
                    add_token(tokens, new_token(TK_NOT));
                }
                break;
            case '=':
                if(match_char('=')){
                    add_token(tokens, new_token(TK_EQ));
                } else{
                    add_token(tokens, new_token(TK_ASSIGN));
                }
                break;
            case '<':
                if(match_char('=')){
                    add_token(tokens, new_token(TK_LE));
                } else{
                    add_token(tokens, new_token(TK_LT));
                }
                break;
            case '>':
                if(match_char('=')){
                    add_token(tokens, new_token(TK_GE));
                } else{
                    add_token(tokens, new_token(TK_GT));
                }
                break;
                // 跳过空白字符
            case ' ':
            case '\t':
            case '\n':
                break;
                // str
            case '"':
                while (read_char() != '"'){}
                add_token(tokens, new_token(TK_STR));
                break;
            default:
                if(is_digit(c)){
                    while (is_digit(peek_char())){
                        right++;
                    }
                    add_token(tokens, new_token(TK_NUM));
                } else if(is_alpha(c)){
                    while (is_digit(peek_char()) || is_alpha(peek_char())){
                        right++;
                    }
                    token_t *token=new_token(TK_ID);// 默认类型是id
                    for (int i = 0; i < KEY_WORD_SIZE; ++i) {
                        key_word_t *key_word=key_words+i;
                        if(match_key_word(token,key_word)){
                            token->type=key_word->type;
                        }
                    }
                    add_token(tokens, token);
                } else{
                    panic("err char %c",c);
                }
                break;
        }
        left=right;  // 每次循环必须产生一个token
    }
    // 解析完毕
    return tokens;
}
