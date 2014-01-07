#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <opencv/cv.h> 
#include <opencv/cxcore.h> 
#include <opencv/highgui.h> 

#include "alcon2009.h"

void DrawNextContour(IplImage *img, CvSeq *Contour, int Level);
void DrawChildContour(IplImage *img, CvSeq *Contour, int Level);
void cv_Labeling(IplImage *src_img, IplImage *dst_img);

//from myAlgo.c
int  assign_id(unsigned char *img, int width, int height, int **obj_id);

int main(int argc,char **argv){

	char *img;
	int i;
	int n; //物体数
	img = argv[1];
	IplImage *src_img = cvLoadImage(img,0);
	IplImage *dst_img = cvCreateImage(cvGetSize(src_img),IPL_DEPTH_8U,3);

	object *obj;//結果を格納する領域
	int **obj_id = (int **)malloc(dst_img->height*sizeof(int*));
	for(i = 0;i<dst_img->height;i++){
		obj_id[i] = (int*)malloc(dst_img->width*sizeof(int));	
	}


	/* オープニング */
	cvDilate(src_img,src_img,NULL,7);	

	cv_Labeling(src_img,dst_img);

	//n = assign_id(argv[1],dst_img->width,dst_img->height,obj_id);
	//printf("number of assign_id is %d",n);

	cvSaveImage(argv[2],dst_img,0);

	cvReleaseImage(&src_img);
	cvReleaseImage(&dst_img);
	

	return 0;
}


void DrawNextContour(IplImage *img, CvSeq *Contour, int Level){
	
	//輪郭を描画する色の設定
	CvScalar ContoursColor;

	if((Level % 2) == 1){
		
		//白の輪郭の場合、赤で輪郭を描画
		ContoursColor = CV_RGB(255,255,255);
	}else{
		ContoursColor = CV_RGB(0,0,255);	
	}

	//輪郭の描画
	cvDrawContours(img, Contour, ContoursColor,ContoursColor,0,CV_FILLED,8,cvPoint(0,0));

	if(Contour->h_next != NULL){
		//次の輪郭がある場合は次の輪郭を描画
		DrawNextContour(img,Contour->h_next,Level);
	}

	if(Contour->v_next != NULL){
		//子の輪郭がある場合は子の輪郭を描画
		DrawChildContour(img,Contour->v_next,Level+1);
	}

}

//子の輪郭を描画
void DrawChildContour(IplImage *img, CvSeq *Contour, int Level){

	//輪郭を描画する色の設定
	CvScalar ContoursColor;

	if((Level % 2) == 1){
	//白の輪郭の場合、赤で輪郭を描画
	ContoursColor = CV_RGB(255,0,0);
	}
	else{
	//黒の輪郭の場合、赤で輪郭を描画
	ContoursColor = CV_RGB(0,0,255);
	}

	//輪郭の描画
	cvDrawContours(img,Contour,ContoursColor,ContoursColor,0,CV_FILLED,8,cvPoint(0,0));

	if(Contour->h_next != NULL)
		//次の輪郭がある場合は次の輪郭を描画
		DrawNextContour(img,Contour->h_next,Level);
	if(Contour->v_next != NULL)
		//子の輪郭がある場合は子の輪郭を描画
		DrawChildContour(img,Contour->v_next,Level+1);

}

void cv_Labeling(IplImage *src_img, IplImage *dst_img){
	
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = NULL;

	if(src_img == NULL)
		return;

	//画像のニ値化
	cvThreshold(src_img,src_img,0,255,CV_THRESH_BINARY | CV_THRESH_OTSU);
	//輪郭の検出
	int find_contour_num = cvFindContours(src_img,storage,&contours,sizeof(CvContour),CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0));

	printf("find_countour_num is %d\n",find_contour_num);
	printf("cvSeq is %lu\n",sizeof(contours));


	if(contours != NULL){
		//処理後画像を0で初期化
		cvSet(dst_img,CV_RGB(0,0,0),0);
		//輪郭の描画
		DrawNextContour(dst_img,contours,1);
	}


	//メモリストレージの解放
	cvReleaseMemStorage(&storage);
	
}

//from myalgo.c
void replace_obj_id(int **obj_id, int n, int t, int width, int height)
{
	int x, y;
	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			if (obj_id[y][x] == t) {
				obj_id[y][x] =  n;
			}
		}
	}
}

int assign_id(unsigned char *img, int width, int height, int **obj_id)
{


  int x, y, s;
  int count, val = 0;

  for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {	
	  obj_id[y][x] = 0;
	}
  }

  for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	  /* 物体領域では画素値は0 */
	  if (img[x+y*width] == 0 && obj_id[y][x] == 0) {
#if 0 /* 2009/5/8 修正 */
		/* 物体領域である場合，左,上を探索 */
		int u = obj_id[y-1][x  ];	/* 注目画素の上 */
		int l = obj_id[y  ][x-1];	/* 注目画素の左 */
#endif
		int u = 0, l = 0;
		if (y > 0) u = obj_id[y-1][x  ];	/* 注目画素の上 */
		if (x > 0) l = obj_id[y  ][x-1];	/* 注目画素の左 */

		if (u == 0 && l == 0)        { /* 新しい物体 */
		  obj_id[y][x] = ++val;
		} else if (u == l && u != 0) { /* 上，左ともに同一物体 */
		  obj_id[y][x] = u;
		} else if (u != 0 && l == 0) { /* 上と同一物体 */
		  obj_id[y][x] = u;
		} else if (u == 0 && l != 0) { /* 左と同一物体 */
		  obj_id[y][x] = l;
		} else if (u != 0 && l != 0) { /* 上と左が連結 */
		  if (u < l) { /* 小さい物体IDにあわせる */
			obj_id[y][x] = u;
			replace_obj_id(obj_id, u, l, width, height);
		  } else {
			obj_id[y][x] = l;
			replace_obj_id(obj_id, l, u, width, height);
		  }
		}
	  }
	}
  }

  /* 物体IDを振りなおす */
  count = 0;
  for (s = 1; s <= val; s++) {
	int n = 0;
	for (y = 0; y < height; y++){
	  for (x = 0; x < width; x++){
		if (obj_id[y][x] == s) n += 1;
	  }
	}

	if (n > 0) {
	  count += 1;
	  replace_obj_id(obj_id, count, s, width, height);
	}
  }

  return count;

}

