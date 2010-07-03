#include "loader.h"
#include "map.h"
#include "list.h"

#include <stdlib.h>


unsigned char *rle_encode(unsigned const char *data, int *size);
unsigned char *rle_decode(unsigned const char *data, int size);

void map_save(struct Map *map, const char *file_name) {
    FILE *fp = fopen(file_name, "w");
    
    // Header
    int header[] = {map->size_x, map->size_y, 1};
    fwrite(header, sizeof(int), 3, fp);
    
    // Block
    int block_size = map->size_x * map->size_y;
    unsigned char *data = rle_encode(map->blocks, &block_size);
    fwrite(&block_size, sizeof(int), 1, fp);
    fwrite(data, sizeof(unsigned char), block_size, fp);
    free(data);
    
    // Zones
    int zone_count = map->zones->length;
    fwrite(&zone_count, sizeof(int), 1, fp);
    
    int *d = (int*)calloc(sizeof(int), 4);
    for(int i = 0; i < zone_count; i++) {
        struct MapZone *zone = (struct MapZone*)list_get(map->zones, i);
        map_zone_get_region(zone, &d[0], &d[1], &d[2], &d[3]);
        fwrite(d, sizeof(int), 4, fp);
    }
    free(d);
    fclose(fp);
}

bool map_load(struct Map *map, const char *file_name) {
    FILE *fp = fopen(file_name, "r");
    if (fp != NULL) {
    
        // Header
        int *header = (int*)calloc(sizeof(int), 3);
        fread(header, sizeof(int), 3, fp);
        map->pos_x = 0;
        map->pos_y = 0;
        map->map_x = 0;
        map->map_y = 0;
        map->size_x = header[0];
        map->size_y = header[1];
        map_zones_remove(map);
        free(header);
        
        // Blocks
        int *block_size = (int*)malloc(sizeof(int));
        fread(block_size, sizeof(int), 1, fp);
        
        unsigned char *block_data = (unsigned char*)calloc(sizeof(unsigned char), *block_size);
        fread(block_data, sizeof(unsigned char), *block_size, fp);
        
        unsigned char *tmp = map->blocks;
        map->blocks = rle_decode(block_data, *block_size);
        free(tmp);
        free(block_size);
        free(block_data);
        
        // Zones
        int *zone_count = (int*)malloc(sizeof(int));
        fread(zone_count, sizeof(int), 1, fp);  
        
        int *d = (int*)calloc(sizeof(int), 4);
        for(int i = 0, l = *zone_count; i < l; i++) {
            fread(d, sizeof(int), 4, fp);
            map_zone_create(map, d[0], d[1], d[2], d[3]);
        }
        free(zone_count);
        free(d);
        fclose(fp);
        return true;
    }
    return false;
}


// RLE Encoding ----------------------------------------------------------------
// -----------------------------------------------------------------------------
unsigned char *rle_encode(unsigned const char *data, int *size) {
    int csize = *size;
    unsigned char *rle = (unsigned char*)calloc(sizeof(unsigned char), csize * 2);
    
    char mode = 1;
    unsigned char current = data[0];
    unsigned char count = 1;
    int elements = 0;
    for(int i = 1; i < csize; i++) {
        if (!mode) {
            current = data[i];
            count = 1;
            mode = 1;
        
        } else if (data[i] == current) {
            count++;
            if (count == 255) {
                rle[elements] = count;
                rle[elements + 1] = current;
                elements += 2;
                mode = 0;
            }
        
        } else {
            rle[elements] = count;
            rle[elements + 1] = current;
            elements += 2;
            current = data[i];
            count = 1;
        }
    }
    
    if (mode) {
        rle[elements] = count;
        rle[elements + 1] = current;
        elements += 2;
    }
    *size = elements;
    return rle;
}

unsigned char *rle_decode(unsigned const char *data, int size) {
    int csize = size;
    int complete_size = 0;
    for(int i = 0; i < csize; i+= 2) {
        complete_size += data[i];
    }
    
    unsigned char *rle = (unsigned char*)calloc(sizeof(unsigned char), complete_size);
    for(int i = 0, pos = 0; i < csize; i+= 2) {
        for(int e = 0, l = data[i], c = data[i + 1]; e < l; e++) {
            rle[pos] = c;
            pos++;
        }
    }
    return rle;
}

