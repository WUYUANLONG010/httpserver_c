#include "channelmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct ChannelMap *channel_map_init(int size) {
    struct ChannelMap *ch_map = (struct ChannelMap *)malloc(sizeof(struct ChannelMap));
    ch_map->size              = size;
    ch_map->list              = (struct Channel **)malloc(size * sizeof(struct Channel *)); // 为二级指针开辟存储区域
    return ch_map;
}
void channel_map_clean(struct ChannelMap *map) {
    // 清空结构体
    /*
    1.清空数组指针对应的堆内存
    2.清空数组指针内存
    */
    if (map != NULL) {
        for (int i = 0; i < map->size; i++) {
            free(map->list[i]);
        }
    }
    free(map->list);
    map->list = NULL;
    map->size = 0;
}
bool make_map_rom(struct ChannelMap *map, int new_size, int unitsize) {
    // 数组扩容
    if (new_size > map->size) {
        int cur_size = map->size;
        while (cur_size < new_size) {
            cur_size *= 2;
        }
        // 扩容realloc

        struct Channel **temp = realloc(map->list, cur_size * unitsize);
        if (temp == NULL) {
            return false;
        }
        map->list = temp;

        // 从最后一个元素的后一个索引开始进行初始化
        memset(&map->list[map->size], 0, (cur_size - map->size) * unitsize);
        map->size = cur_size;
        return true;
    }
    return false;
}