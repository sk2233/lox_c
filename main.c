#include <printf.h>
#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "parser.h"
#include "compile.h"
#include "common.h"
#include "map.h"

// https://readonly.link/books/https://raw.githubusercontent.com/GuoYaxiang/craftinginterpreters_zh/main/book.json
int main() {
//    uint32_t pos= write_constant(22);
//    write_code(OP_CONSTANT);
//    write_data4(pos);
//    pos= write_constant(33);
//    write_code(OP_CONSTANT);
//    write_data4(pos);
//    write_code(OP_SUB);
//    write_code(OP_NOT);
//    write_code(OP_RETURN);
//    printf("size %lu\n", sizeof(value_t));
//    char *str1=malloc(10);
//    char *str2="sssss";
//    for (int i = 0; i < 5; ++i) {
//        str1[i]='s';
//    }
//    printf("%d\n",memcmp(str1,str2,-1));
//    map_t *map= new_map(1024);
//    value_t *val= map_set(map,"test", new_bool(true));
//     val= map_get(map,"test");
//    val= map_del(map,"test");
//    val= map_get(map,"test");
    //  解析token
    array_t *tokens= parse_token("class A{"
                                 "test(){"
                                 "print \"AAA\";"
                                 "}"
                                 "}"
                                 "class B < A{"
                                 "test(){"
                                 "super.test();"
                                 "print \"BBB\";"
                                 "}"
                                 "}"
                                 "var b=B();"
                                 "b.test();"
                                 "var num=22;"
                                 "if(num>33){"
                                 "print 22;"
                                 "}else{"
                                 "print 33;"
                                 "}"
                                 "for(var i=0;i<10;i=i+1){"
                                 "print i;"
                                 "}");
    // 为添加code做准备
    init_data();
    // 编译为code直接放到虚拟机中
    compile_code(tokens);
    // 运行代码
    run_code();
    return 0;
}
