#include <stdlib.h>
#include <string.h>
#include <printf.h>
#include "common.h"
#include "value.h"

value_t *new_bool(bool bool0){
    value_t *data= malloc(sizeof(value_t)); // 内存泄露
    data->type=VT_BOOL;
    data->bool0=bool0;
    return data;
}

value_t *new_num(int32_t num){
    value_t *data= malloc(sizeof(value_t));
    data->type=VT_NUM;
    data->num=num;
    return data;
}

value_t *new_nil(){
    value_t *data= malloc(sizeof(value_t));
    data->type=VT_NIL;
    return data;
}

value_t *new_ptr(void *ptr){
    value_t *data= malloc(sizeof(value_t));
    data->type=VT_PTR;
    data->ptr=ptr;
    return data;
}

str_t *create_str(char *str,uint32_t len){
    str_t *res= malloc(sizeof(str_t));
    res->len=len;
    res->str= malloc(len + 1); // 多存储一位保存结束字符 主要用于调试输出，大部分情况下使用 str.len
    memcpy(res->str, str, len);
    res->str[len]='\0';
    return res;
}

value_t *new_str(char *str,uint32_t len){
    value_t *data= malloc(sizeof(value_t));
    data->type=VT_STR;
    data->ptr= create_str(str,len);
    return data;
}

value_t *new_func(str_t *name,uint32_t ip,uint32_t args_size,map_t *map){
    value_t *data= malloc(sizeof(value_t));
    data->type=VT_FUNC;
    func_t *temp= malloc(sizeof(func_t));
    temp->name= malloc(name->len + 1); // 多存储一位保存结束字符 主要用于调试输出，大部分情况下使用 str.len
    memcpy(temp->name, name->str, name->len);
    temp->name[name->len]='\0';
    temp->ip=ip;
    temp->args_size=args_size;
    temp->map=map;
    data->ptr=temp;
    return data;
}

value_t *method_bind_instance(value_t *method,instance_t *instance){
    // 先克隆一份
    method= clone(method, sizeof(value_t));
    func_t *func= clone(method->ptr, sizeof(func_t));
    method->ptr=func;
    // 再设置一层map 为其设置this变量
    func->map= new_map_with_parent(1024,func->map);
    value_t *val= malloc(sizeof(value_t));
    val->type=VT_INSTANCE;
    val->ptr=instance;
    map_set_no_parent(func->map,"this",val);
    return method;
}

value_t *new_native(char *name,uint32_t len,native_func_t func,uint32_t args_size){
    value_t *data= malloc(sizeof(value_t));
    data->type=VT_NATIVE;
    native_t *temp= malloc(sizeof(native_t));
    temp->name= malloc(len + 1); // 多存储一位保存结束字符 主要用于调试输出，大部分情况下使用 str.len
    memcpy(temp->name, name, len);
    temp->name[len]='\0';
    temp->args_size=args_size;
    temp->func=func;
    data->ptr=temp;
    return data;
}

value_t *new_class(str_t *name,class_t *parent){
    value_t *data= malloc(sizeof(value_t));
    data->type=VT_CLASS;
    class_t *temp= malloc(sizeof(class_t));
    temp->name= malloc(name->len + 1); // 多存储一位保存结束字符 主要用于调试输出，大部分情况下使用 str.len
    memcpy(temp->name, name->str, name->len);
    temp->name[name->len]='\0';
    temp->methods= new_map(1024);
    temp->parent=parent;
    data->ptr=temp;
    return data;
}

value_t *class_method(class_t *class,char *name){
    value_t *method= map_get(class->methods,name);
    if(method!=NULL){
        return method;
    }
    if(class->parent!=NULL){
        return class_method(class->parent,name);
    }
    panic("class %s no method of %s",class->name,name);
    return NULL;
}

value_t *new_instance(class_t *class){
    value_t *data= malloc(sizeof(value_t));
    data->type=VT_INSTANCE;
    instance_t *instance= malloc(sizeof(instance_t));
    instance->class=class;
    instance->fields= new_map(1024);
    data->ptr=instance;
    return data;
}

value_t *instance_get(instance_t *instance,char *name){
    value_t *field = map_get(instance->fields,name);// 优先获取属性
    if(field!=NULL){
        return field;
    }
    value_t *method = class_method(instance->class,name); // 再尝试获取方法 这里不会为空 内部为空会报错
    return method_bind_instance(method,instance);
}

value_t *instance_super_method(instance_t *instance,char *name){
    if(instance->class->parent==NULL){
        panic("class %s no super class",instance->class->name);
    }
    value_t *method= class_method(instance->class->parent,name);
    return method_bind_instance(method,instance);
}

char *to_chars(value_t *val){
    if(val->type!=VT_STR){
        panic("val type %d not str",val->type);
    }
    str_t *str=val->ptr;
    return str->str;
}

str_t *to_str(value_t *val){
    if(val->type!=VT_STR){
        panic("val type %d not str",val->type);
    }
    return val->ptr;
}

func_t *to_func(value_t *val) {
    if(val->type!=VT_FUNC){
        panic("val type %d not func",val->type);
    }
    return val->ptr;
}

native_t *to_native(value_t *val) {
    if(val->type!=VT_NATIVE){
        panic("val type %d not native",val->type);
    }
    return val->ptr;
}

class_t *to_class(value_t *val){
    if(val->type!=VT_CLASS){
        panic("val type %d not class",val->type);
    }
    return val->ptr;
}

instance_t *to_instance(value_t *val){
    if(val->type!=VT_INSTANCE){
        panic("val type %d not instance",val->type);
    }
    return val->ptr;
}

void print_value(value_t *val){
    switch (val->type) {
        case VT_STR:
            printf("%s", to_chars(val));
            break;
        case VT_NIL:
            printf("nil");
            break;
        case VT_BOOL:
            if(val->bool0){
                printf("true");
            } else{
                printf("false");
            }
            break;
        case VT_NUM:
            printf("%d",val->num);
            break;
        case VT_CLASS:
            printf("%s{}", to_class(val)->name);
            break;
        case VT_INSTANCE:
            printf("%s[]", to_instance(val)->class->name);
            break;
        case VT_PTR:
            printf("0x%08x",val->num);
            break;
        default:
            if(val->type==VT_FUNC){
                func_t *func= to_func(val);
                printf("%s(%d)",func->name,func->args_size);
            } else if(val->type==VT_NATIVE){
                native_t *native=to_native(val);
                printf("%s<%d>",native->name,native->args_size);
            } else{
                printf("unknown");
            }
            break;
    }
}
