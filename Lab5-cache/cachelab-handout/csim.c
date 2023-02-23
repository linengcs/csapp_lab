#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct cache_line{
    unsigned valid;
    int tag;
    int lru_cnt;
}cache_line;

cache_line** create_cache();
void initial(cache_line** cache);
void free_cache(cache_line** cache);
void load(cache_line** cache, unsigned address, int size, int _time);


int hit=0,miss=0,evict=0;
int s,E,b,t;
int cover=1;
int row,col;

int main(int argc, char** argv)
{
    int opt;
    FILE *pFile;
    
    while((opt = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            pFile =fopen(optarg, "r");
            break;
        default:
            printf("unknown argument\n");
            exit(0);
            break;
        }
    }
    cache_line** cache = create_cache();
    initial(cache);
    
    char identifier;
    unsigned address;
    int size;
    int cnt=1;
    while(fscanf(pFile, "%c %x,%d",&identifier, &address, &size) > 0)
    {
        switch (identifier) 
        {
        case 'L':
            load(cache,address,size,cnt);
            break;
        case 'S':
            load(cache,address,size,cnt);
            break;
        case 'M':
            load(cache,address,size,cnt);
            load(cache,address,size,cnt);
            break;
        // case 'I':

        //     break;
        default:
            break;
        }
        cnt++;
    }

    printSummary(hit, miss, evict);
    fclose(pFile);
    free_cache(cache);
    return 0;
}

cache_line** create_cache()
{
    row = pow(2,s);
    col = E;
    cache_line** cache = (cache_line **)malloc(sizeof(cache_line*)*row);
    int i;
    for(i=0; i<row; i++) 
        cache[i] = (cache_line *)malloc(sizeof(cache_line)*col);
    return cache;
}

void initial(cache_line** cache)
{
    t = 64-b-s;
    for(int i=0; i<(b+s-1); i++) {
        cover<<=1;
        cover+=1;
    } 
    for(int i=0; i<row; i++)
        for(int j=0; j<col; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru_cnt = 0;
        }
}

void free_cache(cache_line** cache)
{
    int row = pow(2,s);
    for(int i=0; i<row; i++)
        free(cache[i]);
}

void load(cache_line** cache, unsigned address, int size, int _time)
{
    int tag = address>>(b+s);
    int set = (address&cover)>>b;
    for(int j=0; j<col; j++) {
        if(cache[set][j].valid && (cache[set][j].tag == tag)) {
            hit++;
            cache[set][j].lru_cnt = _time;
            return;
        }
        if(cache[set][j].valid == 0) {
            miss++;
            cache[set][j].valid = 1;
            cache[set][j].tag = tag;
            cache[set][j].lru_cnt = _time;
            return;
        }
    }
    // 没有找到并且set内的所有line都满了
    // 找到没有使用时间最长的数据 将它evict掉
    int index=0;
    for(int j=0; j<col; j++) {
        if(cache[set][j].lru_cnt < cache[set][index].lru_cnt)
            index = j;
    }
    cache[set][index].tag = tag;
    cache[set][index].lru_cnt = _time;
    evict++;
    miss++;
    return;
}