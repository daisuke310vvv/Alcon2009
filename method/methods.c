
//画像に対して様々な処理を行うモジュール
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>

/*OpenCV*/
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define PI  3.1415926535897932384626433832795
#define RT2 1.4142135623730950488016887242097 /* = sqrt(2.0) */


/*関数のプロトタイプ宣言*/
void getHist(IplImage *src_img, IplImage *hist_img);
void getHist_rgb(FILE *fp,unsigned char *hist_img);


int main(int argc,char* argv[1]){
	
	//画像ファイルポインタの宣言
	//IplImage* img;
	//HSV画像ファイルポインタの宣言
//	IplImage* hsv_img;

	//IplImage* hist_img;

	//出力用画像
	//IplImage* outputImg = 0;

	//読み込み画像ファイル
	//char imgFile[] = "../Alcon2009_Lv2/level2-2.ppm";
	FILE *fp;
	unsigned char *hist_img;
	fp = fopen("../Alcon2009_Lv2/level2-2.ppm","rb");
	if(fp == NULL){
		printf("cannot read the file");	
	}

	//グレースケールで読み込む
	img = cvLoadImage(imgFile,CV_LOAD_IMAGE_GRAYSCALE);
	//RGBで読み込む
//	img = cvLoadImage(imgFile,CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);

	//getHist(img,hist_img);

	//hsv_imgのデータ領域確保
//	hsv_img = cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,3);
	
	//画像ヘッダ及びそのデータ領域を確保
//	outputImg = cvCreateImage(cvGetSize(img),IPL_DEPTH_8U,1);

	//RGB to HSV
//	cvCvtColor(img,hsv_img,CV_RGB2HSV);
/*
	int i = 0;
	printf("sizeof hsv_img is %f",hsv_img->nsize);
	for(i = 0;i<sizeof(hsv_img);i++){
		printf("%f\n",hsv_img->imageData[3]);
	}
*/	
	//画像の読み込み
//	img = cvLoadImage(imgfile,CV_LOAD_IMAGE_ANYCOLOR | CV_LOAD_IMAGE_ANYDEPTH);

	
	//画像を出力する
//	cvSaveImage("hsv_img.jpg",hsv_img,0);
//

	fclose(fp);

	
	return 0;
}


#pragma mark メソッド集
void getHist(IplImage* src_img, IplImage* hist_img){

	int i,j,bin_w;
	int hist_size = 256;
	int sch = 0, ch_width = 260;
	float max_val = 0;
	float range_0[] = {0,256};
	float *ranges[] = {range_0};
	IplImage *planes[4] = {0,0,0,0};
	CvHistogram *hist[3];
	CvScalar color = cvScalarAll(100);

	sch = src_img->nChannels;
	for(i = 0;i<sch;i++){
		planes[i] = cvCreateImage(cvSize(src_img->width,src_img->height),src_img->depth,1);	
		hist[i] = cvCreateHist(1,&hist_size,CV_HIST_ARRAY,ranges,1);
	}

	hist_img = cvCreateImage(cvSize(ch_width*sch,200),8,3); 
	cvSet(hist_img,cvScalarAll(255),0);

	if(sch == 1){
		cvCopy(src_img,planes[0],NULL);	
		cvCalcHist(&planes[0],hist[0],0,NULL);
		cvGetMinMaxHistValue(hist[0],0,&max_val,0,0);
	}else{
		cvSplit(src_img,planes[0],planes[1],planes[2],planes[3]);	
		for(i = 0;i<sch;i++){
			float tmp_val;	
			cvCalcHist(&planes[i],hist[i],0,NULL);
			cvGetMinMaxHistValue(hist[i],0,&tmp_val,0,0);
			max_val = max_val < tmp_val? tmp_val : max_val;
		}
	}

	for(i =0;i<sch;i++){
		if(sch == 3){
			color = cvScalar((0xaa<<i*8)&0x0000ff,(0xaa<<i*8)&0x00ff00,(0xaa<<i*8)&0xff0000,0);
		}

		cvScale(hist[i]->bins,hist[i]->bins,((double)hist_img->height)/max_val,0);
		bin_w = cvRound((double)ch_width/hist_size);

		for(j = 0;j<hist_size;j++){

			cvRectangle(hist_img,
					cvPoint(j*bin_w+(i*ch_width),hist_img->height),
					cvPoint((j+1)*bin_w + (i*ch_width),
						hist_img->height-cvRound(cvGetReal1D(hist[i]->bins,j))),color,-1,8,0);

		}	
	}

	/*RGBそれぞれの値を足して255で割る*/

	cvSaveImage("hist_2-2.jpg",hist_img,0);
	/*
	cvReleaseImage(&src_img);
	cvReleaseImage(&hist_img);
	for(i =0;i<sch;i++){
		cvReleaseImage(&planes[i]);	
		cvReleaseImage(&hist[i]);	
	}
*/
}


void getHist_rgb(FILE *fp,unsigned char *hist_img){

	long hist_r[256];
	long hist_g[256];
	long hist_b[256];
	long width,height;
	long i;

	memset(hist_r,0,sizeof(hist_r));
	memset(hist_r,0,sizeof(hist_g));
	memset(hist_r,0,sizeof(hist_b));

	fseek(fp,0x12,SEEK_SET);
	fread(&width,sizeof(long),1,fp);
	fread(&height,sizeof(long),1,fp);
	fseek(fp,0x36,SEEK_SET);

	for(i = 0;i<width*height;i++){
		RGB rgb;	
		fread(&rgb,sizeof(rgb),1,fp);
		hist_r[rgb.r]++;
		hist_g[rgb.g]++;
		hist_b[rgb.b]++;
	}

	for(i = 0;i<256;i++){
		printf("%ld,%ld,%ld\n",hist_r[i],hist_g[i],hist_b[i]);	
	}



}
