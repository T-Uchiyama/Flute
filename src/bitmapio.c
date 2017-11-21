#include"bitmapio.h"
#include<stdlib.h>
#include<stdio.h>


void init_mono_bmp(MonoBmp *bmp){
  bmp->width = 0;
  bmp->height = 0;
  bmp->map = NULL;
}

void free_for_bmp(unsigned char ** map){
  if(map != NULL){
    if(map[0] != NULL){
      free(map[0]);
    }
    free(map);
  }
}


void delete_mono_bmp(MonoBmp *bmp){
  bmp->height = 0;
  bmp->width = 0;
  free_for_bmp(bmp->map);
  bmp->map = NULL;
}

unsigned char **mem_alloc_for_bmp(int width,int height){
  unsigned char ** ret;
  int i;
  ret=(unsigned char**)malloc(sizeof(char*)*(height));
  if(ret == NULL){
    return NULL;
  }
  ret[0]=(unsigned char*)malloc(sizeof(char)*(height)*((width+31)&0xFFFFFFE0));
  if(ret[0] == NULL){
    free(ret);
    return NULL;
  }
  for(i=1;i<height;i++) ret[i]=ret[i-1]+((width+31)&0xFFFFFFE0);
  return ret;
}


int alloc_bmp(int width,int height,MonoBmp *bmp){
  int i;
  bmp->width = width;
  bmp->height = height;
  bmp->map=mem_alloc_for_bmp(width,height);
  if(bmp->map == NULL){
    return 0;
  }
    return 1;
}



//TODO: ヘッダ管理関係書き直し（オフセット値とか使うように変更しないと一部ファイルでやばい）
BMP_IO_ERR_CODE load_mono_bmp_file(char* filename, MonoBmp *bmp){
  int i,j,k;
  int bk=-1,wh=-1;//色アクセス用変数
  unsigned int temp;
  unsigned int mask;

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

  free_for_bmp(bmp->map);//どっかに移動したい,もしここでセグメントエラーが起きたら初期化忘れ
  //メモリー領域確保
  if(alloc_bmp(bmp->width,bmp->height,bmp)==0){
    fclose(fp);
    return BMP_IO_ERROR_FAILD_TO_ALLOCATE_MEMORY;
  }

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
          bmp->map[i][j+k]=(temp>>(31-k))&1;
        }
      }
      if(j<bmp->width){
        fread(&temp,4,1,fp);
        temp = __builtin_bswap32(temp);
        temp ^= mask;
        for(k = 0;j + k< bmp->width;k++){
          bmp->map[i][j+k]=(temp>>(31-k))&1;
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
          bmp->map[i][j+k]=(temp>>(31-k))&1;
        }
      }
      if(j<bmp->width){
        fread(&temp,4,1,fp);
        temp = __builtin_bswap32(temp);
        temp ^= mask;
        for(k = 0;j + k< bmp->width;k++){
          bmp->map[i][j+k]=(temp>>(31-k))&1;
        }
      }
    }
  }
  bmp->init_height = bmp->height;
  bmp->init_width  = bmp->width;
  bmp->angle = 0.0;
  
  READ_END:;
  fclose(fp);
  return BMP_IO_SUCCESS;
}


/*
//あとでなおす
//モノクロビットマップを出力
void write_bmp(MonoBmp *bmp,char *dst_path){
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