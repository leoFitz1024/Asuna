/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 2.7
 */
/*
 *                      ATOK Library Sample
 *
 *                         Version 0.10
 *                           Shift-JIS
 *
 *      Copyright (C) 2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *      0.10
 */
#include <stdio.h>

// TIM2ファイルヘッダの??夛を?O協

#ifdef R5900

// PS2?h廠のとき
#include <eekernel.h>
#include <sifdev.h>
#include <libgraph.h>
#include "tim2.h"

// プロトタイプ傚冱
static void Tim2LoadTexture(int psm, u_int tbp, int tbw, int sx, int sy, u_long128 *pImage);
static int  Tim2CalcBufWidth(int psm, int w);
static int  Tim2CalcBufSize(int psm, int w, int h);
static int  Tim2GetLog2(int n);

#else	// R5900

// 掲PS2?h廠のとき

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4200)
#endif	// WIN32
#include "tim2.h"
#ifdef WIN32
#pragma warning(pop)
#endif	// WIN32

#endif	// R5900



// TIM2ファイルのファイルヘッダをチェックする
// 哈方
// pTim2    TIM2侘塀のデ?`タの枠?^アドレス
// 卦り??
//          0のときエラ?`
//          1のとき屎械?K阻(TIM2)
//          2のとき屎械?K阻(CLUT2)
int Tim2CheckFileHeaer(void *pTim2)
{
	TIM2_FILEHEADER *pFileHdr = (TIM2_FILEHEADER *)pTim2;
	int i;

	// TIM2シグネチャをチェック
	if(pFileHdr->FileId[0]=='T' || pFileHdr->FileId[1]=='I' || pFileHdr->FileId[2]=='M' || pFileHdr->FileId[3]=='2') {
		// TIM2だったとき
		i = 1;
	} else if(pFileHdr->FileId[0]=='C' || pFileHdr->FileId[1]=='L' || pFileHdr->FileId[2]=='T' || pFileHdr->FileId[3]=='2') {
		// CLUT2だったとき
		i = 2;
	} else {
		// イリ?`ガルな?R?e猟忖だったとき
		printf("Tim2CheckFileHeaer: TIM2 is broken %02X,%02X,%02X,%02X\n",
					pFileHdr->FileId[0], pFileHdr->FileId[1], pFileHdr->FileId[2], pFileHdr->FileId[3]);
		return(0);
	}

	// TIM2ファイルフォ?`マットバ?`ジョン,フォ?`マットIDをチェック
	if(!(pFileHdr->FormatVersion==0x03 ||
	    (pFileHdr->FormatVersion==0x04 && (pFileHdr->FormatId==0x00 || pFileHdr->FormatId==0x01)))) {
		printf("Tim2CheckFileHeaer: TIM2 is broken (2)\n");
		return(0);
	}
	return(i);
}



// 峺協された桑催のピクチャヘッダを誼る
// 哈方
// pTim2    TIM2侘塀のデ?`タの枠?^アドレス
// imgno    採桑朕のピクチャヘッダを歌孚するか峺協
// 卦り??
//          ピクチャヘッダへのポインタ
TIM2_PICTUREHEADER *Tim2GetPictureHeader(void *pTim2, int imgno)
{
	TIM2_FILEHEADER *pFileHdr = (TIM2_FILEHEADER *)pTim2;
	TIM2_PICTUREHEADER *pPictHdr;
	int i;

	// ピクチャ桑催をチェック
	if(imgno>=pFileHdr->Pictures) {
		printf("Tim2GetPictureHeader: Illegal image no.(%d)\n", imgno);
		return(NULL);
	}

	if(pFileHdr->FormatId==0x00) {
		// フォ?`マットIDが0x00のとき、16バイトアラインメント
		pPictHdr = (TIM2_PICTUREHEADER *)((char *)pTim2 + sizeof(TIM2_FILEHEADER));
	} else {
		// フォ?`マットIDが0x01のとき、128バイトアラインメント
		pPictHdr = (TIM2_PICTUREHEADER *)((char *)pTim2 + 0x80);
	}

	// 峺協されたピクチャ桑催までスキップ
	for(i=0; i<imgno; i++) {
		pPictHdr = (TIM2_PICTUREHEADER *)((char *)pPictHdr + pPictHdr->TotalSize);
	}
	return(pPictHdr);
}


// ピクチャデ?`タがCLUT2デ?`タであるかどうか登?e
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// ??り??
//          0のときTIM2
//          1のときCLUT2
int Tim2IsClut2(TIM2_PICTUREHEADER *ph)
{
	// ピクチャヘッダのMipMapTexturesメンバを登?e
	if(ph->MipMapTextures==0) {
		// テクスチャ旦方が0のとき、CLUT2デ?`タ
		return(1);
	} else {
		// テクスチャ旦方が1旦參貧のとき、TIM2デ?`タ
		return(0);
	}
}


// MIPMAPレベルごとのテクスチャサイズを誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// mipmap:  MIPMAPテクスチャレベル(慧抹雫艶??0-圻兵寄弌)
// pWidth:  Xサイズを鞭け函るためのint侏?篳?へポインタ
// pHeight: Yサイズを鞭け函るためのint侏?篳?へポインタ
// ??り??
//          MIPMAPテクスチャのサイズ
int Tim2GetMipMapPictureSize(TIM2_PICTUREHEADER *ph, int mipmap, int *pWidth, int *pHeight)
{
	int w, h, n;
	w = ph->ImageWidth>>mipmap;
	h = ph->ImageHeight>>mipmap;
	if(pWidth) {
		*pWidth  = w;
	}
	if(pHeight) {
		*pHeight = h;
	}

	n = w * h;
	switch(ph->ImageType) {
		case TIM2_RGB16:	n *= 2;		break;
		case TIM2_RGB24:	n *= 3;		break;
		case TIM2_RGB32:	n *= 4;		break;
		case TIM2_IDTEX4:	n /= 2;		break;
		case TIM2_IDTEX8:				break;
	}

	// MIPMAPテクスチャサイズはファイルヘッダのFormatIdの峺協にかかわらず、
	// 械に16バイトアラインメント廠順で屁双される
	n = (n + 15) & ~15;
	return(n);
}


// MIPMAPヘッダのアドレス,サイズを誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// pSize:   MIPMAPヘッダのサイズを鞭け函るためのint侏?篳?へポインタ
//          サイズが駅勣のないときはNULLに
// ??り??
//          NULLのときMIPMAPヘッダなし
//          NULLでないとき、MIPMAPヘッダの枠?^アドレス
TIM2_MIPMAPHEADER *Tim2GetMipMapHeader(TIM2_PICTUREHEADER *ph, int *pSize)
{
	TIM2_MIPMAPHEADER *pMmHdr;

	static const char mmsize[] = {
		 0,  // テクスチャ0旦(CLUT2デ?`タのとき)
		 0,  // LV0テクスチャのみ(MIPMAPヘッダなし)
		32,  // LV1 MipMapまで
		32,  // LV2 MipMapまで
		32,  // LV3 MipMapまで
		48,  // LV4 MipMapまで
		48,  // LV5 MipMapまで
		48   // LV6 MipMapまで
	};

	if(ph->MipMapTextures>1) {
		pMmHdr = (TIM2_MIPMAPHEADER *)((char *)ph + sizeof(TIM2_PICTUREHEADER));
	} else {
		pMmHdr = NULL;
	}

	if(pSize) {
		// ????ヘッダがなかった??栽、
		*pSize = mmsize[ph->MipMapTextures];
	}
	return(pMmHdr);
}


// ユ?`ザ?`スペ?`スのアドレス,サイズを誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// pSize:   ユ?`ザ?`スペ?`スのサイズを鞭け函るためのint侏?篳?へポインタ
//          サイズが駅勣のないときはNULLに
// ??り??
//          NULLのときユ?`ザ?`スペ?`スなし
//          NULLでないとき、ユ?`ザ?`スペ?`スの枠?^アドレス
void *Tim2GetUserSpace(TIM2_PICTUREHEADER *ph, int *pSize)
{
	void *pUserSpace;

	static const char mmsize[] = {
		sizeof(TIM2_PICTUREHEADER)     ,	// テクスチャ0旦(CLUT2デ?`タのとき)
		sizeof(TIM2_PICTUREHEADER)     ,	// LV0テクスチャのみ(MIPMAPヘッダなし)
		sizeof(TIM2_PICTUREHEADER) + 32,	// LV1 MipMapまで
		sizeof(TIM2_PICTUREHEADER) + 32,	// LV2 MipMapまで
		sizeof(TIM2_PICTUREHEADER) + 32,	// LV3 MipMapまで
		sizeof(TIM2_PICTUREHEADER) + 48,	// LV4 MipMapまで
		sizeof(TIM2_PICTUREHEADER) + 48,	// LV5 MipMapまで
		sizeof(TIM2_PICTUREHEADER) + 48 	// LV6 MipMapまで
	};

	// ヘッダサイズを?{べる
	if(ph->HeaderSize==mmsize[ph->MipMapTextures]) {
		// ユ?`ザ?`スペ?`スが贋壓しないとき
		if(pSize) *pSize = 0;
		return(NULL);
	}

	// ユ?`ザ?`スペ?`スが贋壓するとき
	pUserSpace = (void *)((char *)ph + mmsize[ph->MipMapTextures]);
	if(pSize) {
		// ユ?`ザ?`スペ?`スのサイズを??麻
		// ????ヘッダがある??栽は、そちらからト?`タルサイズを函誼
		TIM2_EXHEADER *pExHdr;

		pExHdr = (TIM2_EXHEADER *)pUserSpace;
		if(pExHdr->ExHeaderId[0]!='e' ||
			pExHdr->ExHeaderId[1]!='X' ||
			pExHdr->ExHeaderId[2]!='t' ||
			pExHdr->ExHeaderId[3]!=0x00) {

			// ????ヘッダがなかった??栽、
			*pSize = (ph->HeaderSize - mmsize[ph->MipMapTextures]);
		} else {
			// ????ヘッダがあった??栽
			*pSize = pExHdr->UserSpaceSize;
		}
	}
	return(pUserSpace);
}


// ユ?`ザ?`デ?`タのアドレス,サイズを誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// pSize:   ユ?`ザ?`デ?`タのサイズを鞭け函るためのint侏?篳?へポインタ
//          サイズが駅勣のないときはNULLに
// ??り??
//          NULLのときユ?`ザ?`デ?`タなし
//          NULLでないとき、ユ?`ザ?`デ?`タの枠?^アドレス
void *Tim2GetUserData(TIM2_PICTUREHEADER *ph, int *pSize)
{
	void *pUserSpace;
	TIM2_EXHEADER *pExHdr;

	pUserSpace = Tim2GetUserSpace(ph, pSize);
	if(pUserSpace==NULL) {
		// ユ?`ザ?`スペ?`スが贋壓しなかったとき
		return(NULL);
	}

	// ユ?`ザ?`スペ?`スに????ヘッダがあるかどうかチェック
	pExHdr = (TIM2_EXHEADER *)pUserSpace;
	if(pExHdr->ExHeaderId[0]!='e' ||
		pExHdr->ExHeaderId[1]!='X' ||
		pExHdr->ExHeaderId[2]!='t' ||
		pExHdr->ExHeaderId[3]!=0x00) {

		// ????ヘッダが??つからなかった??栽
		return(pUserSpace);
	}

	// ????ヘッダが??つかった??栽
	if(pSize) {
		// ユ?`ザ?`デ?`タ何蛍のサイズを卦す
		*pSize = pExHdr->UserDataSize;
	}
	return((char *)pUserSpace + sizeof(TIM2_EXHEADER));
}


// ユ?`ザ?`スペ?`スに鯉?{されたコメント猟忖双の枠?^アドレスを誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// ??り??
//          NULLのときコメント猟忖双なし
//          NULLでないとき、コメント猟忖双(ASCIZ)の枠?^アドレス
char *Tim2GetComment(TIM2_PICTUREHEADER *ph)
{
	void *pUserSpace;
	TIM2_EXHEADER *pExHdr;

	pUserSpace = Tim2GetUserSpace(ph, NULL);
	if(pUserSpace==NULL) {
		// ユ?`ザ?`スペ?`スが贋壓しなかったとき
		return(NULL);
	}

	// ユ?`ザ?`スペ?`スに????ヘッダがあるかどうかチェック
	pExHdr = (TIM2_EXHEADER *)pUserSpace;
	if(pExHdr->ExHeaderId[0]!='e' ||
		pExHdr->ExHeaderId[1]!='X' ||
		pExHdr->ExHeaderId[2]!='t' ||
		pExHdr->ExHeaderId[3]!=0x00) {

		// ????ヘッダが??つからなかった??栽
		return(NULL);
	}

	// ????ヘッダ贋壓していたとき
	if(pExHdr->UserSpaceSize==((sizeof(TIM2_EXHEADER) + pExHdr->UserDataSize))) {
		// ユ?`ザ?`スペ?`スの嗤吭なサイズが、????ヘッダとユ?`ザ?`デ?`タの栽??サイズに吉しかったとき
		// コメント猟忖双は贋壓しないと登僅できる
		return(NULL);
	}

	// コメントが??つかった??栽
	return((char *)pUserSpace + sizeof(TIM2_EXHEADER) + pExHdr->UserDataSize);
}



// 峺協したMIPMAPレベルのイメ?`ジデ?`タの枠?^アドレスを誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// mipmap:  MIPMAPテクスチャレベル
// ??り??
//          NULLのとき、??輝するイメ?`ジデ?`タなし
//          NULLでないとき、イメ?`ジデ?`タの枠?^アドレス
void *Tim2GetImage(TIM2_PICTUREHEADER *ph, int mipmap)
{
	void *pImage;
	TIM2_MIPMAPHEADER *pm;
	int i;

	if(mipmap>=ph->MipMapTextures) {
		// 峺協されたレベルのMIPMAPテクスチャは贋壓しない
		return(NULL);
	}

	// イメ?`ジデ?`タの枠?^アドレスを??麻
	pImage = (void *)((char *)ph + ph->HeaderSize);
	if(ph->MipMapTextures==1) {
		// LV0テクスチャのみの??栽
		return(pImage);
	}

	// MIPMAPテクスチャが贋壓している??栽
	pm = (TIM2_MIPMAPHEADER *)((char *)ph + sizeof(TIM2_PICTUREHEADER));
	for(i=0; i<mipmap; i++) {
		pImage = (void *)((char *)pImage + pm->MMImageSize[i]);
	}
	return(pImage);
}


// CLUTデ?`タの枠?^アドレスを誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// ??り??
//          NULLのとき、??輝するCLUTデ?`タなし
//          NULLでないとき、CLUTデ?`タの枠?^アドレス
void *Tim2GetClut(TIM2_PICTUREHEADER *ph)
{
	void *pClut;
	if(ph->ClutColors==0) {
		// CLUTデ?`タ何を??撹する弼方が0のとき
		pClut = NULL;
	} else {
		// CLUTデ?`タが贋壓するとき
		pClut = (void *)((char *)ph + ph->HeaderSize + ph->ImageSize);
	}
	return(pClut);
}


// CLUTカラ?`を誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// clut:    CLUTセットの峺協
// no:      採桑朕のインデクスを函誼するか峺協
// ??り??
//          RGBA32フォ?`マットで弼を卦す
//          clut,no吉の峺協にエラ?`があるとき、0x00000000(?\)を卦す
unsigned int Tim2GetClutColor(TIM2_PICTUREHEADER *ph, int clut, int no)
{
	unsigned char *pClut;
	int n;
	unsigned char r, g, b, a;

	pClut = (unsigned char *)Tim2GetClut(ph);
	if(pClut==NULL) {
		// CLUTデ?`タがなかったとき
		return(0);
	}

	// まず、採桑朕の弼デ?`タか??麻
	switch(ph->ImageType) {
		case TIM2_IDTEX4:	n = clut*16 + no;	break;
		case TIM2_IDTEX8:	n = clut*256 + no;	break;
		default:         	return(0);    // 音屎なピクセルカラ?`のとき
	}
	if(n>ph->ClutColors) {
		// 峺協されたCLUT桑催,インデクスの弼デ?`タが贋壓しなかったとき
		return(0);
	}

	// CLUT何のフォ?`マットによっては、塘崔が?Kび紋えられている辛嬬來がある
	switch((ph->ClutType<<8) | ph->ImageType) {
		case (((TIM2_RGB16 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,16ビット,?Kび紋えずみ
		case (((TIM2_RGB24 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,24ビット,?Kび紋えずみ
		case (((TIM2_RGB32 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,32ビット,?Kび紋えずみ
		case (( TIM2_RGB16        <<8) | TIM2_IDTEX8): // 256弼,CSM1,16ビット,?Kび紋えずみ
		case (( TIM2_RGB24        <<8) | TIM2_IDTEX8): // 256弼,CSM1,24ビット,?Kび紋えずみ
		case (( TIM2_RGB32        <<8) | TIM2_IDTEX8): // 256弼,CSM1,32ビット,?Kび紋えずみ
			if((n & 31)>=8) {
				if((n & 31)<16) {
					n += 8;    // +8゛15を+16゛23に
				} else if((n & 31)<24) {
					n -= 8;    // +16゛23を+8゛15に
				}
			}
			break;
	}

	// CLUT何のピクセルフォ?`マットに児づいて、弼デ?`タを誼る
	switch(ph->ClutType & 0x3F) {
		case TIM2_RGB16:
			// 16bitカラ?`のとき
			r = (unsigned char)((((pClut[n*2 + 1]<<8) | pClut[n*2])<<3) & 0xF8);
			g = (unsigned char)((((pClut[n*2 + 1]<<8) | pClut[n*2])>>2) & 0xF8);
			b = (unsigned char)((((pClut[n*2 + 1]<<8) | pClut[n*2])>>7) & 0xF8);
			a = (unsigned char)((((pClut[n*2 + 1]<<8) | pClut[n*2])>>8) & 0x80);
			break;

		case TIM2_RGB24:
			// 24bitカラ?`のとき
			r = pClut[n*3];
			g = pClut[n*3 + 1];
			b = pClut[n*3 + 2];
			a = 0x80;
			break;

		case TIM2_RGB32:
			// 32bitカラ?`のとき
			r = pClut[n*4];
			g = pClut[n*4 + 1];
			b = pClut[n*4 + 2];
			a = pClut[n*4 + 3];
			break;

		default:
			// 音屎なピクセルフォ?`マットの??栽
			r = 0;
			g = 0;
			b = 0;
			a = 0;
			break;
	}
	return((a<<24) | (g<<16) | (b<<8) | r);
}


// CLUTカラ?`を?O協する
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// clut:    CLUTセットの峺協
// no:      採桑朕のインデクスを?O協するか峺協
// color:   ?O協する弼(RGB32フォ?`マット)
// ??り??
//          RGBA32フォ?`マットで硬い弼を卦す
//          clut,no吉の峺協にエラ?`があるとき、0x00000000(?\)を卦す
unsigned int Tim2SetClutColor(TIM2_PICTUREHEADER *ph, int clut, int no, unsigned int newcolor)
{
	unsigned char *pClut;
	unsigned char r, g, b, a;
	int n;

	pClut = (unsigned char *)Tim2GetClut(ph);
	if(pClut==NULL) {
		// CLUTデ?`タがなかったとき
		return(0);
	}

	// まず、採桑朕の弼デ?`タか??麻
	switch(ph->ImageType) {
		case TIM2_IDTEX4:	n = clut*16 + no;	break;
		case TIM2_IDTEX8:	n = clut*256 + no;	break;
		default:         	return(0);    // 音屎なピクセルカラ?`のとき
	}
	if(n>ph->ClutColors) {
		// 峺協されたCLUT桑催,インデクスの弼デ?`タが贋壓しなかったとき
		return(0);
	}

	// CLUT何のフォ?`マットによっては、塘崔が?Kび紋えられている辛嬬來がある
	switch((ph->ClutType<<8) | ph->ImageType) {
		case (((TIM2_RGB16 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,16ビット,?Kび紋えずみ
		case (((TIM2_RGB24 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,24ビット,?Kび紋えずみ
		case (((TIM2_RGB32 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,32ビット,?Kび紋えずみ
		case (( TIM2_RGB16        <<8) | TIM2_IDTEX8): // 256弼,CSM1,16ビット,?Kび紋えずみ
		case (( TIM2_RGB24        <<8) | TIM2_IDTEX8): // 256弼,CSM1,24ビット,?Kび紋えずみ
		case (( TIM2_RGB32        <<8) | TIM2_IDTEX8): // 256弼,CSM1,32ビット,?Kび紋えずみ
			if((n & 31)>=8) {
				if((n & 31)<16) {
					n += 8;    // +8゛15を+16゛23に
				} else if((n & 31)<24) {
					n -= 8;    // +16゛23を+8゛15に
				}
			}
			break;
	}

	// CLUT何のピクセルフォ?`マットに児づいて、弼デ?`タを誼る
	switch(ph->ClutType & 0x3F) {
		case TIM2_RGB16:
			// 16bitカラ?`のとき
			{
				unsigned char rr, gg, bb, aa;
				r = (unsigned char)((((pClut[n*2 + 1]<<8) | pClut[n*2])<<3) & 0xF8);
				g = (unsigned char)((((pClut[n*2 + 1]<<8) | pClut[n*2])>>2) & 0xF8);
				b = (unsigned char)((((pClut[n*2 + 1]<<8) | pClut[n*2])>>7) & 0xF8);
				a = (unsigned char)((((pClut[n*2 + 1]<<8) | pClut[n*2])>>8) & 0x80);

				rr = (unsigned char)((newcolor>>3)  & 0x1F);
				gg = (unsigned char)((newcolor>>11) & 0x1F);
				bb = (unsigned char)((newcolor>>19) & 0x1F);
				aa = (unsigned char)((newcolor>>31) & 1);

				pClut[n*2]     = (unsigned char)((((aa<<15) | (bb<<10) | (gg<<5) | rr))    & 0xFF);
				pClut[n*2 + 1] = (unsigned char)((((aa<<15) | (bb<<10) | (gg<<5) | rr)>>8) & 0xFF);
			}
			break;

		case TIM2_RGB24:
			// 24bitカラ?`のとき
			r = pClut[n*3];
			g = pClut[n*3 + 1];
			b = pClut[n*3 + 2];
			a = 0x80;
			pClut[n*3]     = (unsigned char)((newcolor)     & 0xFF);
			pClut[n*3 + 1] = (unsigned char)((newcolor>>8)  & 0xFF);
			pClut[n*3 + 2] = (unsigned char)((newcolor>>16) & 0xFF);
			break;

		case TIM2_RGB32:
			// 32bitカラ?`のとき
			r = pClut[n*4];
			g = pClut[n*4 + 1];
			b = pClut[n*4 + 2];
			a = pClut[n*4 + 3];
			pClut[n*4]     = (unsigned char)((newcolor)     & 0xFF);
			pClut[n*4 + 1] = (unsigned char)((newcolor>>8)  & 0xFF);
			pClut[n*4 + 2] = (unsigned char)((newcolor>>16) & 0xFF);
			pClut[n*4 + 3] = (unsigned char)((newcolor>>24) & 0xFF);
			break;

		default:
			// 音屎なピクセルフォ?`マットの??栽
			r = 0;
			g = 0;
			b = 0;
			a = 0;
			break;
	}
	return((a<<24) | (g<<16) | (b<<8) | r);
}


// テクセル(弼秤?鵑箸蕨泙蕕覆?)デ?`タを誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// mipmap:  MIPMAPテクスチャレベル
// x:       テクセルデ?`タを誼るテクセルX恙??
// y:       テクセルデ?`タを誼るテクセルY恙??
// ??り??
//          弼秤??(4bitまたは8bitのインデクス桑催、または16bit,24bit,32bitのダイレクトカラ?`)
unsigned int Tim2GetTexel(TIM2_PICTUREHEADER *ph, int mipmap, int x, int y)
{
	unsigned char *pImage;
	int t;
	int w, h;

	pImage = (unsigned char *)Tim2GetImage(ph, mipmap);
	if(pImage==NULL) {
		// 峺協レベルのテクスチャデ?`タがなかった??栽
		return(0);
	}
	Tim2GetMipMapPictureSize(ph, mipmap, &w, &h);
	if(x>w || y>h) {
		// テクセル恙?砲?音屎なとき
		return(0);
	}

	t = y*w + x;
	switch(ph->ImageType) {
		case TIM2_RGB16:
			return((pImage[t*2 + 1]<<8) | pImage[t*2]);

		case TIM2_RGB24:
			return((pImage[t*3 + 2]<<16) | (pImage[t*3 + 1]<<8) | pImage[t*3]);

		case TIM2_RGB32:
			return((pImage[t*4 + 3]<<24) | (pImage[t*4 + 2]<<16) | (pImage[t*4 + 1]<<8) | pImage[t*4]);

		case TIM2_IDTEX4:
			if(x & 1) {
				return(pImage[t/2]>>4);
			} else {
				return(pImage[t/2] & 0x0F);
			}
		case TIM2_IDTEX8:
			return(pImage[t]);
	}

	// 音屎なピクセルフォ?`マットだった??栽
	return(0);
}



// テクセル(弼秤?鵑箸蕨泙蕕覆?)デ?`タを?O協する
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// mipmap:  MIPMAPテクスチャレベル
// x:       テクセルデ?`タを誼るテクセルX恙??
// y:       テクセルデ?`タを誼るテクセルY恙??
// ??り??
//          弼秤??(4bitまたは8bitのインデクス桑催、または16bit,24bit,32bitのダイレクトカラ?`)
unsigned int Tim2SetTexel(TIM2_PICTUREHEADER *ph, int mipmap, int x, int y, unsigned int newtexel)
{
	unsigned char *pImage;
	int t;
	int w, h;
	unsigned int oldtexel;

	pImage = (unsigned char *)Tim2GetImage(ph, mipmap);
	if(pImage==NULL) {
		// 峺協レベルのテクスチャデ?`タがなかった??栽
		return(0);
	}
	Tim2GetMipMapPictureSize(ph, mipmap, &w, &h);
	if(x>w || y>h) {
		// テクセル恙?砲?音屎なとき
		return(0);
	}

	t = y*w + x;
	switch(ph->ImageType) {
		case TIM2_RGB16:
			oldtexel = (pImage[t*2 + 1]<<8) | pImage[t*2];
			pImage[t*2]     = (unsigned char)((newtexel)    & 0xFF);
			pImage[t*2 + 1] = (unsigned char)((newtexel>>8) & 0xFF);
			return(oldtexel);

		case TIM2_RGB24:
			oldtexel = (pImage[t*3 + 2]<<16) | (pImage[t*3 + 1]<<8) | pImage[t*3];
			pImage[t*3]     = (unsigned char)((newtexel)     & 0xFF);
			pImage[t*3 + 1] = (unsigned char)((newtexel>>8)  & 0xFF);
			pImage[t*3 + 2] = (unsigned char)((newtexel>>16) & 0xFF);
			return(oldtexel);

		case TIM2_RGB32:
			oldtexel = (pImage[t*4 + 3]<<24) | (pImage[t*4 + 2]<<16) | (pImage[t*4 + 1]<<8) | pImage[t*4];
			pImage[t*4]     = (unsigned char)((newtexel)     & 0xFF);
			pImage[t*4 + 1] = (unsigned char)((newtexel>>8)  & 0xFF);
			pImage[t*4 + 2] = (unsigned char)((newtexel>>16) & 0xFF);
			pImage[t*4 + 3] = (unsigned char)((newtexel>>24) & 0xFF);
			return(oldtexel);

		case TIM2_IDTEX4:
			if(x & 1) {
				oldtexel = pImage[t/2]>>4;
				pImage[t/2] = (unsigned char)((newtexel<<4) | oldtexel);
			} else {
				oldtexel = pImage[t/2] & 0x0F;
				pImage[t/2] = (unsigned char)((oldtexel<<4) | newtexel);
			}
			return(oldtexel);
		case TIM2_IDTEX8:
			oldtexel = pImage[t];
			pImage[t] = (unsigned char)newtexel;
			return(oldtexel);
	}

	// 音屎なピクセルフォ?`マットだった??栽
	return(0);
}


// テクスチャカラ?`を誼る
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// mipmap:  MIPMAPテクスチャレベル
// clut:    インデクスカラ?`の???Qに喘いるCLUTセット桑催
// x:       テクセルデ?`タを誼るテクセルX恙??
// y:       テクセルデ?`タを誼るテクセルY恙??
// ??り??
//          RGBA32フォ?`マットで弼を卦す
//          clutの峺協にエラ?`があるとき、0x00000000(?\)を卦す
//          x,yの峺協にエラ?`があったときの?嘛?は隠?^しない
unsigned int Tim2GetTextureColor(TIM2_PICTUREHEADER *ph, int mipmap, int clut, int x, int y)
{
	unsigned int t;
	if(Tim2GetImage(ph, mipmap)==NULL) {
		// 峺協レベルのテクスチャデ?`タがなかった??栽
		return(0);
	}
	t = Tim2GetTexel(ph, mipmap, (x>>mipmap), (y>>mipmap));
	switch(ph->ImageType) {
		case TIM2_RGB16:
			{
				unsigned char r, g, b, a;
				r = (unsigned char)((t<<3) & 0xF8);
				g = (unsigned char)((t>>2) & 0xF8);
				b = (unsigned char)((t>>7) & 0xF8);
				a = (unsigned char)((t>>8) & 0x80);
				return((a<<24) | (g<<16) | (b<<8) | r);
			}

		case TIM2_RGB24:
			return((0x80<<24) | (t & 0x00FFFFFF));

		case TIM2_RGB32:
			return(t);

		case TIM2_IDTEX4:
		case TIM2_IDTEX8:
			return(Tim2GetClutColor(ph, clut, t));
	}
	return(0);
}






// これ參週の?v方は、PS2のee-gccでのみ聞喘できる?v方
#ifdef R5900

// TIM2ピクチャデ?`タをGSロ?`カルメモリに?iみ?zむ
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// tbp:     ??僕枠テクスチャバッファのペ?`ジベ?`スアドレス(-1のときヘッダ坪の?､鯤荒?)
// cbp:     ??僕枠CLUTバッファのペ?`ジベ?`スアドレス(-1のときヘッダ坪の?､鯤荒?)
// ??り??
//          NULLのとき	エラ?`
//          NULLでないとき、肝のバッファアドレス
// 廣吭
//          CLUT鯉?{モ?`ドとしてCSM2が峺協されていた??栽も、??崙議にCSM1に???QしてGSに僕佚される
unsigned int Tim2LoadPicture(TIM2_PICTUREHEADER *ph, unsigned int tbp, unsigned int cbp)
{
	// CLUTデ?`タを??僕??僕
	tbp = Tim2LoadImage(ph, tbp);
	Tim2LoadClut(ph, cbp);
	return(tbp);
}


// TIM2ピクチャのイメ?`ジデ?`タ何をGSロ?`カルメモリに?iみ?zむ
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// tbp:     ??僕枠テクスチャバッファのペ?`ジベ?`スアドレス(-1のときヘッダ坪の?､鯤荒?)
// ??り??
//          NULLのとき	エラ?`
//          NULLでないとき、肝のテクスチャバッファアドレス
// 廣吭
//          CLUT鯉?{モ?`ドとしてCSM2が峺協されていた??栽も、??崙議にCSM1に???QしてGSに僕佚される
unsigned int Tim2LoadImage(TIM2_PICTUREHEADER *ph, unsigned int tbp)
{
	// イメ?`ジデ?`タを僕佚
	if(ph->MipMapTextures>0) {
		static const int psmtbl[] = {
			SCE_GS_PSMCT16,
			SCE_GS_PSMCT24,
			SCE_GS_PSMCT32,
			SCE_GS_PSMT4,
			SCE_GS_PSMT8
		};
		int i;
		int psm;
		u_long128 *pImage;
		int w, h;
		int tbw;

		psm = psmtbl[ph->ImageType - 1]; // ピクセルフォ?`マットを誼る
		((sceGsTex0 *)&ph->GsTex0)->PSM  = psm;

		w = ph->ImageWidth;  // イメ?`ジXサイズ
		h = ph->ImageHeight; // イメ?`ジYサイズ

		// イメ?`ジデ?`タの枠?^アドレスを??麻
		pImage = (u_long128 *)((char *)ph + ph->HeaderSize);
		if(tbp==-1) {
			// tbpの峺協が-1のとき、ピクチャヘッダに峺協されたGsTex0からtbp,tbwを誼る
			tbp = ((sceGsTex0 *)&ph->GsTex0)->TBP0;
			tbw = ((sceGsTex0 *)&ph->GsTex0)->TBW;

			Tim2LoadTexture(psm, tbp, tbw, w, h, pImage); // テクスチャデ?`タをGSに??僕
		} else {
			// tbpの峺協が峺協されたとき、ピクチャヘッダのGsTex0メンバのtbp,tbwをオ?`バ?`ライド
			tbw = Tim2CalcBufWidth(psm, w);
			// GSのTEX0レジスタに?O協する?､鮓?仟
			((sceGsTex0 *)&ph->GsTex0)->TBP0 = tbp;
			((sceGsTex0 *)&ph->GsTex0)->TBW  = tbw;
			Tim2LoadTexture(psm, tbp, tbw, w, h, pImage); // テクスチャデ?`タをGSに??僕
			tbp += Tim2CalcBufSize(psm, w, h);            // tbpの?､鮓?仟
		}

		if(ph->MipMapTextures>1) {
			// MIPMAPテクスチャがある??栽
			TIM2_MIPMAPHEADER *pm;

			pm = (TIM2_MIPMAPHEADER *)(ph + 1); // ピクチャヘッダの岷瘁にMIPMAPヘッダ

			// LV0のテクスチャサイズを誼る
			// tbpを哈方で峺協されたとき、ピクチャヘッダにあるmiptbpを?o??して徭?嗷?麻
			if(tbp!=-1) {
				pm->GsMiptbp1 = 0;
				pm->GsMiptbp2 = 0;
			}

			pImage = (u_long128 *)((char *)ph + ph->HeaderSize);
			// 光MIPMAPレベルのイメ?`ジを??僕
			for(i=1; i<ph->MipMapTextures; i++) {
				// MIPMAPレベルがあがると、テクスチャサイズは1/2になる
				w = w / 2;
				h = h / 2;

				pImage = (u_long128 *)((char *)pImage + pm->MMImageSize[i - 1]);
				if(tbp==-1) {
					// テクスチャペ?`ジの峺協が-1のとき、MIPMAPヘッダに峺協されたtbp,tbwを聞喘する
					int miptbp;
					if(i<4) {
						// MIPMAPレベル1,2,3のとき
						miptbp = (pm->GsMiptbp1>>((i-1)*20)) & 0x3FFF;
						tbw    = (pm->GsMiptbp1>>((i-1)*20 + 14)) & 0x3F;
					} else {
						// MIPMAPレベル4,5,6のとき
						miptbp = (pm->GsMiptbp2>>((i-4)*20)) & 0x3FFF;
						tbw    = (pm->GsMiptbp2>>((i-4)*20 + 14)) & 0x3F;
					}
					Tim2LoadTexture(psm, miptbp, tbw, w, h, pImage);
				} else {
					// テクスチャペ?`ジが峺協されているとき、MIPMAPヘッダを壅?O協
					tbw = Tim2CalcBufWidth(psm, w);    // テクスチャ嫌を??麻
					if(i<4) {
						// MIPMAPレベル1,2,3のとき
						pm->GsMiptbp1 |= ((u_long)tbp)<<((i-1)*20);
						pm->GsMiptbp1 |= ((u_long)tbw)<<((i-1)*20 + 14);
					} else {
						// MIPMAPレベル4,5,6のとき
						pm->GsMiptbp2 |= ((u_long)tbp)<<((i-4)*20);
						pm->GsMiptbp2 |= ((u_long)tbw)<<((i-4)*20 + 14);
					}
					Tim2LoadTexture(psm, tbp, tbw, w, h, pImage);
					tbp += Tim2CalcBufSize(psm, w, h); // tbpの?､鮓?仟
				}
			}
		}
	}
	return(tbp);
}



// TIM2ピクチャのCLUTデ?`タ何をGSロ?`カルメモリに??僕
// 哈方
// ph:      TIM2ピクチャヘッダの枠?^アドレス
// cbp:     ??僕枠CLUTバッファのペ?`ジベ?`スアドレス(-1のときヘッダ坪の?､鯤荒?)
// ??り??
//          0のときエラ?`
//          掲0のとき撹孔
// 廣吭
//          CLUT鯉?{モ?`ドとしてCSM2が峺協されていた??栽も、??崙議にCSM1に???QしてGSに僕佚される
unsigned int Tim2LoadClut(TIM2_PICTUREHEADER *ph, unsigned int cbp)
{
	int i;
	sceGsLoadImage li;
	u_long128 *pClut;
	int	cpsm;

	// CLUTピクセルフォ?`マットを誼る
	if(ph->ClutType==TIM2_NONE) {
		// CLUTデ?`タが贋壓しないとき
		return(1);
	} else if((ph->ClutType & 0x3F)==TIM2_RGB16) {
		cpsm = SCE_GS_PSMCT16;
	} else if((ph->ClutType & 0x3F)==TIM2_RGB24) {
		cpsm = SCE_GS_PSMCT24;
	} else {
		cpsm = SCE_GS_PSMCT32;
	}
	((sceGsTex0 *)&ph->GsTex0)->CPSM = cpsm; // CLUT何ピクセルフォ?`マット?O協
	((sceGsTex0 *)&ph->GsTex0)->CSM  = 0;    // CLUT鯉?{モ?`ド(械にCSM1)
	((sceGsTex0 *)&ph->GsTex0)->CSA  = 0;    // CLUTエントリオフセット(械に0)
	((sceGsTex0 *)&ph->GsTex0)->CLD  = 1;    // CLUTバッファのロ?`ド崙囮(械にロ?`ド)

	if(cbp==-1) {
		// cbpの峺協がないとき、ピクチャヘッダのGsTex0メンバから?､鯣ゝ?
		cbp = ((sceGsTex0 *)&ph->GsTex0)->CBP;
	} else {
		// cbpが峺協されたとき、ピクチャヘッダのGsTex0メンバの?､鬟??`バ?`ライド
		((sceGsTex0 *)&ph->GsTex0)->CBP = cbp;
	}

	// CLUTデ?`タの枠?^アドレスを??麻
	pClut = (u_long128 *)((char *)ph + ph->HeaderSize + ph->ImageSize);

	// CLUTデ?`タをGSロ?`カルメモリに僕佚
	// CLUT侘塀とテクスチャ侘塀によってCLUTデ?`タのフォ?`マットなどが?Qまる
	switch((ph->ClutType<<8) | ph->ImageType) {
		case (((TIM2_RGB16 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,16ビット,?Kび紋えずみ
		case (((TIM2_RGB24 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,24ビット,?Kび紋えずみ
		case (((TIM2_RGB32 | 0x40)<<8) | TIM2_IDTEX4): // 16弼,CSM1,32ビット,?Kび紋えずみ
		case (( TIM2_RGB16        <<8) | TIM2_IDTEX8): // 256弼,CSM1,16ビット,?Kび紋えずみ
		case (( TIM2_RGB24        <<8) | TIM2_IDTEX8): // 256弼,CSM1,24ビット,?Kび紋えずみ
		case (( TIM2_RGB32        <<8) | TIM2_IDTEX8): // 256弼,CSM1,32ビット,?Kび紋えずみ
			// 256弼CLUTかつ、CLUT鯉?{モ?`ドがCSM1のとき
			// 16弼CLUTかつ、CLUT鯉?{モ?`ドがCSM1で秘れ紋え?gみフラグがONのとき
			// すでにピクセルが秘れ紋えられて塘崔されているのでそのまま??僕辛嬬だ?`
			Tim2LoadTexture(cpsm, cbp, 1, 16, (ph->ClutColors / 16), pClut);
			break;

		case (( TIM2_RGB16        <<8) | TIM2_IDTEX4): // 16弼,CSM1,16ビット,リニア塘崔
		case (( TIM2_RGB24        <<8) | TIM2_IDTEX4): // 16弼,CSM1,24ビット,リニア塘崔
		case (( TIM2_RGB32        <<8) | TIM2_IDTEX4): // 16弼,CSM1,32ビット,リニア塘崔
		case (((TIM2_RGB16 | 0x80)<<8) | TIM2_IDTEX4): // 16弼,CSM2,16ビット,リニア塘崔
		case (((TIM2_RGB24 | 0x80)<<8) | TIM2_IDTEX4): // 16弼,CSM2,24ビット,リニア塘崔
		case (((TIM2_RGB32 | 0x80)<<8) | TIM2_IDTEX4): // 16弼,CSM2,32ビット,リニア塘崔
		case (((TIM2_RGB16 | 0x80)<<8) | TIM2_IDTEX8): // 256弼,CSM2,16ビット,リニア塘崔
		case (((TIM2_RGB24 | 0x80)<<8) | TIM2_IDTEX8): // 256弼,CSM2,24ビット,リニア塘崔
		case (((TIM2_RGB32 | 0x80)<<8) | TIM2_IDTEX8): // 256弼,CSM2,32ビット,リニア塘崔
			// 16弼CLUTかつ、CLUT鯉?{モ?`ドがCSM1で秘れ紋え?gみフラグがOFFのとき
			// 16弼CLUTかつ、CLUT鯉?{モ?`ドがCSM2のとき
			// 256弼CLUTかつ、CLUT鯉?{モ?`ドがCSM2のとき

			// CSM2はパフォ?`マンスが??いので、CSM1として秘れ紋えながら??僕
			for(i=0; i<(ph->ClutColors/16); i++) {
				sceGsSetDefLoadImage(&li, cbp, 1, cpsm, (i & 1)*8, (i>>1)*2, 8, 2);
				FlushCache(WRITEBACK_DCACHE);   // Dキャッシュの?澆?竃し
				sceGsExecLoadImage(&li, pClut); // CLUTデ?`タをGSに??僕
				sceGsSyncPath(0, 0);            // デ?`タ??僕?K阻まで棋?C

				// 肝の16弼へ、アドレス厚仟
				if((ph->ClutType & 0x3F)==TIM2_RGB16) {
					pClut = (u_long128 *)((char *)pClut + 2*16); // 16bit弼のとき
				} else if((ph->ClutType & 0x3F)==TIM2_RGB24) {
					pClut = (u_long128 *)((char *)pClut + 3*16); // 24bit弼のとき
				} else {
					pClut = (u_long128 *)((char *)pClut + 4*16); // 32bit弼のとき
				}
			}
			break;

		default:
			printf("Illegal clut and texture combination. ($%02X,$%02X)\n", ph->ClutType, ph->ImageType);
			return(0);
	}
	return(1);
}


// TIM2ファイルでスナップショットを??き竃す
// 哈方
// d0:      甜方ラスタのフレ?`ムバッファ
// d1:      謎方ラスタのフレ?`ムバッファ(NULLのときノンインタレ?`ス)
// pszFname:TIM2ファイル兆
// 卦り??
//          0のときエラ?`
//          掲0のとき撹孔
int Tim2TakeSnapshot(sceGsDispEnv *d0, sceGsDispEnv *d1, char *pszFname)
{
	int i;

	int h;               // ファイルハンドル
	int nWidth, nHeight; // イメ?`ジの雁隈
	int nImageType;      // イメ?`ジ何?N?e
	int psm;             // ピクセルフォ?`マット
	int nBytes;          // 1ラスタを??撹するバイト方

	// 鮫?颯汽ぅ?,ピクセルフォ?`マットを誼る
	nWidth  = d0->display.DW / (d0->display.MAGH + 1);
	nHeight = d0->display.DH + 1;
	psm     = d0->dispfb.PSM;

	// ピクセルフォ?`マットから、1ラスタあたりのバイト方,TIM2ピクセル?N?eを箔める
	switch(psm) {
		case SCE_GS_PSMCT32 :	nBytes = nWidth*4;	nImageType = TIM2_RGB32;	break;
		case SCE_GS_PSMCT24 :	nBytes = nWidth*3;	nImageType = TIM2_RGB24;	break;
		case SCE_GS_PSMCT16 :	nBytes = nWidth*2;	nImageType = TIM2_RGB16;	break;
		case SCE_GS_PSMCT16S:	nBytes = nWidth*2;	nImageType = TIM2_RGB16;	break;
		default:
			// 音苧なピクセルフォ?`マットのとき、エラ?`?K阻
			// GS_PSGPU24フォ?`マットは掲サポ?`ト
			printf("Illegal pixel format.\n");
			return(0);
	}


	// TIM2ファイルをオ?`プン
	h = sceOpen(pszFname, SCE_WRONLY | SCE_CREAT);
	if(h==-1) {
		// ファイルオ?`プン払??
		printf("file create failure.\n");
		return(0);
	}

	// ファイルヘッダを??き竃し
	{
		TIM2_FILEHEADER fhdr;

		fhdr.FileId[0] = 'T';   // ファイルIDを?O協
		fhdr.FileId[1] = 'I';
		fhdr.FileId[2] = 'M';
		fhdr.FileId[3] = '2';
		fhdr.FormatVersion = 3; // フォ?`マットバ?`ジョン4
		fhdr.FormatId  = 0;     // 16バイトアラインメントモ?`ド
		fhdr.Pictures  = 1;     // ピクチャ旦方は1旦
		for(i=0; i<8; i++) {
			fhdr.pad[i] = 0x00; // パディングメンバを0x00でクリア
		}

		sceWrite(h, &fhdr, sizeof(TIM2_FILEHEADER)); // ファイルヘッダを??き竃し
	}

	// ピクチャヘッダを??き竃し
	{
		TIM2_PICTUREHEADER phdr;
		int nImageSize;

		nImageSize = nBytes * nHeight;
		phdr.TotalSize   = sizeof(TIM2_PICTUREHEADER) + nImageSize; // ト?`タルサイズ
		phdr.ClutSize    = 0;                           // CLUT何なし
		phdr.ImageSize   = nImageSize;                  // イメ?`ジ何サイズ
		phdr.HeaderSize  = sizeof(TIM2_PICTUREHEADER);  // ヘッダ何サイズ
		phdr.ClutColors  = 0;                           // CLUT弼方
		phdr.PictFormat  = 0;                           // ピクチャ侘塀
		phdr.MipMapTextures = 1;                        // MIPMAPテクスチャ旦方
		phdr.ClutType    = TIM2_NONE;                   // CLUT何なし
		phdr.ImageType   = nImageType;                  // イメ?`ジ?N?e
		phdr.ImageWidth  = nWidth;                      // イメ?`ジ罪嫌
		phdr.ImageHeight = nHeight;                     // イメ?`ジ互さ

		// GSレジスタの?O協は畠何0にしておく
		phdr.GsTex0        = 0;
		((sceGsTex0 *)&phdr.GsTex0)->TBW = Tim2CalcBufWidth(psm, nWidth);
		((sceGsTex0 *)&phdr.GsTex0)->PSM = psm;
		((sceGsTex0 *)&phdr.GsTex0)->TW  = Tim2GetLog2(nWidth);
		((sceGsTex0 *)&phdr.GsTex0)->TH  = Tim2GetLog2(nHeight);
		phdr.GsTex1        = 0;
		phdr.GsTexaFbaPabe = 0;
		phdr.GsTexClut     = 0;

		sceWrite(h, &phdr, sizeof(TIM2_PICTUREHEADER)); // ピクチャヘッダを??き竃し
	}


	// イメ?`ジデ?`タの??き竃し
	for(i=0; i<nHeight; i++) {
		u_char buf[4096];   // ラスタバッファを4KB?_隠
		sceGsStoreImage si;

		if(d1) {
			// インタレ?`スのとき
			if(!(i & 1)) {
				// インタレ?`ス燕幣の甜方ラスタのとき
				sceGsSetDefStoreImage(&si, d0->dispfb.FBP*32, d0->dispfb.FBW, psm,
				                      d0->dispfb.DBX, (d0->dispfb.DBY + i/2),
				                      nWidth, 1);
			} else {
				// インタレ?`ス燕幣の謎方ラスタのとき
				sceGsSetDefStoreImage(&si, d1->dispfb.FBP*32, d1->dispfb.FBW, psm,
				                      d1->dispfb.DBX, (d1->dispfb.DBY + i/2),
				                      nWidth, 1);
			}
		} else {
			// ノンインタレ?`スのとき
			sceGsSetDefStoreImage(&si, d0->dispfb.FBP*32, d0->dispfb.FBW, psm,
			                      d0->dispfb.DBX, (d0->dispfb.DBY + i),
			                      nWidth, 1);
		}
		FlushCache(WRITEBACK_DCACHE); // Dキャッシュの?澆?竃し

		sceGsExecStoreImage(&si, (u_long128 *)buf); // VRAMへの??僕を軟??
		sceGsSyncPath(0, 0);                        // デ?`タ??僕?K阻まで棋?C

		sceWrite(h, buf, nBytes);  // 1ラスタ蛍のデ?`タを??き竃し
	}

	// ファイルの??き竃し頼阻
	sceClose(h);  // ファイルをクロ?`ズ
	return(1);
}






// テクスチャデ?`タを??僕
// 哈方
// psm:     テクスチャピクセルフォ?`マット
// tbp:     テクスチャバッファのベ?`スポイント
// tbw:     テクスチャバッファの嫌
// w:       ??僕?I囃の罪嫌
// h:       ??僕?I囃のライン方
// pImage:  テクスチャイメ?`ジが鯉?{されているアドレス
static void Tim2LoadTexture(int psm, u_int tbp, int tbw, int w, int h, u_long128 *pImage)
{
	sceGsLoadImage li;
	int i, l, n;
	u_long128 *p;

	switch(psm) {
		case SCE_GS_PSMZ32:
		case SCE_GS_PSMCT32:
			n = w*4;
			break;

		case SCE_GS_PSMZ24:
		case SCE_GS_PSMCT24:
			n = w*3;
			break;

		case SCE_GS_PSMZ16:
		case SCE_GS_PSMZ16S:
		case SCE_GS_PSMCT16:
		case SCE_GS_PSMCT16S:
			n = w*2;
			break;

		case SCE_GS_PSMT8H:
		case SCE_GS_PSMT8:
			n = w;
			break;

		case SCE_GS_PSMT4HL:
		case SCE_GS_PSMT4HH:
		case SCE_GS_PSMT4:
			n = w/2;
			break;

		default:
			return;
	}

	// DMAの恷寄??僕楚の512KBを階えないように蛍護しながら僕佚
	l = 32764 * 16 / n;
	for(i=0; i<h; i+=l) {
		p = (u_long128 *)((char *)pImage + n*i);
		if((i+l)>h) {
			l = h - i;
		}

		sceGsSetDefLoadImage(&li, tbp, tbw, psm, 0, i, w, l);
		FlushCache(WRITEBACK_DCACHE); // Dキャッシュの?澆?竃し
		sceGsExecLoadImage(&li, p);   // GSロ?`カルメモリへの??僕を軟??
		sceGsSyncPath(0, 0);          // デ?`タ??僕?K阻まで棋?C
	}
	return;
}



// 峺協されたピクセルフォ?`マットとテクスチャサイズから、バッファサイズを??麻
// 哈方
// psm:     テクスチャピクセルフォ?`マット
// w:       罪嫌
// 卦り??
//          1ラインで???Mするバッファサイズ
//          ?g了は256バイト(64word)
static int Tim2CalcBufWidth(int psm, int w)
{
//	return(w / 64);

	switch(psm) {
		case SCE_GS_PSMT8H:
		case SCE_GS_PSMT4HL:
		case SCE_GS_PSMT4HH:
		case SCE_GS_PSMCT32:
		case SCE_GS_PSMCT24:
		case SCE_GS_PSMZ32:
		case SCE_GS_PSMZ24:
		case SCE_GS_PSMCT16:
		case SCE_GS_PSMCT16S:
		case SCE_GS_PSMZ16:
		case SCE_GS_PSMZ16S:
			return((w+63) / 64);

		case SCE_GS_PSMT8:
		case SCE_GS_PSMT4:
			w = (w+63) / 64;
			if(w & 1) {
				w++;
			}
			return(w);
	}
	return(0);
}



// 峺協されたピクセルフォ?`マットとテクスチャサイズから、バッファサイズを??麻
// 哈方
// psm:     テクスチャピクセルフォ?`マット
// w:       罪嫌
// h:       ライン方
// 卦り??
//          1ラインで???Mするバッファサイズ
//          ?g了は256バイト(64word)
// 廣吭
//          ?Aくテクスチャが??なるピクセルフォ?`マットを鯉?{する??栽は、
//          2KBペ?`ジバウンダリまでtbpをアラインメントを?{屁する駅勣がある。
static int Tim2CalcBufSize(int psm, int w, int h)
{
	return(w * h / 64);
/*
	switch(psm) {
		case SCE_GS_PSMT8H:
		case SCE_GS_PSMT4HL:
		case SCE_GS_PSMT4HH:
		case SCE_GS_PSMCT32:
		case SCE_GS_PSMCT24:
		case SCE_GS_PSMZ32:
		case SCE_GS_PSMZ24:
			// 1ピクセルあたり、1ワ?`ド???M
			return(((w+63)/64) * ((h+31)/32));

		case SCE_GS_PSMCT16:
		case SCE_GS_PSMCT16S:
		case SCE_GS_PSMZ16:
		case SCE_GS_PSMZ16S:
			// 1ピクセルあたり、1/2ワ?`ド???M
			return(((w+63)/64) * ((h+63)/64));

		case SCE_GS_PSMT8:
			// 1ピクセルあたり、1/4ワ?`ド???M
			return(((w+127)/128) * ((h+63)/64));

		case SCE_GS_PSMT4:
			// 1ピクセルあたり、1/8ワ?`ド???M
			return(((w+127)/128) * ((h+127)/128));
	}
	// エラ?`?
	return(0);
*/
}



// ビット嫌を誼る
static int Tim2GetLog2(int n)
{
	int i;
	for(i=31; i>0; i--) {
		if(n & (1<<i)) {
			break;
		}
	}
	if(n>(1<<i)) {
		i++;
	}
	return(i);
}


#endif	// R5900
