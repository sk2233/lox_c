#include "compile.h"
#include "parser.h"
#include "common.h"
#include "vm.h"
#include "value.h"
#include <string.h>

static uint32_t offset;

token_t *read_token(array_t *tokens){
    token_t *token= array_at(tokens,offset);
    offset+= sizeof(token_t);
    return token;
}

token_t *peek_token(array_t *tokens) {
    return array_at(tokens,offset);
}

bool match_token(array_t *tokens, token_type type) {
    token_t *token=peek_token(tokens);
    if(token->type!=type){
        return false;
    }
    offset+= sizeof(token_t);
    return true;
}

void must_match_token(array_t *tokens, token_type type) {
    if(!match_token(tokens,type)){
        panic("miss match token type %d",type);
    }
}

token_t *must_read_token(array_t *tokens, token_type type) {
    token_t *token= read_token(tokens);
    if(token->type!=type){
        panic("must_read_token err type %d",type);
    }
    return token;
}

int32_t to_num(token_t *token) {
    int32_t res=0;
    char *c=token->start;
    for (int i = 0; i < token->len; ++i) {
        res=res*10+*c-'0';
        c++;
    }
    return res;
}

void make_constant(value_t *val) {
    uint32_t addr= write_constant(val);
    write_code(OP_CONSTANT);
    write_data4(addr);
}

void primary(array_t *tokens) { // primary -> num | str | true | false | '(' Expression ')' | id | super.id
    if(match_token(tokens,TK_LEFT)){
        expression(tokens);
        must_match_token(tokens,TK_RIGHT);
    } else if(match_token(tokens,TK_TRUE)){
        make_constant(new_bool(true));
    }else if(match_token(tokens,TK_FALSE)){
        make_constant(new_bool(false));
    }else if(match_token(tokens,TK_NIL)){
        make_constant(new_nil());
    } else if(match_token(tokens,TK_SUPER)){
        must_match_token(tokens,TK_DOT);
        token_t *method= must_read_token(tokens,TK_ID);
        make_constant(new_str(method->start,method->len));
        write_code(OP_SUPER_METHOD);
    } else if(peek_token(tokens)->type==TK_STR){
        token_t *token=read_token(tokens);
        // 注意移除  "
        make_constant(new_str(token->start+1,token->len-2));
    }else if(peek_token(tokens)->type==TK_ID){
        token_t *token=read_token(tokens);
        make_constant(new_str(token->start,token->len));
        write_code(OP_GET);
    } else{
        token_t *token=must_read_token(tokens,TK_NUM);
        int32_t num=to_num(token);
        make_constant(new_num(num));
    }
}


void call(array_t *tokens) { // call -> primary ( ( args? ) | .id )*
    primary(tokens);// 先压入函数，再压入各个参数，再压入参数个数，再压入返回地址
    while (true){
        if(match_token(tokens,TK_LEFT)){
            int32_t args_size =0;
            if(!match_token(tokens,TK_RIGHT)){
                expression(tokens);
                args_size++;
                while (!match_token(tokens,TK_RIGHT)){
                    must_match_token(tokens,TK_COMMA);
                    expression(tokens);
                    args_size++;
                }
            }
            make_constant(new_num(args_size));
            uint32_t ip=get_next();
            make_constant(new_num((int32_t)ip+5+1)); // 压入常量 5 OP_CALL 1
            write_code(OP_CALL);
        } else if(match_token(tokens,TK_DOT)){
            token_t *field = must_read_token(tokens,TK_ID);
            make_constant(new_str(field->start,field->len));
            write_code(OP_GET_FIELD);
        } else{
            break;
        }
    }
}

void unary(array_t *tokens) { // unary -> (( - | ! )* call)
    if(match_token(tokens,TK_SUB)||match_token(tokens,TK_NOT)){
        call(tokens);
        write_code(OP_NOT); // 必须放在后面
    } else{
        call(tokens);
    }
}

void factor(array_t *tokens) { // factor -> unary (( / | * )unary)*
    unary(tokens);
    while (peek_token(tokens)->type==TK_DIV||peek_token(tokens)->type==TK_MUL){
        token_t *token= read_token(tokens);
        unary(tokens);
        switch (token->type) {
            case TK_DIV:
                write_code(OP_DIV);
                break;
            case TK_MUL:
                write_code(OP_MUL);
                break;
            default:
                panic("factor err token type %d",token->type);
                return;
        }
    }
}

void term(array_t *tokens) {// factor ( +|- factor )*
    factor(tokens);
    while (peek_token(tokens)->type==TK_ADD||peek_token(tokens)->type==TK_SUB){
        token_t *token= read_token(tokens);
        factor(tokens);
        switch (token->type) {
            case TK_ADD:
                write_code(OP_ADD);
                break;
            case TK_SUB:
                write_code(OP_SUB);
                break;
            default:
                panic("term err token type %d",token->type);
                return;
        }
    }
}

void comparison(array_t *tokens) { // comparison -> term ( >= | <= | > | < | == | != term )*
    term(tokens);
    while (peek_token(tokens)->type==TK_GE||peek_token(tokens)->type==TK_GT||
    peek_token(tokens)->type==TK_LE||peek_token(tokens)->type==TK_LT||
    peek_token(tokens)->type==TK_EQ||peek_token(tokens)->type==TK_NE){
        token_t *token= read_token(tokens);
        term(tokens);
        switch (token->type) {
            case TK_GE:
                write_code(OP_LT);
                write_code(OP_NOT);
                break;
            case TK_GT:
                write_code(OP_GT);
                break;
            case TK_LT:
                write_code(OP_LT);
                break;
            case TK_LE:
                write_code(OP_GT);
                write_code(OP_NOT);
                break;
            case TK_NE:
                write_code(OP_EQ);
                write_code(OP_NOT);
                break;
            case TK_EQ:
                write_code(OP_EQ);
                break;
            default:
                panic("err token type %d",token->type);
        }
    }
}

uint32_t make_empty_jump(op_code code) {
    write_code(code);
    return write_data4(0xFFFFFF00); // 要跳转的未知先随便设置一下，保存地址后面回填
}

void fill_jump(uint32_t addr) { // 回填跳转偏移
    write_jump(addr);
}

void make_jump(op_code code, uint32_t ip) { // 直接构建完整的 jump
    write_code(code);
    write_data4(ip);
}

void expression(array_t *tokens) { // expression -> comparison ( and | or comparison )* 这里and与or视为同一优先级
    comparison(tokens);
    uint32_t addrs[16]; // 最多支持 16 个表达式 and or 连接
    uint32_t index=0;
    while (peek_token(tokens)->type==TK_AND|| peek_token(tokens)->type==TK_OR){
        token_t *token= read_token(tokens);
        switch (token->type) {
            case TK_AND:
                addrs[index++]= make_empty_jump(OP_JUMP_PEEK_FALSE); // 结果已经为 false and 进行短路 且保留结果
                break;
            case TK_OR:
                addrs[index++]= make_empty_jump(OP_JUMP_PEEK_TRUE); // 结果已经为 true or 进行短路 且保留结果
                break;
            default:
                panic("err token type %d",token->type);
        }
        comparison(tokens);
        switch (token->type) {
            case TK_AND:
                write_code(OP_AND);
                break;
            case TK_OR:
                write_code(OP_OR);
                break;
            default:
                panic("err token type %d",token->type);
        }
    }
    for (int i = 0; i < index; ++i) {
        fill_jump(addrs[i]);
    }
}

void print_statement(array_t *tokens) { // print_statement -> print expression ;
    expression(tokens);
    must_match_token(tokens,TK_SEMI);
    write_code(OP_PRINT);
}

void expression_statement(array_t *tokens) {
    expression(tokens);
    must_match_token(tokens,TK_SEMI);
    write_code(OP_POP); // 直接丢弃其结果
}

void block(array_t *tokens) { // block -> { declaration* }
    write_code(OP_BEGIN_SCOPE);
    while (!match_token(tokens,TK_RIGHT2)){
        declaration(tokens);
    }
    write_code(OP_END_SCOPE);
}

void if_statement(array_t *tokens) { // if( expression ) block ( else block )?
    must_match_token(tokens,TK_LEFT);
    expression(tokens);
    must_match_token(tokens,TK_RIGHT);
    uint32_t addr1= make_empty_jump(OP_JUMP_FALSE); // 条件为假jump
    must_match_token(tokens,TK_LEFT2);
    block(tokens);
    uint32_t addr2= make_empty_jump(OP_JUMP); // 能执行到这里条件为真无条件跳过else
    fill_jump(addr1);
    if(match_token(tokens,TK_ELSE)){
        must_match_token(tokens,TK_LEFT2);
        block(tokens);
    }
    fill_jump(addr2);
}

void for_statement(array_t *tokens) { // for(var_declaration?;expression?;declaration?){}
    write_code(OP_BEGIN_SCOPE); // 设定 for循环作用域
    must_match_token(tokens,TK_LEFT);
    if(!match_token(tokens,TK_SEMI)){ // 初始化语句
        must_match_token(tokens,TK_VAR);
        var_declaration(tokens);
    }
    uint32_t ip2=get_next();
    uint32_t addr1=0;
    if(!match_token(tokens,TK_SEMI)){ // 跳出条件判断
        expression(tokens);
        addr1= make_empty_jump(OP_JUMP_FALSE);
        must_match_token(tokens,TK_SEMI);
    }
    uint32_t addr2= make_empty_jump(OP_JUMP);
    uint32_t ip1=get_next();
    if(!match_token(tokens,TK_RIGHT)){ // 增量语句
        token_t *token= must_read_token(tokens,TK_ID);
        make_constant(new_str(token->start,token->len));
        must_match_token(tokens,TK_ASSIGN);
        expression(tokens);
        write_code(OP_SET);
        must_match_token(tokens,TK_RIGHT);
    }
    make_jump(OP_JUMP,ip2);
    fill_jump(addr2);
    must_match_token(tokens,TK_LEFT2);
    block(tokens);
    make_jump(OP_JUMP,ip1);
    if(addr1 != 0){ // 结束循环
        fill_jump(addr1);
    }
    write_code(OP_END_SCOPE);
}

void return_statement(array_t *tokens) { // return_statement -> return expression? ;
    if(match_token(tokens,TK_SEMI)){
        make_constant(new_nil()); // 默认返回nil
    } else{
        expression(tokens);
        must_match_token(tokens,TK_SEMI);
    }
    write_code(OP_RETURN);
}

void statement(array_t *tokens) { // statement -> print_statement | return_statement | expression_statement | block | if_statement | for_statement
    if(match_token(tokens,TK_PRINT)){
        print_statement(tokens);
    } else if (match_token(tokens,TK_LEFT2)){
        block(tokens);
    } else if(match_token(tokens,TK_IF)){
        if_statement(tokens);
    } else if(match_token(tokens,TK_RETURN)){
        return_statement(tokens);
    } else if(match_token(tokens,TK_FOR)){
        for_statement(tokens);
    } else{
        expression_statement(tokens);
    }
}

void var_declaration(array_t *tokens) { // var id ( = expression )? ;
    token_t *token= must_read_token(tokens,TK_ID);
    make_constant(new_str(token->start,token->len));
    if(match_token(tokens,TK_ASSIGN)){
        expression(tokens);
    } else{
        make_constant(new_nil()); // 没有值赋值为 nil
    }
    must_match_token(tokens,TK_SEMI);
    write_code(OP_ASSIGN);
}

void assignment(array_t *tokens) { // expression = expression ;     or      expression ;
    expression(tokens);
    if(match_token(tokens,TK_SEMI)){ // expression ;直接结束了
        write_code(OP_POP);// 结果没有接受者丢弃
        return;
    }
    must_match_token(tokens,TK_ASSIGN);
    uint32_t pre_code= back_code();// 弹出获取上一个 op_code
    expression(tokens);
    must_match_token(tokens,TK_SEMI);
    if(pre_code==OP_GET){// id = expression ;
        write_code(OP_SET);
    } else if(pre_code==OP_GET_FIELD){ // expression.id = expression ;
        write_code(OP_SET_FIELD);
    } else{
        panic("assignment not support op_code %d",pre_code);
    }
}

// op_code OP_FUNC OP_METHOD
void func_declaration(array_t *tokens,uint8_t op_code) { // func_declaration -> func id(id,id ...){}
    // 先声明函数，再跳过函数
    token_t *name= must_read_token(tokens,TK_ID);
    make_constant(new_str(name->start,name->len)); // 压入名称
    must_match_token(tokens,TK_LEFT);
    str_t *args[32];
    int32_t size=0;
    if(!match_token(tokens,TK_RIGHT)){
        token_t *token= must_read_token(tokens,TK_ID);
        args[size++]= create_str(token->start,token->len);
        while (!match_token(tokens,TK_RIGHT)){
            must_match_token(tokens,TK_COMMA);
            token= must_read_token(tokens,TK_ID);
            args[size++]= create_str(token->start,token->len);
        }
    }
    make_constant(new_num(size)); // 压入参数数量
    uint32_t ip= get_next();
    make_constant(new_num((int32_t)ip+5+1+5)); // 压入函数入口
    write_code(op_code);
    uint32_t addr= make_empty_jump(OP_JUMP); // 只定义函数不执行函数
    // 调用函数前 现在栈中弹出的顺序依次是  正序参数，返回地址  还保留了一个返回地址
    for (int i = 0; i<size; i++) {
        make_constant(new_str(args[i]->str,args[i]->len));
        write_code(OP_SWAP);
        write_code(OP_ASSIGN); // 定义所有参数
    }
    must_match_token(tokens,TK_LEFT2); // 具体代码
    while (!match_token(tokens,TK_RIGHT2)){
        declaration(tokens);
    }
    make_constant(new_nil());
    write_code(OP_RETURN); // 兜底return return 会自动 OP_END_SCOPE
    fill_jump(addr);
}

void class_declaration(array_t *tokens) { // class_declaration -> class id ( < id )? {}
    token_t *name= must_read_token(tokens,TK_ID);
    token_t *parent=NULL;
    if(match_token(tokens,TK_LT)){
        parent= must_read_token(tokens,TK_ID);
    }
    must_match_token(tokens,TK_LEFT2);
    int32_t method_size=0;
    while (!match_token(tokens,TK_RIGHT2)){
        func_declaration(tokens,OP_METHOD); // id(){ }    class 附属内容
        method_size++;
    }
    make_constant(new_num(method_size));
    if(parent==NULL){ // 压入父类
        make_constant(new_nil());
    } else{
        make_constant(new_str(parent->start,parent->len));
        write_code(OP_GET);
    }
    make_constant(new_str(name->start, name->len)); // 压入类名称
    write_code(OP_CLASS);
}

void declaration(array_t *tokens) { // declaration -> statement | func_declaration | class_declaration | var_declaration | assignment
    if(match_token(tokens,TK_VAR)){
        var_declaration(tokens);
    } else if(match_token(tokens,TK_FUNC)){
        func_declaration(tokens,OP_FUNC);
    } else if(match_token(tokens,TK_CLASS)){
        class_declaration(tokens);
    } else if (peek_token(tokens)->type==TK_ID){
        assignment(tokens);
    } else{
        statement(tokens);
    }
}

//  - ( 1 + 2 ) * 23 / 2
void compile_code(array_t *tokens){
    while (!match_token(tokens,TK_EOF)){
        declaration(tokens);
    }
}