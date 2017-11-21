#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"bitmapio.h"
#include"png.h"

//ビットマップバイト幅の算出マクロ
#ifndef WIDTHBYTES
#define WIDTHBYTES(bits)    (((bits)+31)/32*4)
#endif//WIDTHBYTES

int SaveBitmapAsPngFile(char* filename, MonoBmp *bmp)
{
    png_structp png;
    png_infop info;
    png_color_8 sBIT;
    png_bytep *lines;
    FILE *outf;
    clock_t t1, t2;
    double time;

    unsigned int dwWidthBytes, cbBits;
    unsigned char * pbBits;
    int x, y;
    int nDepth;
    
    nDepth = 24; //24bit BGR
    dwWidthBytes = WIDTHBYTES(bmp->width * nDepth);
    cbBits = dwWidthBytes * bmp->height;
    //pbBits = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, cbBits);
    pbBits = malloc(cbBits);
    if (pbBits == NULL) {
        return 0;
    }

    outf = fopen(filename, "wb");
    if (!outf)
    {
        free(pbBits);
        return 0;
    }

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL)
    {
        free(pbBits);
        fclose(outf);
        return 0;
    }

    info = png_create_info_struct(png);
    if (info == NULL)
    {
        free(pbBits);
        png_destroy_write_struct(&png, NULL);
        fclose(outf);
        return 0;
    }

    lines = NULL;
    if (setjmp(png_jmpbuf(png)))
    {
        free(pbBits);
        fclose(outf);
        return 0;
    }

    png_init_io(png, outf);
    png_set_IHDR(png, info, bmp->width, bmp->height, 8, 
        (nDepth == 32 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB),
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_BASE);

    sBIT.red = 8;
    sBIT.green = 8;
    sBIT.blue = 8;
    sBIT.alpha = 0;
    png_set_sBIT(png, info, &sBIT);

    png_write_info(png, info);
    png_set_bgr(png);

    lines = (png_bytep *)malloc(sizeof(png_bytep *) * bmp->height);
  
    t1 = clock();
    for (y = 0; y < bmp->height; y++) {
        for (x = 0 ; x < bmp->width; x++){
            pbBits[dwWidthBytes * y + x*3 + 0] = 255 - bmp->map[y][x] * 255; //B
            pbBits[dwWidthBytes * y + x*3 + 1] = 255 - bmp->map[y][x] * 255; //G
            pbBits[dwWidthBytes * y + x*3 + 2] = 255 - bmp->map[y][x] * 255; //R
        }
        lines[y] = (png_bytep)&pbBits[dwWidthBytes * y];
    }
    t2 = clock();
    time = (double)(t2 - t1)/CLOCKS_PER_SEC;
    printf("cp time = %f sec\n", time);

    t1 = clock();
    png_set_compression_level(png, 9); // 0-9 6がデフォルトらしい
    png_write_image(png, lines);
    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    t2 = clock();
    time = (double)(t2 - t1)/CLOCKS_PER_SEC;
    printf("png write time = %f sec\n", time);

    free(pbBits);
    free(lines);
    fclose(outf);
    return 1;
}

int main(){
  MonoBmp bmp;
  clock_t t1, t2;
  double time;
  init_mono_bmp(&bmp);
  t1 = clock();
  load_mono_bmp_file("test.bmp", &bmp);
  t2 = clock();
  time = (double)(t2 - t1)/CLOCKS_PER_SEC;
  printf("load time = %f sec\n", time);
  
  /////////////////////////////////////////////
  SaveBitmapAsPngFile("test_out.png", &bmp);
  
  delete_mono_bmp(&bmp);
}