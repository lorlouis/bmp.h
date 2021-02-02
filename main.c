#include <stdio.h>
#include "bmp.h"

int main(int argc, const char **argv) {
    FILE *bmpf = fopen("test.bmp", "w");
    struct bmp *bmp = bmp_new(7, bmpf);
    struct pixel16 data[6] = {
        {31},
        {0,31},
        {0,0,31},
        {15,15},
        {0,15,15},
        {0,31,31},
    };
    bmp_append_data(bmp, data, sizeof(data), bmpf);

    bmp_close(bmp, bmpf);
    struct bmp_header bmp_header = {0};
    struct BITMAPINFOHEADER bmp_info_header = {0};

    bmpf = fopen("test.bmp", "r");
    if(bmp_parse_header(&bmp_header, bmpf) < 0) {
        perror("");
    }
    if(bmp_parse_BITMAPINFOHEADER(&bmp_info_header, bmpf) < 0) {
        perror("");
    }
    printf(
        "bmp\nsize: %d\ndata_offset: %d\nwidth %d\nheight"
        ": %d\nbit per px: %d\n",
        bmp_header.size, bmp_header.data_offset, bmp_info_header.width,
        bmp_info_header.height, bmp_info_header.colour_depth);
}
