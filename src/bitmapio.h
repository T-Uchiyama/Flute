#ifndef __BITMAP_IO_H__
#define __BITMAP_IO_H__

#define BMP_IO_ERR_CODE int

enum{
BMP_IO_SUCCESS = 0,
BMP_IO_ERROR_FAILD_TO_OPEN_FILE,
BMP_IO_ERROR_INVALID_FILE_TYPE,
BMP_IO_ERROR_FAILD_TO_ALLOCATE_MEMORY
};



typedef struct __MonoBmp__{//白黒ビットマップ
  int width;             /* x方向のピクセル数 */
  int height;             /* y方向のピクセル数 */
  int init_width;
  int init_height;
  double angle;
  unsigned char **map;  /*実際の配列    白=0,黒=1 */
  /* ピクセルのデータは左から右，上から下へ走査した順で格納，左上が(0,0)*/
}MonoBmp;

typedef struct __BmpHeader__{
  unsigned char FileType[2];
  unsigned char FileSize[4];//ファイルサイズ
  unsigned char Reserved1[2];
  unsigned char Reserved2[2];
  unsigned char OffBits[4];//ビットマップまでのバイト数?(0でもよい)62
}BmpHeader;

//INFOタイプ情報ヘッダ
typedef struct __BmpInfo__{
  unsigned char HeaderSize[4];//40固定
  unsigned char Width[4];
  unsigned char Height[4];
  unsigned char Planes[2];//チャンネル数 常に1
  unsigned char BitCounts[2];//ピクセル毎のビット数 常に1
  unsigned char Compression[4];//圧縮形式 0固定(非圧縮)
  unsigned char DataSize[4];//画像データのサイズ
  unsigned char XPelsPerMeter[4];
  unsigned char YPelsPerMeter[4];
  unsigned char ClrUsed[4];//カラーインデックス数 常に2
  unsigned char ClrImpotant[4];//重要インデックス数
}BmpInfo;

//カラーパレット(2〜256色)
typedef struct __ColorPalette__{
  unsigned char color[256][4];//0が白,1が黒 (blue,green,red)
}ColorPalette;

//モノクロビットマップのファイルを開く
BMP_IO_ERR_CODE load_mono_bmp_file(char* filename, MonoBmp *bmp);


void init_mono_bmp(MonoBmp *bmp);
void delete_mono_bmp(MonoBmp *bmp);
int alloc_bmp(int width,int height,MonoBmp *bmp);
void write_bmp(MonoBmp *bmp,char *dst_path);
unsigned char **mem_alloc_for_bmp(int width,int height);
void free_for_bmp(unsigned char ** map);


#endif
