#include "channelmap.h"
#include <stdio.h>
#include <stdlib.h>
struct channel_map* channel_map_init(int size){
    struct channel_map* ch_map=(struct channel_map*)malloc(sizeof(struct channel_map));
    ch_map->size=size;
    ch_map->list=(struct channel_map**)malloc(size*sizeof(struct channel_map*));//为二级指针开辟存储区域
    return ch_map;
}
void channel_map_clean(struct channel_map* map){
    //清空结构体
    /*
    1.清空数组指针对应的堆内存
    2.清空数组指针内存
    */
   if(map!=NULL){
       for(int i=0;i<map->size;i++){
           free(map->list[i]);
       }
   }
   free(map->list);
   map->list=NULL;
   map->size=0;
}
bool make_map_rom(struct channel_map* map, int new_size,int unitsize){
    //数组扩容
    if(new_size>map->size){
        int cur_size=map->size;
        while(cur_size<new_size){
            cursize*=2;
        }
        //扩容realloc

        struct Channel** temp= realloc(map->list,cur_size*unitsize);
        if(temp==NULL){
            return false;
        }
        map->list=temp;
        
        //从最后一个元素的后一个索引开始进行初始化
        memset(&map->list[map->size],0,(cur_size-map->size)*unitsize);
        map->size=cur_size;
        return true;

    }
    return false;
}