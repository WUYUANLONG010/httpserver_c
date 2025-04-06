#progma once
#include "channel.h"
#include <stdbool.h>
struct channel_map{
    int size;
    struct channel** list;
}
struct channel_map* channel_map_init(int size);
void channel_map_clean(struct channel_map* map);
bool make_map_rom(struct channel_map* map, int new_size,int unitsize);