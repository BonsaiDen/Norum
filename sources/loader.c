#include "loader.h"
#include "map.h"
#include "list.h"

#include <stdlib.h>


unsigned char *rle_encode(unsigned const char *data, unsigned int *size);
unsigned char *rle_decode(unsigned const char *data, unsigned int size);

void map_save(struct Map *map, const char *file_name) {
    FILE *fp = fopen(file_name, "wb");
    
    // Header
    unsigned short header[] = {map->size_x, map->size_y, 4};
    fwrite(header, sizeof(unsigned short), 3, fp);
    
    // Block
    unsigned int block_size = map->size_x * map->size_y;
    unsigned char *data = rle_encode(map->blocks, &block_size);
    fwrite(&block_size, sizeof(unsigned int), 1, fp);
    fwrite(data, sizeof(unsigned char), block_size, fp);
    free(data);
    
    // Zones
    unsigned short zone_count = map->zones->length + map->platform_zones->length;
    fwrite(&zone_count, sizeof(unsigned short), 1, fp);
    
    struct MapZone *zone;
    unsigned short *d = (unsigned short*)calloc(sizeof(unsigned short), 4);
    unsigned char *e = (unsigned char*)calloc(sizeof(unsigned char), 2);
    for(int i = 0; i < map->zones->length; i++) {
        zone = (struct MapZone*)list_get(map->zones, i);
        d[0] = zone->x;
        d[1] = zone->y;
        d[2] = zone->w;
        d[3] = zone->h;
        e[0] = zone->type;
        e[1] = zone->extra;
        fwrite(d, sizeof(unsigned short), 4, fp);
        fwrite(e, sizeof(unsigned char), 2, fp);
    }
    
    for(int i = 0; i < map->platform_zones->length; i++) {
        zone = (struct MapZone*)list_get(map->platform_zones, i);
        d[0] = zone->x;
        d[1] = zone->y;
        d[2] = zone->w;
        d[3] = zone->h;
        e[0] = zone->type;
        e[1] = zone->extra;
        fwrite(d, sizeof(unsigned short), 4, fp);
        fwrite(e, sizeof(unsigned char), 2, fp);
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
        unsigned short *header = (unsigned short*)map_read(sizeof(unsigned short), 3, fp);
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
        unsigned short *zone_count = (unsigned short*)map_read(sizeof(unsigned short), 1, fp);
        unsigned short *d = NULL;
        unsigned char *e = NULL;
        for(int i = 0, l = *zone_count; i < l; i++) {
            d = (unsigned short*)map_read(sizeof(unsigned short), 4, fp);
            e = (unsigned char*)map_read(sizeof(unsigned char), 2, fp);
            map_zone_create(map, d[0], d[1], d[2], d[3], e[0], e[1]);
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

