#include <printf.h>
#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "array.h"
#include "stack.h"
#include "common.h"
#include "debug.h"
#include "map.h"
#include "value.h"

static array_t *code;
static array_t *constant;
static stack0_t *stack;
static map_t *map;
static uint32_t ip; // ip寄存器

value_t *time_now(value_t *args[]){
    return new_num((int32_t)(clock() / CLOCKS_PER_SEC));
}

void init_native() {
    char *name= malloc(9);
    memcpy(name,"time_now",8);
    map_set_no_parent(map, name, new_native(name, 8, time_now, 0));
}

void init_data(){
    code= new_array(1024);
    constant= new_array(1024);
    stack= new_stack(1024);
    map= new_map(1024);
    ip=0;
    init_native();
}

//======================常量操作======================

uint32_t write_constant(value_t *data) {
    return array_write(constant,data, sizeof(value_t));
}

value_t *read_constant(uint32_t offset) {
    return array_at(constant,offset);
}

//====================代码操作=======================

uint32_t write_code(uint8_t op_code) {
    return array_write1(code,op_code);
}

uint8_t read_code(uint32_t offset){
    return  array_at1(code, offset);
}

uint32_t write_data4(uint32_t data) {
    return array_write4(code, data);
}

uint32_t get_next(){
    return code->count;
}

uint8_t back_code(){ // 弹出返回最后一条指令
    code->count--;
    return read_code(code->count);
}

void write_jump(uint32_t offset){
    array_write_at(code,offset,&code->count,4);
}

uint32_t read_data4(uint32_t offset) {
    return array_at4(code, offset);
}

bool has_more(uint32_t offset){
    return offset<code->count;
}

//===================栈操作=======================

void print_stack(){
    printf("stack[");
    for (uint32_t i = 0; i < stack->top; i+= sizeof(value_t)) {
        if(i>0){
            printf(",");
        }
        value_t *data= stack_at(stack,i);
        print_value(data);
    }
    printf("]\n");
}

void push_data4(uint32_t data){
    stack_push4(stack,data);
    print_stack();
}

uint32_t pop_data4(){
    uint32_t data=stack_pop4(stack);
    print_stack();
    return data;
}

value_t *peek_value(){
    return stack_peek(stack, sizeof(value_t));
}

value_t *must_peek_value(value_type type){
    value_t *data=stack_peek(stack, sizeof(value_t));
    if(data->type!=type){
        panic("must_peek_value type miss match %d %d",data->type,type);
    }
    return data;
}

value_t *pop_value(){
    value_t *data=stack_pop(stack, sizeof(value_t));
    print_stack();
    return clone(data, sizeof(value_t));
}

value_t *must_pop_value(value_type type){
    value_t *data=stack_pop(stack, sizeof(value_t));
    if(data->type!=type){
        panic("must_pop_value type miss match %d %d",data->type,type);
    }
    print_stack();
    return clone(data, sizeof(value_t));
}

void push_value(value_t *data){
    stack_push(stack,data, sizeof(value_t));
    print_stack();
}

//=================执行代码=================

void run_constant() {
    uint32_t addr= read_data4(ip);
    ip+=4;
    value_t *temp= read_constant(addr);
    push_value(temp);
}

void run_not() {
    value_t *data=pop_value();
    switch (data->type) {
        case VT_NUM:
            data->num=-data->num;
            break;
        case VT_BOOL:
            data->bool0=!data->bool0;
            break;
        default:
            panic("err data type %d",data->type);
    }
    push_value(data);
}

value_t *link_str(str_t *str1, str_t *str2) {
    char *temp =malloc(str1->len+str2->len);
    memcpy(temp,str1->str,str1->len);
    memcpy(temp+str1->len,str2->str,str2->len);
    value_t *res= new_str(temp,str1->len+str2->len);
    free(temp);
    return res;
}

void run_binary(uint8_t op_code) {
    value_t *val1 = pop_value();
    value_t *val2 = pop_value();
    if(val1->type==VT_NUM&&val2->type==VT_NUM){
        switch (op_code) {
            case OP_ADD: // 还有字符串拼接
                val2->num+=val1->num;
                push_value(val2);
                break;
            case OP_SUB:
                val2->num-=val1->num;
                push_value(val2);
                break;
            case OP_MUL:
                val2->num*=val1->num;
                push_value(val2);
                break;
            case OP_DIV:
                val2->num/=val1->num;
                push_value(val2);
                break;
            case OP_GT:
                push_value(new_bool(val2->num>val1->num));
                break;
            case OP_LT:
                push_value(new_bool(val2->num<val1->num));
                break;
            case OP_EQ:
                push_value(new_bool(val2->num==val1->num));
                break;
            default:
                panic("binary VT_NUM not support op %d",op_code);
        }
    } else if(val1->type==VT_BOOL&&val2->type==VT_BOOL){
        switch (op_code) {
            case OP_EQ:
                val2->bool0=val1->bool0==val2->bool0;
                push_value(val2);
                break;
            case OP_ADD:
                val2->bool0=val1->bool0&&val2->bool0;
                push_value(val2);
                break;
            case OP_OR:
                val2->bool0=val1->bool0||val2->bool0;
                push_value(val2);
                break;
            default:
                panic("binary VT_BOOL not support op %d",op_code);
        }
    } else if(val1->type==VT_NIL&&val2->type==VT_NIL){
        switch (op_code) {
            case OP_EQ:
                push_value(new_bool(true));
                break;
            default:
                panic("binary VT_NIL not support op %d",op_code);
        }
    }else if(val1->type==VT_STR&&val2->type==VT_STR){
        str_t *str1=val1->ptr;
        str_t *str2=val2->ptr;
        switch (op_code) {
            case OP_EQ:
                if(str1->len!=str2->len){
                    push_value(new_bool(false));
                } else{
                    push_value(new_bool(memcmp(str1->str,str2->str,str1->len)==0));
                }
                break;
            case OP_ADD:
                push_value(link_str(str1,str2));
                break;
            default:
                panic("binary VT_NIL not support op %d",op_code);
        }
    } else{
        panic("binary val type err %d %d",val1->type,val2->type);
    }
}

void run_add() {
    run_binary(OP_ADD);
}

void run_sub() {
    run_binary(OP_SUB);
}

void run_mul() {
    run_binary(OP_MUL);
}

void run_div() {
    run_binary(OP_DIV);
}

void run_gt() {
    run_binary(OP_GT);
}

void run_lt() {
    run_binary(OP_LT);
}

void run_eq() {
    run_binary(OP_EQ);
}

void run_print() {
    value_t *value= pop_value();
    printf(">>");
    print_value(value);
    printf("\n");
}

void run_pop() {
    pop_value();
}

void run_assign() {
    value_t *val=pop_value();
    value_t *id=must_pop_value(VT_STR);
    // 这里只会定义在当前层次
    if(map_set_no_parent(map, to_chars(id), val) != NULL){
        panic("var %s already definition", to_chars(id));
    }
}

void run_get() {
    value_t *id=pop_value();
    value_t *val= map_get(map, to_chars(id));
    if(val==NULL){
        panic("err var %s", to_chars(id));
    }
    push_value(val);
}

void run_set() {
    value_t *val=pop_value();
    value_t *id=must_pop_value(VT_STR);
    if(map_set(map, to_chars(id), val) == NULL){
        panic("var %s no definition", to_chars(id));
    }
}

void run_begin_scope() {
    map= new_map_with_parent(1024,map); // 开始新作用域
}

void run_end_scope() {
    map_t *old=map->parent;
    map_free(map); // 结束作用域
    map=old;
}

void run_jump0(bool jump) {
    if(jump){
        ip=read_data4(ip);
    } else{
        ip+=4;
    }
}

void run_jump_false() {
    value_t *val= must_pop_value(VT_BOOL);
    run_jump0(!val->bool0); // 为false直接进行跳转
}

void run_jump() {
    run_jump0(true);
}

void run_jump_peek_true() {
    value_t *val= must_peek_value(VT_BOOL);
    run_jump0(val->bool0); // 为true直接进行跳转
}

void run_jump_peek_false() {
    value_t *val= must_peek_value(VT_BOOL);
    run_jump0(!val->bool0); // 为true直接进行跳转
}

void run_and() {
    run_binary(OP_AND);
}

void run_or() {
    run_binary(OP_OR);
}

void run_swap() { // 交换栈顶两个元素
    value_t *val1=pop_value();
    value_t *val2=pop_value();
    push_value(val1);
    push_value(val2);
}

void run_return() {
    value_t *val= pop_value(); // 返回值
    value_t *ip_val= must_pop_value(VT_NUM); // ip返回值
    ip=ip_val->num;
    value_t *map_val= must_pop_value(VT_PTR);
    map=map_val->ptr;
    push_value(val); // 设置返回值
}

void run_call() {
    value_t *ret=must_pop_value(VT_NUM); // 返回地址
    value_t *args_size = must_pop_value(VT_NUM); // 参数个数
    value_t *args[args_size->num];
    for (int i = args_size->num-1; i >=0; i--) {// 调用参数
        args[i]=pop_value();
    }
    value_t *val = pop_value(); // 调用函数
    if(val->type == VT_FUNC){
        func_t *func= to_func(val);
        if(args_size->num!=func->args_size){
            panic("call func %s args_size not match %d %d",func->name,args_size->num,func->args_size);
        }
        push_value(new_ptr(map));
        map= new_map_with_parent(1024,func->map);// 使用函数自己的环境
        push_value(ret);
        for (int i = args_size->num-1; i >=0 ; i--) {
            push_value(args[i]);
        }
        ip=func->ip; // 做好准备函数调用
    } else if(val->type == VT_NATIVE){
        native_t *native= to_native(val);
        if(args_size->num!=native->args_size){
            panic("call native %s args_size not match %d %d",native->name,args_size->num,native->args_size);
        }
        val= native->func(args);
        push_value(val); // 压入原生函数返回值
        ip=ret->num; //直接调用到达返回地址
    } else if(val->type==VT_CLASS){ // 把类当方法调用就是构造其实例
        class_t *class= to_class(val);
        push_value(new_instance(class));
        ip=ret->num;
    } else{
        panic("call type %d not val or native", val->type);
    }
}

void run_class() { // method_name method ...  method_size class_name
    value_t *name= must_pop_value(VT_STR);
    class_t *parent=NULL;
    value_t *temp= pop_value();
    if(temp->type==VT_CLASS){
        parent= to_class(temp);
    }
    value_t *val= new_class(to_str(name),parent);
    class_t *class= to_class(val);
    value_t *size= must_pop_value(VT_NUM);
    for (int32_t i = 0; i < size->num; ++i) {
        value_t *method=pop_value();
        value_t *method_name = must_pop_value(VT_STR);
        map_set_no_parent(class->methods, to_chars(method_name),method);
    }
    // 这里只会定义在当前层次
    if(map_set_no_parent(map, to_chars(name), val) != NULL){
        panic("var %s already definition", to_chars(name));
    }
}

void run_func() {
    value_t *ip_val= must_pop_value(VT_NUM);
    value_t *args_size= must_pop_value(VT_NUM);
    value_t *name= must_peek_value(VT_STR);
    push_value(new_func(to_str(name),ip_val->num,args_size->num,map));
    run_assign(); // 定义函数
}

void run_method() {// 先 把函数名称 -> 函数 留在栈中
    value_t *ip_val= must_pop_value(VT_NUM);
    value_t *args_size= must_pop_value(VT_NUM);
    value_t *name= must_peek_value(VT_STR);
    push_value(new_func(to_str(name),ip_val->num,args_size->num,map));
}

void run_get_field() {
    value_t *field= must_pop_value(VT_STR);
    value_t *val= must_pop_value(VT_INSTANCE);
    instance_t *instance= to_instance(val);
    val= instance_get(instance, to_chars(field));
    if(val==NULL){
        panic("err var %s", to_chars(field));
    }
    push_value(val);
}

void run_set_field() {
    value_t *val= pop_value();
    value_t *field= must_pop_value(VT_STR);
    value_t *temp= must_pop_value(VT_INSTANCE);
    instance_t *instance= to_instance(temp);
    map_set_no_parent(instance->fields, to_chars(field),val);
}

void run_super_method() {
    value_t *method= must_pop_value(VT_STR);
    value_t *val= map_get(map,"this");
    if(val->type!=VT_INSTANCE){
        panic("super must has instance this");
    }
    instance_t *instance= to_instance(val);
    val= instance_super_method(instance,to_chars(method));
    push_value(val);
}

void run_code(){
    while (has_more(ip)){
        disassemble_instruction(ip);
        uint8_t op_code = read_code(ip++);
        switch (op_code) {
            case OP_RETURN:
                run_return();
                break;
            case OP_CONSTANT:
                run_constant();
                break;
            case OP_NOT:
                run_not();
                break;
            case OP_ADD:
                run_add();
                break;
            case OP_SUB:
                run_sub();
                break;
            case OP_MUL:
                run_mul();
                break;
            case OP_DIV:
                run_div();
                break;
            case OP_GT:
                run_gt();
                break;
            case OP_LT:
                run_lt();
                break;
            case OP_EQ:
                run_eq();
                break;
            case OP_PRINT:
                run_print();
                break;
            case OP_POP:
                run_pop();
                break;
            case OP_ASSIGN:
                run_assign();
                break;
            case OP_GET:
                run_get();
                break;
            case OP_SET:
                run_set();
                break;
            case OP_BEGIN_SCOPE:
                run_begin_scope();
                break;
            case OP_END_SCOPE:
                run_end_scope();
                break;
            case OP_JUMP_FALSE:
                run_jump_false();
                break;
            case OP_JUMP:
                run_jump();
                break;
            case OP_JUMP_PEEK_TRUE:
                run_jump_peek_true();
                break;
            case OP_JUMP_PEEK_FALSE:
                run_jump_peek_false();
                break;
            case OP_AND:
                run_and();
                break;
            case OP_OR:
                run_or();
                break;
            case OP_FUNC:
                run_func();
                break;
            case OP_SWAP:
                run_swap();
                break;
            case OP_CALL:
                run_call();
                break;
            case OP_CLASS:
                run_class();
                break;
            case OP_GET_FIELD:
                run_get_field();
                break;
            case OP_SET_FIELD:
                run_set_field();
                break;
            case OP_METHOD:
                run_method();
                break;
            case OP_SUPER_METHOD:
                run_super_method();
                break;
            default:
                panic("unknown op_code %d",op_code);
        }
    }
}
