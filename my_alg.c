#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <float.h>




/*OpenCV*/
#include <opencv/cv.h> 
#include <opencv/highgui.h>

/*#include "cvblob.h"*/

#include "alcon2009.h"


#define PI  3.1415926535897932384626433832795
#define RT2 1.4142135623730950488016887242097 /* = sqrt(2.0) */


/* クラスタ数 */
#define NUM_CLASS 2


/* 関数のプロトタイプ宣言 */
int  assign_id(unsigned char *img, int width, int height, int **obj_id);
void find_rect(int **obj_id, int width, int height, int n_object, object *obj);
void calculate_area(int **obj_id, int width, int height, int n_object, int *area);
void calculate_length(int **obj_id, int width, int height, int n_object, double *len, unsigned char *image);
void k_means(double *x, object *obj, int N);

void sum_RGB(unsigned char *image, int n_object, object *obj,float *sumOfRGB);

void sum_Hist(unsigned char *image,int n_object,object *obj,int *sumOfHist);

void DrawNextContour(IplImage *img, CvSeq *Contour, int Level);
void DrawChildContour(IplImage *img, CvSeq *Contour, int Level);
void cv_Labeling(IplImage *src_img, IplImage *dst_img);

/* レベル2，レベル3は未実装 */
object *my_alg_level2(unsigned char *image, int width, int height, int *n_object) { 

	int i,j,n;

	IplImage *src_img,*src2_img;;
	IplImage *canny_img;
	IplImage *dst_img;

	char hoge[] = "level2-1.ppm";

	/* グレイ画像に変換 */
	src2_img = cvLoadImage(hoge,CV_LOAD_IMAGE_COLOR);
	src_img = cvLoadImage(hoge,CV_LOAD_IMAGE_GRAYSCALE);
	if(src_img == 0) printf("error");
	else cvSaveImage("hoge.jpg",src2_img,0);

	/* キャニーフィルタ */
	canny_img = cvCreateImage(cvGetSize(src_img),IPL_DEPTH_8U,1);	
	cvCanny(src_img,canny_img,20,200,3);
	cvSaveImage("canny_img.jpg",canny_img,0);

	/* 膨張 */
	cvDilate(canny_img,canny_img,NULL,7);
	/* ラベリング */
	dst_img = cvCreateImage(cvGetSize(canny_img),IPL_DEPTH_8U,1);
	cv_Labeling(canny_img,dst_img);
	
	//２値画像反転
	int count = 0;
	unsigned char *img = (int *)malloc(dst_img->height*dst_img->width*sizeof(int));
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){

			if(dst_img->imageData[j+i*width] == 0){
				dst_img->imageData[j+i*width] = 255;	
				img[j+i*width] = 255;
				count++;
			}else{
				dst_img->imageData[j+i*width] = 0;
				img[j+i*width] = 0;
			}

		}
	}
	cvSaveImage("LabelledImage.jpg",dst_img,0);

	/* assign_id */
	object *obj;	/* 結果を格納する領域 */
	int **obj_id = (int **)malloc(height*sizeof(int *));
	for (i = 0; i < dst_img->height; i++) obj_id[i] = (int *)malloc(dst_img->width*sizeof(int));

	/* 各物体領域にIDを割り当てて物体数を数える */
	n = 0;
	n = assign_id(img, dst_img->width, dst_img->height, obj_id);

	printf("assign_id %d\n",n);
	/* find_rect */
	obj  = (object *)malloc(n * sizeof(object));
	find_rect(obj_id,dst_img->width,dst_img->height,n,obj);
	for(i = 0;i<n;i++){
		
	printf("%d:(%d,%d),(%d,%d)\n",i,obj[i].x1,obj[i].y1,obj[i].x2,obj[i].y2);
		
		}

	int w = obj[0].x2 - obj[0].x1;
	int h = obj[0].y2 - obj[0].y1;
	int x=0,y=0;
//	int val1,val2,val3;

	FILE *fp = fopen("tempImage2-1.ppm","w");
	unsigned char tempImg[w*h*3];
	int o = 0;
	fprintf(fp,"P3\n%d %d\n255\n",w,h);
	printf("w = %d,h = %d\n",w,h);

	IplImage* temp_img = cvCreateImage(cvSize(w,h),src2_img->depth,src2_img->nChannels);
	printf("%d, %d\n",temp_img->widthStep,src2_img->widthStep);

	for(i=obj[0].y1;i<obj[0].y2;i++){
		for(j=obj[0].x1;j<obj[0].x2;j++){

			//val1 = src2_img->imageData[(src2_img->widthStep*i+j*3) + 0];
			unsigned char val1 = src2_img->imageData[(src2_img->widthStep*i+j*3) + 0];
			val1 = (0 > val1)? 0 : val1;	
			val1 = (val1 < 255)? val1:255;

			temp_img->imageData[(temp_img->widthStep*y+x*temp_img->nChannels)+0] = src2_img->imageData[(src2_img->widthStep*i+j*src2_img->nChannels) + 0];

			//val2 = src2_img->imageData[(src2_img->widthStep*i+j*3) + 1];
			unsigned char val2 = src2_img->imageData[(src2_img->widthStep*i+j*3) + 1];
			val2 = (0 > val2)? 0 : val2;	
			val2 = (val2 < 255)? val2:255;

			temp_img->imageData[(temp_img->widthStep*y+x*temp_img->nChannels)+1] = src2_img->imageData[(src2_img->widthStep*i+j*src2_img->nChannels) + 1];

			//val3 = src2_img->imageData[(src2_img->widthStep*i+j*3) + 2];
			unsigned char val3 = src2_img->imageData[(src2_img->widthStep*i+j*3) + 2];
			val3 = (0 > val3)? 0 : val3;	
			val3 = (val3 < 255)? val3:255;

			temp_img->imageData[(temp_img->widthStep*y+x*temp_img->nChannels)+2] = src2_img->imageData[(src2_img->widthStep*i+j*src2_img->nChannels) + 2];

			//printf(" %d ",temp_img->imageData[(temp_img->widthStep*0+x*temp_img->nChannels)+1]);
			fprintf(fp,"%5d%5d%5d",val3,val2,val1);
			x++;	
		}
		fprintf(fp,"\n");
		y++;
	}


	if(temp_img != NULL){
		cvSaveImage("tImg.jpg",temp_img,0);
	}else printf("error\n");


	fclose(fp);

	/* 全探索 */
	/*
	IplImage *pyr_img1 = cvCreateImage(cvSize(src2_img->width/2,src2_img->height/2),src2_img->depth,src2_img->nChannels);	
	cvPyrDown (src2_img,pyr_img1, CV_GAUSSIAN_5x5);
	cvSaveImage("pyr_img1.jpg",pyr_img1,0);
	cvReleaseImage(&pyr_img1);
*/	
	/*
	IplImage *pyr_img2 = cvCreateImage(cvSize(src2_img->width/2,src2_img->height/2),src2_img->depth,src2_img->nChannels);
	cvPyrDown (src2_img,pyr_img2, CV_GAUSSIAN_5x5);
	cvSaveImage("pyr_img2.jpg",pyr_img2,0);
	cvReleaseImage(&pyr_img2);
*/

	/* RGB to HSV */
	IplImage *hsv_img = cvCloneImage(src2_img);
	cvCvtColor(src2_img,hsv_img,CV_BGR2HSV);
//	IplImage *hsv_tImg = cvCloneImage(
	

	//free(obj_id);
	//free(obj);
	//
	
	cvRelease(&src2_img);
	cvRelease(&temp_img);

	
	return NULL;

}

object *my_alg_level3(unsigned char *image, int width, int height, int *n_object) { return NULL; }

object *my_alg_level1(unsigned char *image, unsigned char *mask, int width, int height, int *n_object)
{

 	int w,h;


	int i, j;
	int n;	/* 物体数 */

	int    *area;	/* 面積 */
	double *len;	/* 周囲長 */
	double *circ;	/* 円形度 */

	float *sumOfRGB; /*RGBの合計値*/
	
	/*ヒストグラム*/
	/*
	int hist_size = 256;
	int sch = 0;,ch_width = 260;
	float max_value[] = 0;
	float range_0[] = {0,256};
	float *ranges[] = {range_0};
	*/
	//IplImage *src_img = 0,*dst_img[4] = {0,0,0,0},*hist_img;
	CvHistogram *hist;
	int *sumOfHist;


	object *obj;	/* 結果を格納する領域 */

	int **obj_id = (int **)malloc(height*sizeof(int *));
	for (i = 0; i < height; i++) obj_id[i] = (int *)malloc(width*sizeof(int));

	/* 各物体領域にIDを割り当てて物体数を数える */
	n = assign_id(mask, width, height, obj_id);

	
	/* 必要な領域の確保 */
	area = (int *)malloc(n * sizeof(int));
	len  = (double *)malloc(n * sizeof(double));
	circ = (double *)malloc(n * sizeof(double));
	
	obj  = (object *)malloc(n * sizeof(object));

	sumOfRGB = (float *)malloc(n*sizeof(float));
	sumOfHist = (int*)malloc(n*sizeof(int));

	/*対象の物体領域のRGBの総計をだす*/
//	sum_RGB(obj_id,width,height,n,sumOfRGB);


	/* 面積を得る */
//	calculate_area(obj_id, width, height, n, area);

	/* 周囲長を得る */
//	calculate_length(obj_id, width, height, n, len, image);

	/* 円形度の算出 */
//	for (i = 0; i < n; i++) {
//		circ[i] = 4.0*PI * area[i] / (len[i]*len[i]);
//	}

	/* 最小矩形を見つける */
	find_rect(obj_id, width, height, n, obj);

	/*ヒストグラムの合計を求める*/
	sum_Hist(image,n,obj,sumOfHist);

	/*対象の物体領域のRGBの総計をだす*/
	sum_RGB(image,n,obj,sumOfRGB);

	/* k-means法によるクラスタリング */
	//k_means(sumOfRGB, obj, n);
	k_means(circ, obj, n);

	/* 代表例の決定(各クラスタで一番若い) */
	for (i = 0; i < n; i++) obj[i].rep = 0;
	for (j = 0; j < NUM_CLASS; j++) {
		for (i = 0; i < n; i++) {
			if (obj[i].label == j) {
				obj[i].rep = 1;
				break;
			}
		}
	}



	for (i = 0; i < height; i++) free(obj_id[i]);
	free(obj_id);

	free(area);
	free(len);
	free(circ);

	*n_object = n;

//	printf("width = %d height = %d",width,height);

	return obj;
}

/*************************************************************
 * My orignal Algorizm                                          *
 *************************************************************/
void sum_RGB(unsigned char *image, int n_object, object *obj, float *sumOfRGB){

	int w,h;		
	int i;
	int k,l;


	for(i = 0;i<n_object;i++){

		w = obj[i].x2 - obj[i].x1;
		h = obj[i].y2 - obj[i].y1;

//		printf("(%d %d) ,(%d %d) w = %d, h = %d\n",obj[i].x1,obj[i].y1,obj[i].x2,obj[i].y2,w,h);
//		printf("sumOfRGB[%d] = %f\n",i,sumOfRGB[i]);

		sumOfRGB[i] = 0;

		for(k = 0;k<h;k++){

	
			for(l = 0;l<w;l++){

				sumOfRGB[i] += (image[((obj[i].x1+k)+(obj[i].y1 + l)*w)*3 + 0] + image[((obj[i].x1+k)+(obj[i].y1 + l)*w)*3+1] + image[((obj[i].x1+k)+(obj[i].y1 + l)*w)*3 + 2]) ;


			}	

		}

			printf("sumOfRGB[%d] = %f\n",i,sumOfRGB[i]);

	}
	

	printf("\n");
	printf("/***********************************Sample circ*********************************/");
//	printf("/***********************************using sumOfRGB*********************************/");


}

void sum_Hist(unsigned char *image,int n_object,object *obj,int *sumOfHist){
	
  //RGB to Gray
  int w,h;

  int i;
  int k,l;

	//src_img生成	
	IplImage* src_img = 0;

  for(i = 0;i<1;i++){

	w = obj[i].x2 - obj[i].x1;
	h = obj[i].y2 - obj[i].y1;
	src_img = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);

	printf("%d %d\n",w,h);

	for(k = 0;k<h;k++){
	  for(l = 0;l<w;l++){

		src_img->imageData[(k*src_img->widthStep+l)*3 + 0] = image[((obj[i].x1+k)+(obj[i].y1 + l)*w)*3 + 0];

		src_img->imageData[(k*src_img->widthStep+l)*3 + 1] = image[((obj[i].x1+k)+(obj[i].y1 + l)*w)*3 + 1];

		src_img->imageData[(k*src_img->widthStep+l)*3 + 2] = image[((obj[i].x1+k)+(obj[i].y1 + l)*w)*3 + 2];

	  }

	}	

  }
	cvSaveImage("hogehoge.jpg",src_img,0);
	//ヒストグラム 

}

/*************************************************************
 * 物体IDの置き換え                                          *
 *************************************************************/
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


/*************************************************************
 * 各領域に物体IDを付与                                      *
 *************************************************************/
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




/*************************************************************
 * 各物体の最小矩形の算出                                    *
 *************************************************************/
void find_rect(int **obj_id, int width, int height, int n_object, object *obj)
{    
	int i, x, y;

	for (i = 0; i < n_object; i++) {
		obj[i].x1 = width;
		obj[i].y1 = height;
		obj[i].x2 = 0;
		obj[i].y2 = 0;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			int l = obj_id[y][x] - 1;
			if (l == -1) continue;

			if (obj[l].x1 > x) obj[l].x1 = x;
			if (obj[l].y1 > y) obj[l].y1 = y;
			if (obj[l].x2 < x) obj[l].x2 = x;
			if (obj[l].y2 < y) obj[l].y2 = y;
		}
	}
}

/*************************************************************
 * 各物体のピクセル数（面積）を数える                        *
 *************************************************************/
void calculate_area(int **obj_id, int width, int height, int n_object, int *area)
{	    
	int i, x, y;

	for (i = 0; i < n_object; i++) area[i] = 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			int l = obj_id[y][x];
			if (l == 0) continue;
			area[l-1] += 1;
		}
	}
}


/*************************************************************
 * 各物体の周囲長を算出                                      *
 *************************************************************/
void calculate_length(int **obj_id, int width, int height, int n_object, double *len, unsigned char *image)
{
	static const int    dx[8] = { -1,  -1,  -1,   0,   1,   1,   1,   0};
	static const int    dy[8] = { -1,   0,   1,   1,   1,   0,  -1,  -1};
	static const double dl[8] = {RT2, 1.0, RT2, 1.0, RT2, 1.0, RT2, 1.0};
	static const int  next[8] = {  6,   0,   0,   2,   2,   4,   4,   6};

    int i, j;
	int  x = 0,  y = 0;
	int px = 0, py = 0;

	for(i = 0; i < n_object; i++) {
		int l = i + 1; /* 対象物体のID */
		int found = 0;
		int dir;

		len[i] = 0;
		
		/* lの値を持つ連結領域を探索 */
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (obj_id[y][x] == l) {
					px = x;
					py = y;
					found = 1;
					break;
				}
			}
			if (found) break;
		}

		/* 8近傍を走査して輪郭をなぞる */
		dir = 2;
		do {
			for (j = 0; j < 8; j++) {
				int a = (dir + j) % 8;
				if (obj_id[py+dy[a]][px+dx[a]] == l) {
					len[i] += dl[a];
					px += dx[a];
					py += dy[a];
					dir = next[a];
					
					break;
				}
			}
		} while (px != x || py != y);
	}
}



/*************************************************************
 * k-means法によるクラスタリング                             *
 *************************************************************/
void k_means(double *x, object *obj, int n_object)
{
	static const double R = (double)NUM_CLASS / (RAND_MAX+1.0);

	int i, c;
	int    *num = (int    *)malloc(NUM_CLASS*sizeof(int));
	double *cen = (double *)malloc(NUM_CLASS*sizeof(double));

	srand((unsigned int)time(NULL));

	for (i = 0; i < n_object; i++) obj[i].label = (int)(rand()*R);

	while (1) {
		int count;

		for (c = 0; c < NUM_CLASS; c++) {
			num[c] =   0;
			cen[c] = 0.0;
		}

		/* クラスタ中心の算出 */
		for (i = 0; i < n_object; i++) {
			num[obj[i].label] +=    1;
			cen[obj[i].label] += x[i];
		}

		for (c = 0; c < NUM_CLASS; c++) cen[c] /= num[c];

		/* クラスタの再割り当て */
		count = 0;
		for (i = 0; i < n_object; i++) {
			double min = DBL_MAX;
			int min_class = 0;

			for (c = 0; c < NUM_CLASS; c++) {
				double distance = fabs(x[i] - cen[c]);
				if (distance < min) {
					min = distance;
					min_class = c;
				}
			}

			/* 終了判定のため */
			if (obj[i].label == min_class) count += 1;

			obj[i].label = min_class;
		}

		/* 終了判定 */
		if (count == n_object) break;
	}

	free(num);
	free(cen);
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
