#ifndef BMP_H
#define BMP_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>


enum compression_method {
    BI_RGB,
};

#pragma pack(push, 1)
struct bmp_header {
    uint8_t type[2];
    uint32_t size;
    uint16_t reserved[2];
    /* header + dib_header */
    uint32_t data_offset;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BITMAPINFOHEADER {
    uint32_t dib_header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;                 /* set to 1 */
    uint16_t colour_depth;           /*bit per px*/
    uint32_t compression_method;
    uint32_t image_size;             /* in bytes */
    int32_t horizontal_px_per_meter; /* set to 0 */
    int32_t vertical_px_per_meter;   /* set to 0 */
    uint32_t nb_colours_in_palette;
    uint32_t nb_important_colours;   /* set to 0 */
};
#pragma pack(pop)

#pragma pack(push, 1)
struct pixel16 {
    unsigned int B :5;
    unsigned int G :5;
    unsigned int R :5;
    unsigned int A :1;
};
#pragma pack(pop)

struct bmp {
    int32_t width;
    int32_t height;
    uint32_t size;  /* size of the data */
};

/* writes the skeleton of a BMP header to file
 * and mallocs a bmp struct
 * MUST BE CALLED FIRST */
static struct bmp* bmp_new(int32_t width, FILE *file) {
    struct bmp_header header = {
        .type = {'B', 'M'},
        .size = 0,  /* will be updated when close the file */
        .reserved = {0, 0},
        .data_offset = sizeof(struct bmp_header)
            + sizeof(struct BITMAPINFOHEADER),
    };

    if(fwrite((void*)&header, sizeof(struct bmp_header), 1, file) < 0) {
        return 0;
    }

    struct BITMAPINFOHEADER dib_header = {
        .dib_header_size = sizeof(struct BITMAPINFOHEADER),
        .width = width,
        .height = 0, /* will be updated when we close the file */
        .planes = 1,
        .colour_depth = 16,
        .compression_method = 0,
        .image_size = 0,  /* will be set when we close the file */
        .horizontal_px_per_meter = 0,
        .vertical_px_per_meter = 0,
        .nb_colours_in_palette = 0,
        .nb_important_colours = 0,
    };

    if(fwrite((void*)&dib_header, sizeof(struct BITMAPINFOHEADER), 1, file)<0){
        return 0;
    }
    struct bmp *bmp = calloc(1, sizeof(struct bmp));
    bmp->width = width;
    bmp->height = 0;
    return bmp;
}

static int pad_with_null_byte(FILE *file, size_t nb_bytes) {
    uint8_t filler = 0;
    for(size_t i = 0; i < nb_bytes; i++) {
        if(fwrite((void*)&filler, 1, 1, file) < 0) return -1;
    }
    return 0;
}

/* CALL bmp_new FIRST
 * appends data to file
 * updates bmp.size and bmp.height if it worked
 * otherwise returns -1 on write error */
static int bmp_append_data(struct bmp *bmp, struct pixel16 *data,
                    size_t data_size, FILE *file) {
    /* you cannot call append  */
    if(!ftell(file)) return -1;

    /* TODO optimise */
    int nb_px = (bmp->size * bmp->height) / sizeof(struct pixel16);
    int empty_px_on_line = nb_px % bmp->width;
    size_t fill_size = empty_px_on_line * sizeof(struct pixel16);

    /* do not go out of bounds */
    if(data_size < fill_size) fill_size = data_size;

    if(fwrite((void*)(data), fill_size, 1,file) < 0 ) return -1;
    bmp->size += fill_size;
    data_size -= fill_size;
    data += fill_size/sizeof(struct pixel16);
    while(data_size > 0) {

        /* check if the line can be completed */
        if(bmp->width*sizeof(struct pixel16) > data_size) {
            fill_size = data_size;
        }
        /* fill the whole line with data */
        else {
            fill_size = bmp->width * sizeof(struct pixel16);
            bmp->height++;
        }
        if(fwrite((void*)(data), fill_size, 1,file) < 0 ) return -1;
        data += fill_size/sizeof(struct pixel16);
        bmp->size += fill_size;
        data_size -= fill_size;
    }
    return 0;
}

/* updates the data in the file and closes it
 * returns -1 on error
 * on error *bmp is not freed and *file is not closed */
static int bmp_close(struct bmp *bmp, FILE *file) {
    /* padding a row with black pixels */
    /* TODO remove the padding bytes */
    int tmp_size = (
            bmp->width - (
                bmp->size
                - (2 * bmp->height * (bmp->width % 2)
                    / sizeof(struct pixel16))
            ) % bmp->width);

    if(tmp_size && tmp_size % bmp->width) {
        pad_with_null_byte(file, tmp_size*2);
        bmp->size += tmp_size*2;
        bmp->height++;
    }

    if(bmp->width % 2) {
        pad_with_null_byte(file, 2);
        bmp->size += 2;
    }

    uint32_t fullsize = bmp->size + sizeof(struct bmp_header)
                        + sizeof(struct BITMAPINFOHEADER);

    /* updating the size of the file */
    if(fseek(file, offsetof(struct bmp_header, size), SEEK_SET) < 0) return -1;
    if(fwrite((void*)&fullsize, sizeof(uint32_t), 1, file) < 0 ) return -1;

    /* updating the height */
    if(fseek(file,
                sizeof(struct bmp)
                + offsetof(struct BITMAPINFOHEADER, height) + 2,
                SEEK_SET) < 0) {
        return -1;
    }
    if(fwrite((void*)&bmp->height, sizeof(uint32_t), 1, file) < 0 ) return -1;

    /* updating the size of the image */
    if(fseek(file,
                sizeof(struct bmp)
                + offsetof(struct BITMAPINFOHEADER, image_size) + 2,
                SEEK_SET) < 0) {
        return -1;
    }
    if(fwrite((void*)&bmp->size, sizeof(uint32_t), 1, file) < 0 ) return -1;
    free(bmp);
    return fclose(file);
}

/* parses the bmp_header
 * returns -1 on file error */
static int bmp_parse_header(
        struct bmp_header *bmp_header,
        FILE *file) {
    if(fread(bmp_header, sizeof(struct bmp_header), 1, file) < 0)
        return -1;
    return 0;
}

static int bmp_parse_BITMAPINFOHEADER(
        struct BITMAPINFOHEADER *info_header, FILE *file) {
    fseek(file, sizeof(struct bmp_header), SEEK_SET);
    if(info_header) {
        if(fread(info_header, sizeof(struct BITMAPINFOHEADER), 1, file) < 0) {
            return -1;
        }
    }
    return 0;
}
#endif
