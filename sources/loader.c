#include "loader.h"
#include "map.h"
#include "list.h"

#include <stdlib.h>


unsigned char *rle_encode(unsigned const char *data, unsigned int *size);
unsigned char *rle_decode(unsigned const char *data, unsigned int size);

void map_save(struct Map *map, const char *file_name) {
    FILE *fp = fopen(file_name, "wb");
    
    // Header
    int header[] = {map->size_x, map->size_y, 3};
    fwrite(header, sizeof(int), 3, fp);
    
    // Block
    unsigned int block_size = map->size_x * map->size_y;
    unsigned char *data = rle_encode(map->blocks, &block_size);
    fwrite(&block_size, sizeof(int), 1, fp);
    fwrite(data, sizeof(unsigned char), block_size, fp);
    free(data);
    
    // Zones
    int zone_count = map->zones->length + map->platform_zones->length;
    fwrite(&zone_count, sizeof(int), 1, fp);
    
    struct MapZone *zone;
    int *d = (int*)calloc(sizeof(int), 6);
    for(int i = 0; i < map->zones->length; i++) {
        zone = (struct MapZone*)list_get(map->zones, i);
        map_zone_get_region(zone, &d[0], &d[1], &d[2], &d[3]);
        d[4] = zone->type;
        d[5] = zone->extra;
        fwrite(d, sizeof(int), 6, fp);
    }
    
    for(int i = 0; i < map->platform_zones->length; i++) {
        zone = (struct MapZone*)list_get(map->platform_zones, i);
        map_zone_get_region(zone, &d[0], &d[1], &d[2], &d[3]);
        d[4] = zone->type;
        d[5] = zone->extra;
        fwrite(d, sizeof(int), 6, fp);
    }
    free(d);
    fclose(fp);
}

void *map_read(size_t size, size_t count, FILE *fp) {
    void *data = (void*)malloc(size * count);
    if (fread(data, size, count, fp) == count) {
        return data;
    
    } else {
        return NULL;   
    }
}

bool map_load(struct Map *map, const char *file_name) {
    FILE *fp = fopen(file_name, "rb");
    if (fp != NULL) {
        // Header
        int *header = (int*)map_read(sizeof(int), 3, fp);
        map->pos_x = 0;
        map->pos_y = 0;
        map->map_x = 0;
        map->map_y = 0;
        map->size_x = header[0];
        map->size_y = header[1];
        map_zones_remove(map);        
        free(header);
        
        // Blocks
        unsigned int *block_size = (unsigned int*)map_read(sizeof(unsigned int), 1, fp);
        unsigned char *block_data = (unsigned char*)map_read(sizeof(unsigned char), *block_size, fp);
        unsigned char *tmp = map->blocks;
        map->blocks = rle_decode(block_data, *block_size);
        free(tmp);
        free(block_size);
        free(block_data);
        
        // Zones
        int *zone_count = (int*)map_read(sizeof(int), 1, fp);
        int *d = NULL;
        for(int i = 0, l = *zone_count; i < l; i++) {
            d = (int*)map_read(sizeof(int), 6, fp);
            map_zone_create(map, d[0], d[1], d[2], d[3], d[4], d[5]);
        }
        map_platforms_create(map);
        free(zone_count);
        free(d);
        fclose(fp);
        return true;
    
    } else {
        return false;
    }
}


// RLE Encoding ----------------------------------------------------------------
// -----------------------------------------------------------------------------
unsigned char *rle_encode(unsigned const char *input, unsigned int *o_size) {
    bool mode = true;
    unsigned int c_size = *o_size, p = 0;
    unsigned char *out = (unsigned char*)calloc(sizeof(unsigned char), c_size * 2);
    unsigned char cur = input[0];
    unsigned char count = 1;
    for(unsigned int i = 1; i < c_size; i++) {
        if (!mode) {
            cur = input[i];
            count = 1;
            mode = true;
        
        } else if (input[i] == cur) {
            count++;
            if (count == 255) {
                out[p] = 16 + cur;
                out[p + 1] = count;
                p += 2;
                mode = false;
            }
        
        } else {
            if (count == 1) {
                out[p] = cur;
                p += 1;
            
            } else {
                out[p] = 16 + cur;
                out[p + 1] = count;
                p += 2;
            }
            cur = input[i];
            count = 1;
        }
    }
    
    if (mode) {
        if (count == 1) {
            out[p] = cur;
            p += 1;
        
        } else {
            out[p] = 16 + cur;
            out[p + 1] = count;
            p += 2;
        }
        p += 2;
    }
    *o_size = p;
    return out;
}

unsigned char *rle_decode(unsigned const char *input, unsigned int i_size) {
    unsigned int c_size = 0;
    unsigned char cur = 0;
    for(unsigned int i = 0; i < i_size;) {
        cur = input[i];
        if (cur >= 16) {
            c_size += input[i + 1];
            i += 2;
        
        } else {
            c_size += 1;
            i += 1;
        }
    }
    
    unsigned char *out = (unsigned char*)calloc(sizeof(unsigned char), c_size);
    for(unsigned int i = 0, pos = 0; i < i_size;) {
        cur = input[i];
        if (cur >= 16) {
            for(unsigned char e = 0, l = input[i + 1]; e < l; e++) {
                out[pos] = cur - 16;
                pos++;
            }
            i += 2;
        
        } else {
            out[pos] = cur;
            pos += 1;
            i += 1;
        }
    }
    return out;
}

