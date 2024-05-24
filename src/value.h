#ifndef MY_LOX_VALUE_H
#define MY_LOX_VALUE_H

#include "type.h"
#include "map.h"

typedef struct { // VT_STR 对应的结构体原生str不太好用，这里封装一下
    char *str;
    uint32_t len;
}str_t;

typedef struct {
    char *name; // 函数名称
    uint32_t ip; // 函数入口
    uint32_t args_size; // 参数数量
    map_t *map; // 声明时的参数情况
}func_t;

typedef value_t *(*native_func_t)(value_t *args[]) ;

typedef struct {
    char *name; // 函数名称
    native_func_t func; // 实际函数
    uint32_t args_size; // 参数数量
}native_t;

typedef struct class0 class_t;

struct class0{
    class_t *parent; // 父类
    char *name; // 类型名称
    map_t *methods; // 方法列表
};

typedef struct {
    class_t *class;
    map_t *fields;
}instance_t;

value_t *new_bool(bool bool0);
value_t *new_num(int32_t num);
value_t *new_nil();
value_t *new_ptr(void *ptr);
value_t *new_str(char *str,uint32_t len);
value_t *new_func(str_t *name,uint32_t ip,uint32_t args_size,map_t *map);
value_t *new_native(char *name,uint32_t len,native_func_t func,uint32_t args_size);
value_t *new_class(str_t *name,class_t *parent);
value_t *new_instance(class_t *class);

value_t *class_method(class_t *class,char *name);
value_t *instance_get(instance_t *instance,char *name);
value_t *instance_super_method(instance_t *instance,char *name);

char *to_chars(value_t *val);
str_t *to_str(value_t *val);
func_t *to_func(value_t *val);
native_t *to_native(value_t *val);
class_t *to_class(value_t *val);
instance_t *to_instance(value_t *val);

void print_value(value_t *val);

str_t *create_str(char *str,uint32_t len);


#endif
