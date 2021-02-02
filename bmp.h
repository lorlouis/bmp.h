#ifndef BMP_H
#define BMP_H 1

#include <stdio.h>
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
 * bmp.width needs to be set to a positive integer
 * MUST BE CALLED FIRST */
struct bmp* bmp_new(int32_t width, FILE *file);

/* CALL bmp_write_header FIRST
 * appends data to file
 * updates bmp.size and bmp.height if it worked
 * otherwise returns -1 on write error */
int bmp_append_data(struct bmp *bmp, struct pixel16 *data,
                    size_t data_size, FILE *file);

/* updates the data in the file and closes it
 * returns -1 on error */
int bmp_close(struct bmp *bmp, FILE *file);

int bmp_parse_header(struct bmp_header *bmp_header, FILE *file);

int bmp_parse_BITMAPINFOHEADER(struct BITMAPINFOHEADER *info_header, FILE *file);
#endif
