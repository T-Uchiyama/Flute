#include"bitmapio.h"
#include<stdlib.h>
#include<stdio.h>

#ifndef WIDTHBYTES
#define WIDTHBYTES(bits)    (((bits)+31)/32*4)
#endif//WIDTHBYTES

void init_mono_bmp(Bmp *bmp){
  bmp->width = 0;
  bmp->height = 0;
  bmp->map = NULL;
}


void delete_mono_bmp(Bmp *bmp){
  bmp->height = 0;
  bmp->width = 0;
  free(bmp->map);
  bmp->map = NULL;
}



int alloc_bmp(int width,int height,Bmp *bmp){
    int i;
    unsigned int dwWidthBytes, cbBits;
    int nDepth;
    
    bmp->width = width;
    bmp->height = height;
    
    nDepth = 24; //24bit BGR
    dwWidthBytes = WIDTHBYTES(bmp->width * nDepth);
    cbBits = dwWidthBytes * bmp->height;
    
    bmp->map = malloc(cbBits);
    if(bmp->map == NULL){
      return 0;
    }
    return 1;
}



//TODO: ヘッダ管理関係書き直し（オフセット値とか使うように変更しないと一部ファイルでやばい）
BMP_IO_ERR_CODE load_mono_bmp_file(char* filename, Bmp *bmp){
  int i,j,k;
  int bk=-1,wh=-1;//色アクセス用変数
  unsigned int temp;
  unsigned int mask;
  unsigned int dwWidthBytes, cbBits;
  int nDepth;

  BmpHeader bf;
  BmpInfo bi;

  ColorPalette bc;

  FILE *fp;
  fp = fopen(filename,"rb");

  if( fp == NULL ){
    return BMP_IO_ERROR_FAILD_TO_OPEN_FILE;
  }

  //ファイルヘッダ読み込み,ビットマップはバイナリ先頭が"BM"
  if((bf.FileType[0]=getc(fp))!='B' ||(bf.FileType[1]=getc(fp))!='M'){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;
  }
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bf.FileSize[i]=getc(fp));

  bf.Reserved1[0]=getc(fp);
  bf.Reserved1[1]=getc(fp);
  bf.Reserved2[0]=getc(fp);
  bf.Reserved2[1]=getc(fp);
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bf.OffBits[i]=getc(fp));
  //ファイルヘッダ読み込みここまで
  //情報ヘッダ読み込み
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.HeaderSize[i]=getc(fp));
  if(temp!=40){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;
  }
  bmp->width=0;//ビットマップ横ピクセル幅
  for(i=0;i<4;i++) bmp->width+=(1<<(8*i))*(bi.Width[i]=getc(fp));
  bmp->height = 0;
  for(i=0;i<4;i++) bmp->height+=(1<<(8*i))*(bi.Height[i]=getc(fp));

  temp=bi.Planes[0]=getc(fp);
  temp+=(1<<8)*(bi.Planes[1]=getc(fp));
  if(temp!=1){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;}
  temp=bi.BitCounts[0]=getc(fp);
  temp+=(1<<8)*(bi.BitCounts[1]=getc(fp));
  if(temp!=1){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;}
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.Compression[i]=getc(fp));
  if(temp!=0){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;}
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.DataSize[i]=getc(fp));
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.XPelsPerMeter[i]=getc(fp));
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.YPelsPerMeter[i]=getc(fp));
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.ClrUsed[i]=getc(fp));
  if(temp>2){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;}
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.ClrImpotant[i]=getc(fp));
  //情報ヘッダ読み込みここまで
  //カラーパレット読み込み
  for(i=0;i<4;i++) bc.color[0][i]=getc(fp);
  for(i=0;i<4;i++) bc.color[1][i]=getc(fp);
  //カラーパレット読み込みここまで
  //color[0]が白の場合
  if(bc.color[0][0]==255 && bc.color[0][1]==255 && bc.color[0][2]==255)
    wh=0;
  //color[0]が黒
  else if(bc.color[0][0]==0 && bc.color[0][1]==0 && bc.color[0][2]==0)
    bk=0;
  //color[1]が白の場合
  if(bc.color[1][0]==255 && bc.color[1][1]==255 && bc.color[1][2]==255)
    wh=1;
  //color[1]が黒
  else if(bc.color[1][0]==0 && bc.color[1][1]==0 && bc.color[1][2]==0)
    bk=1;
  if(wh+bk!=1){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;
  }
  //イメージ本体読み込み

  free(bmp->map);//どっかに移動したい,もしここでセグメントエラーが起きたら初期化忘れ
  //メモリー領域確保
  if(alloc_bmp(bmp->width,bmp->height,bmp)==0){
    fclose(fp);
    return BMP_IO_ERROR_FAILD_TO_ALLOCATE_MEMORY;
  }

  nDepth = 24; //24bit BGR
  dwWidthBytes = WIDTHBYTES(bmp->width * nDepth);

  temp=0;
  mask = ~(unsigned int)((int)0-bk);
  if(bmp->height > 0){//高さが正の値の時はデータ先頭が画像の底
    for(i=bmp->height;i--;){
      k=0;
      for(j = 0;j<(int)((bmp->width& 0xFFFFFFE0));j+= 32){
        if(fread(&temp,4,1,fp)!=1){
          goto READ_END;
        }
        temp = __builtin_bswap32(temp);
        temp ^= mask;
        for(k = 0;k<32;k++){
          //bmp->map[i][j+k]=(temp>>(31-k))&1;
          int tmp = (temp>>(31-k))&1;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 0] = 255 - tmp * 255;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 1] = 255 - tmp * 255;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 2] = 255 - tmp * 255;
        }
      }
      if(j<bmp->width){
        fread(&temp,4,1,fp);
        temp = __builtin_bswap32(temp);
        temp ^= mask;
        for(k = 0;j + k< bmp->width;k++){
          //bmp->map[i][j+k]=(temp>>(31-k))&1;
          int tmp = (temp>>(31-k))&1;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 0] = 255 - tmp * 255;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 1] = 255 - tmp * 255;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 2] = 255 - tmp * 255;
        }
      }
    }
  }else{
    bmp->height = - bmp->height;
    for(i = 0; i < bmp->height ; i++){
      k=0;
      for(j = 0;j<(int)((bmp->width& 0xFFFFFFE0));j+= 32){
        if(fread(&temp,4,1,fp)!=1){
          goto READ_END;
        }
        temp = __builtin_bswap32(temp);
        temp ^= mask;
        for(k = 0;k<32;k++){
          //bmp->map[i][j+k]=(temp>>(31-k))&1;
          int tmp = (temp>>(31-k))&1;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 0] = 255 - tmp * 255;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 1] = 255 - tmp * 255;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 2] = 255 - tmp * 255;
        }
      }
      if(j<bmp->width){
        fread(&temp,4,1,fp);
        temp = __builtin_bswap32(temp);
        temp ^= mask;
        for(k = 0;j + k< bmp->width;k++){
          //bmp->map[i][j+k]=(temp>>(31-k))&1;
          int tmp = (temp>>(31-k))&1;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 0] = 255 - tmp * 255;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 1] = 255 - tmp * 255;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 2] = 255 - tmp * 255;
        }
      }
    }
  }
  bmp->height;
  bmp->width;
  
  READ_END:;
  fclose(fp);
  return BMP_IO_SUCCESS;
}

BMP_IO_ERR_CODE load_24bit_bmp_file(char* filename, Bmp *bmp){
  int i,j,k;
  int bk=-1,wh=-1,rd=-1;//色アクセス用変数
  unsigned int temp;
  unsigned int mask;
  unsigned int dwWidthBytes, cbBits;
  int nDepth;
  int bitCount;
  int colorIndex;

  BmpHeader bf;
  BmpInfo bi;

  ColorPalette bc;

  FILE *fp;
  fp = fopen(filename,"rb");
  
  if( fp == NULL ){
    return BMP_IO_ERROR_FAILD_TO_OPEN_FILE;
  }

  //ファイルヘッダ読み込み,ビットマップはバイナリ先頭が"BM"
  if((bf.FileType[0]=getc(fp))!='B' ||(bf.FileType[1]=getc(fp))!='M'){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;
  }
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bf.FileSize[i]=getc(fp));
  bf.Reserved1[0]=getc(fp);
  bf.Reserved1[1]=getc(fp);
  bf.Reserved2[0]=getc(fp);
  bf.Reserved2[1]=getc(fp);
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bf.OffBits[i]=getc(fp));
  //ファイルヘッダ読み込みここまで
  //情報ヘッダ読み込み
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.HeaderSize[i]=getc(fp));
  if(temp!=40 && temp!=124){ 
    return BMP_IO_ERROR_INVALID_FILE_TYPE;
  }
  bmp->width=0;//ビットマップ横ピクセル幅
  for(i=0;i<4;i++) bmp->width+=(1<<(8*i))*(bi.Width[i]=getc(fp));
  bmp->height = 0;
  for(i=0;i<4;i++) bmp->height+=(1<<(8*i))*(bi.Height[i]=getc(fp));

  temp=bi.Planes[0]=getc(fp);
  temp+=(1<<8)*(bi.Planes[1]=getc(fp));
  if(temp!=1){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;}
  temp=bi.BitCounts[0]=getc(fp);
  temp+=(1<<8)*(bi.BitCounts[1]=getc(fp));
  bitCount = temp;
  if(temp!=1 && temp!=24){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;}
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.Compression[i]=getc(fp));
  if(temp!=0){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;}
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.DataSize[i]=getc(fp));
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.XPelsPerMeter[i]=getc(fp));
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.YPelsPerMeter[i]=getc(fp));
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.ClrUsed[i]=getc(fp));
    colorIndex = temp;
  if(temp>2){
    return BMP_IO_ERROR_INVALID_FILE_TYPE;}
  for(i=0,temp=0;i<4;i++)
    temp+=(1<<(8*i))*(bi.ClrImpotant[i]=getc(fp));
    if (bitCount == 24) {
        fseek(fp, 84, SEEK_CUR);
    }
  //情報ヘッダ読み込みここまで
  //カラーパレット読み込み
  if (bitCount == 1 || bitCount == 4 || bitCount == 8 || colorIndex >= 1) {
      for(i=0;i<4;i++) bc.color[0][i]=getc(fp);
      for(i=0;i<4;i++) bc.color[1][i]=getc(fp);
      //カラーパレット読み込みここまで
      //color[0]が白の場合
      if(bc.color[0][0]==255 && bc.color[0][1]==255 && bc.color[0][2]==255)
        wh=0;
      //color[0]が黒
      else if(bc.color[0][0]==0 && bc.color[0][1]==0 && bc.color[0][2]==0)
        bk=0;
      //color[1]が白の場合
      if(bc.color[1][0]==255 && bc.color[1][1]==255 && bc.color[1][2]==255)
        wh=1;
      //color[1]が黒
      else if(bc.color[1][0]==0 && bc.color[1][1]==0 && bc.color[1][2]==0)
        bk=1;
        
      if(wh+bk!=1 ){
        return BMP_IO_ERROR_INVALID_FILE_TYPE;
      } 
  }
  
  //イメージ本体読み込み

  free(bmp->map);//どっかに移動したい,もしここでセグメントエラーが起きたら初期化忘れ
  //メモリー領域確保
  if(alloc_bmp(bmp->width,bmp->height,bmp)==0){
    fclose(fp);
    return BMP_IO_ERROR_FAILD_TO_ALLOCATE_MEMORY;
  }

  nDepth = 24; //24bit BGR
  dwWidthBytes = WIDTHBYTES(bmp->width * nDepth);
  
  mask = ~-1;
  // mask = ~(unsigned int)((int)0-bk);
  
  if(bmp->height > 0){//高さが正の値の時はデータ先頭が画像の底
    for(i=bmp->height;i--;){
      k=0;
      for(j = 0;j<(int)((bmp->width& 0xFFFFFFFC));j++){
        char array[3];
        if(fread(array,3,1,fp)!=1){
          goto READ_END;
        }
        // temp = __builtin_bswap32(temp);
        // *array ^= mask;
        for(k = 0;k<1;k++){
          //bmp->map[i][j+k]=(temp>>(31-k))&1;
          // int tmp = ((int)*array>>(2+k))&1;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 0] = array[0];
          bmp->map[dwWidthBytes * i + (j+k)*3 + 1] = array[1];
          bmp->map[dwWidthBytes * i + (j+k)*3 + 2] = array[2];
        }
      }
      if(j<bmp->width){
        // array = __builtin_bswap32(array);
        for(k = 0;j + k< bmp->width;k++){
          char array[3];
          fread(array,3,1,fp);
          *array ^= mask;
          //bmp->map[i][j+k]=(temp>>(31-k))&1;
          // int tmp = ((int)*array>>(2+k))&1;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 0] = array[0];
          bmp->map[dwWidthBytes * i + (j+k)*3 + 1] = array[1];
          bmp->map[dwWidthBytes * i + (j+k)*3 + 2] = array[2];
        }
        if (k == 1 || k == 2 || k == 3) {
            fseek(fp, k, SEEK_CUR); 
        }
      }
    }
  }else{
    bmp->height = - bmp->height;
    for(i = 0; i < bmp->height ; i++){
      k=0;
      char array[3];
      for(j = 0;j<(int)((bmp->width& 0xFFFFFFFC));j++){
        if(fread(array,3,1,fp)!=1){
          goto READ_END;
        }
        // array = __builtin_bswap32(array);
        *array ^= mask;
        for(k = 0;k<1;k++){
          //bmp->map[i][j+k]=(temp>>(31-k))&1;
          // int tmp = ((int)*array>>(31-k))&1;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 0] = array[0];
          bmp->map[dwWidthBytes * i + (j+k)*3 + 1] = array[1];
          bmp->map[dwWidthBytes * i + (j+k)*3 + 2] = array[2];
        }
      }
      if(j<bmp->width){
        // array = __builtin_bswap32(array);
        for(k = 0;j + k< bmp->width;k++){
            char array[3];
            fread(array,3,1,fp);
            *array ^= mask;
          //bmp->map[i][j+k]=(temp>>(31-k))&1;
          // int tmp = ((int)*array>>(2+k))&1;
          bmp->map[dwWidthBytes * i + (j+k)*3 + 0] = array[0];
          bmp->map[dwWidthBytes * i + (j+k)*3 + 1] = array[1];
          bmp->map[dwWidthBytes * i + (j+k)*3 + 2] = array[2];
        }
        if (k == 1 || k == 2 || k == 3) {
            fseek(fp, k, SEEK_CUR); 
        }
      }
    }
  }
  bmp->height;
  bmp->width;
  
  READ_END:;
  fclose(fp);
  return BMP_IO_SUCCESS;
}

/*
//あとでなおす
//モノクロビットマップを出力
void write_bmp(Bmp *bmp,char *dst_path){
  FILE *fp;
  int i,j,k;
  unsigned int fs;
  unsigned int tmp;

  if(bmp == NULL){
    return;
  }
  if(bmp->map == NULL){
    return;
  }
  _tfopen_s(&fp,dst_path,_T("wb"));
  if(fp == NULL){
    return;
  }
  fs=(bmp->width/32*4+4)*bmp->height+62;

  fputc('B',fp);
  fputc('M',fp);
  for(i=0;i<4;i++)//ファイルサイズ
    fputc((unsigned char)(fs>>(i<<3)),fp);
  for(i=0;i<4;i++)
    fputc(0,fp);
  fputc(62,fp);//オフセット
  for(i=0;i<3;i++)
    fputc(0,fp);
  fputc(40,fp);//情報ヘッダサイズ
  for(i=0;i<3;i++)
    fputc(0,fp);
  for(i=0;i<4;i++)//幅
    fputc((unsigned char)(bmp->width>>(i<<3)),fp);
  for(i=0;i<4;i++)//高さ
    fputc((unsigned char)(bmp->height>>(i<<3)),fp);
  fputc(1,fp);
  fputc(0,fp);
  fputc(1,fp);
  for(i=0;i<5;i++)
    fputc(0,fp);
  fs=fs-62;
  for(i=0;i<4;i++)//イメージサイズ
    fputc((unsigned char)(fs>>(i<<3)),fp);
  for(i=0;i<16;i++)
    fputc(0,fp);

  fputc(0,fp);
  fputc(0,fp);
  fputc(0,fp);
  fputc(0,fp);

  fputc(255,fp);
  fputc(255,fp);
  fputc(255,fp);
  fputc(0,fp);

  for(i=0;i<bmp->height;i++){//あとでなおす
    tmp=0;
    for(j=0;j<bmp->width;j++){
      tmp|=(((unsigned int)(bmp->map[bmp->height-i-1][j])^1)<<(7-(j&7)))<<((j>>3)<<3);
      if((j&31)==31){
        for(k=0;k<4;k++)
          fputc((unsigned char)(tmp>>(k<<3)),fp);
        tmp=0;
      }
    }
    if((j&31)!=0){
      for(k=0;k<4;k++)
        fputc((unsigned char)(tmp>>(k<<3)),fp);
    }
  }
  fclose(fp);
}
*/