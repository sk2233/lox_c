#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "common.h"

map_t *new_map(uint32_t size){
    map_t *res=malloc(sizeof(map_t));
    res->count=0;
    for (int i = 0; i < MAP_ENTRY_SIZE; ++i) {
        res->items[i]= new_array(size);
    }
    res->parent=NULL;
    return res;
}

map_t *new_map_with_parent(uint32_t size,map_t *parent){
    map_t *map= new_map(size);
    map->parent=parent;
    return map;
}

uint32_t get_index(char *key) {
    uint32_t res =0;
    while (*key!='\0'){
        res+=*key++;
    }
    return res&(MAP_ENTRY_SIZE-1);
}

void *new_entry(char *key, value_t *val) {
    entry_t *entry= malloc(sizeof(entry_t));
    entry->key=key;
    entry->value= clone(val, sizeof(value_t)); // 必须克隆
    return entry;
}

bool str_eq(char *val1, char *val2) {
    while (*val1!='\0'&&*val2!='\0'){
        if(*val1++!=*val2++){
            return false;
        }
    }
    return *val1=='\0'&&*val2=='\0';
}

value_t *map_set(map_t *map, char *key, value_t *val){
    uint32_t index = get_index(key);
    array_t *array= map->items[index];
    for (uint32_t offset = 0; offset < array->count;offset+= sizeof(entry_t)) {
        entry_t *entry= array_at(array,offset);
        if(str_eq(entry->key,key)){
            value_t *old=entry->value;
            entry->value= clone(val, sizeof(value_t));
            return old;
        }
    }
    if(map->parent!=NULL){
        return map_set(map->parent,key,val);
    }
    array_write(array,new_entry(key,val), sizeof(entry_t));
    map->count++;
    return NULL;
}

value_t *map_set_no_parent(map_t *map,char *key,value_t *val){
    uint32_t index = get_index(key);
    array_t *array= map->items[index];
    for (uint32_t offset = 0; offset < array->count;offset+= sizeof(entry_t)) {
        entry_t *entry= array_at(array,offset);
        if(str_eq(entry->key,key)){
            value_t *old=entry->value;
            entry->value=val;
            return old;
        }
    }
    array_write(array,new_entry(key,val), sizeof(entry_t));
    map->count++;
    return NULL;
}

value_t *map_get(map_t *map,char *key){
    uint32_t index = get_index(key);
    array_t *array= map->items[index];
    for (uint32_t offset = 0; offset < array->count;offset+= sizeof(entry_t)) {
        entry_t *entry= array_at(array,offset);
        if(str_eq(entry->key,key)){
            return entry->value;
        }
    }
    if(map->parent!=NULL){
        return map_get(map->parent,key);
    }
    return NULL;
}

value_t *map_del(map_t *map,char *key){ // 基本不用
    uint32_t index = get_index(key);
    array_t *array= map->items[index];
    for (uint32_t offset = 0; offset < array->count;offset+= sizeof(entry_t)) {
        entry_t *entry= array_at(array,offset);
        if(str_eq(entry->key,key)){
            value_t *old=entry->value;
            uint32_t size= sizeof(entry_t);
            memcpy(array->data+offset,array->data+offset+size,array->count-offset- size);
            array->count-=size;
            map->count--;
            return old;
        }
    }
    return NULL;
}

void map_free(map_t *map){
    for (int i = 0; i < MAP_ENTRY_SIZE; ++i) {
        array_free(map->items[i]);
    }
    free(map);
}