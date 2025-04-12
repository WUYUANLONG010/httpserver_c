#pragma once
#include "channel.h"
#include <stdbool.h>
struct ChannelMap{
    int size;
    struct Channel** list;
};
struct ChannelMap* channel_map_init(int size);
void channel_map_clean(struct ChannelMap* map);
bool make_map_rom(struct ChannelMap* map, int new_size,int unitsize);