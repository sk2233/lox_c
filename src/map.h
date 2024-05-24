#ifndef MY_LOX_MAP_H
#define MY_LOX_MAP_H
#include "type.h"
#include "array.h"

#define MAP_ENTRY_SIZE 32

typedef struct {
    char *key;
    value_t *value;
}entry_t;

typedef struct map map_t;

struct map{
    map_t *parent;
    uint32_t count;
    array_t *items[MAP_ENTRY_SIZE];
};

map_t *new_map(uint32_t size);
value_t *map_set(map_t *map,char *key,value_t *val);
value_t *map_get(map_t *map,char *key);
value_t *map_del(map_t *map,char *key);
void map_free(map_t *map);

map_t *new_map_with_parent(uint32_t size,map_t *parent);
value_t *map_set_no_parent(map_t *map,char *key,value_t *val);

#endif
