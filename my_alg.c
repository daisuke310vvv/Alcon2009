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


/* ���饹���� */
#define NUM_CLASS 2


/* �ؿ��Υץ�ȥ�������� */
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

/* ��٥�2����٥�3��̤���� */
object *my_alg_level2(unsigned char *image, int width, int height, int *n_object) { 

	int i,j,n;

	IplImage *src_img,*src2_img;;
	IplImage *canny_img;
	IplImage *dst_img;

	char hoge[] = "level2-1.ppm";

	/* ���쥤�������Ѵ� */
	src2_img = cvLoadImage(hoge,CV_LOAD_IMAGE_COLOR);
	src_img = cvLoadImage(hoge,CV_LOAD_IMAGE_GRAYSCALE);
	if(src_img == 0) printf("error");
	else cvSaveImage("hoge.jpg",src2_img,0);

	/* ����ˡ��ե��륿 */
	canny_img = cvCreateImage(cvGetSize(src_img),IPL_DEPTH_8U,1);	
	cvCanny(src_img,canny_img,20,200,3);
	cvSaveImage("canny_img.jpg",canny_img,0);

	/* ��ĥ */
	cvDilate(canny_img,canny_img,NULL,7);
	/* ��٥�� */
	dst_img = cvCreateImage(cvGetSize(canny_img),IPL_DEPTH_8U,1);
	cv_Labeling(canny_img,dst_img);
	
	//���Ͳ���ȿž
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
	object *obj;	/* ��̤��Ǽ�����ΰ� */
	int **obj_id = (int **)malloc(height*sizeof(int *));
	for (i = 0; i < dst_img->height; i++) obj_id[i] = (int *)malloc(dst_img->width*sizeof(int));

	/* ��ʪ���ΰ��ID�������Ƥ�ʪ�ο�������� */
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

	/* ��õ�� */
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
	int n;	/* ʪ�ο� */

	int    *area;	/* ���� */
	double *len;	/* ����Ĺ */
	double *circ;	/* �߷��� */

	float *sumOfRGB; /*RGB�ι����*/
	
	/*�ҥ��ȥ����*/
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


	object *obj;	/* ��̤��Ǽ�����ΰ� */

	int **obj_id = (int **)malloc(height*sizeof(int *));
	for (i = 0; i < height; i++) obj_id[i] = (int *)malloc(width*sizeof(int));

	/* ��ʪ���ΰ��ID�������Ƥ�ʪ�ο�������� */
	n = assign_id(mask, width, height, obj_id);

	
	/* ɬ�פ��ΰ�γ��� */
	area = (int *)malloc(n * sizeof(int));
	len  = (double *)malloc(n * sizeof(double));
	circ = (double *)malloc(n * sizeof(double));
	
	obj  = (object *)malloc(n * sizeof(object));

	sumOfRGB = (float *)malloc(n*sizeof(float));
	sumOfHist = (int*)malloc(n*sizeof(int));

	/*�оݤ�ʪ���ΰ��RGB����פ����*/
//	sum_RGB(obj_id,width,height,n,sumOfRGB);


	/* ���Ѥ����� */
//	calculate_area(obj_id, width, height, n, area);

	/* ����Ĺ������ */
//	calculate_length(obj_id, width, height, n, len, image);

	/* �߷��٤λ��� */
//	for (i = 0; i < n; i++) {
//		circ[i] = 4.0*PI * area[i] / (len[i]*len[i]);
//	}

	/* �Ǿ�����򸫤Ĥ��� */
	find_rect(obj_id, width, height, n, obj);

	/*�ҥ��ȥ����ι�פ����*/
	sum_Hist(image,n,obj,sumOfHist);

	/*�оݤ�ʪ���ΰ��RGB����פ����*/
	sum_RGB(image,n,obj,sumOfRGB);

	/* k-meansˡ�ˤ�륯�饹����� */
	//k_means(sumOfRGB, obj, n);
	k_means(circ, obj, n);

	/* ��ɽ��η���(�ƥ��饹���ǰ��ּ㤤) */
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

	//src_img����	
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
	//�ҥ��ȥ���� 

}

/*************************************************************
 * ʪ��ID���֤�����                                          *
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
 * ���ΰ��ʪ��ID����Ϳ                                      *
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
			/* ʪ���ΰ�Ǥϲ����ͤ�0 */
			if (img[x+y*width] == 0 && obj_id[y][x] == 0) {
#if 0 /* 2009/5/8 ���� */
				/* ʪ���ΰ�Ǥ����硤��,���õ�� */
				int u = obj_id[y-1][x  ];	/* ���ܲ��Ǥξ� */
				int l = obj_id[y  ][x-1];	/* ���ܲ��Ǥκ� */
#endif
				int u = 0, l = 0;
				if (y > 0) u = obj_id[y-1][x  ];	/* ���ܲ��Ǥξ� */
				if (x > 0) l = obj_id[y  ][x-1];	/* ���ܲ��Ǥκ� */

				if (u == 0 && l == 0)        { /* ������ʪ�� */
					obj_id[y][x] = ++val;
				} else if (u == l && u != 0) { /* �塤���Ȥ��Ʊ��ʪ�� */
					obj_id[y][x] = u;
				} else if (u != 0 && l == 0) { /* ���Ʊ��ʪ�� */
					obj_id[y][x] = u;
				} else if (u == 0 && l != 0) { /* ����Ʊ��ʪ�� */
					obj_id[y][x] = l;
				} else if (u != 0 && l != 0) { /* ��Ⱥ���Ϣ�� */
					if (u < l) { /* ������ʪ��ID�ˤ��碌�� */
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

	/* ʪ��ID�򿶤�ʤ��� */
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
 * ��ʪ�ΤκǾ�����λ���                                    *
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
 * ��ʪ�ΤΥԥ�����������ѡˤ������                        *
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
 * ��ʪ�Τμ���Ĺ�򻻽�                                      *
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
		int l = i + 1; /* �о�ʪ�Τ�ID */
		int found = 0;
		int dir;

		len[i] = 0;
		
		/* l���ͤ����Ϣ���ΰ��õ�� */
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

		/* 8��˵�����������سԤ�ʤ��� */
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
 * k-meansˡ�ˤ�륯�饹�����                             *
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

		/* ���饹���濴�λ��� */
		for (i = 0; i < n_object; i++) {
			num[obj[i].label] +=    1;
			cen[obj[i].label] += x[i];
		}

		for (c = 0; c < NUM_CLASS; c++) cen[c] /= num[c];

		/* ���饹���κƳ������ */
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

			/* ��λȽ��Τ��� */
			if (obj[i].label == min_class) count += 1;

			obj[i].label = min_class;
		}

		/* ��λȽ�� */
		if (count == n_object) break;
	}

	free(num);
	free(cen);
}


void DrawNextContour(IplImage *img, CvSeq *Contour, int Level){
	
	//�سԤ����褹�뿧������
	CvScalar ContoursColor;

	if((Level % 2) == 1){
		
		//����سԤξ�硢�֤��سԤ�����
		ContoursColor = CV_RGB(255,255,255);
	}else{
		ContoursColor = CV_RGB(0,0,255);	
	}

	//�سԤ�����
	cvDrawContours(img, Contour, ContoursColor,ContoursColor,0,CV_FILLED,8,cvPoint(0,0));

	if(Contour->h_next != NULL){
		//�����سԤ�������ϼ����سԤ�����
		DrawNextContour(img,Contour->h_next,Level);
	}

	if(Contour->v_next != NULL){
		//�Ҥ��سԤ�������ϻҤ��سԤ�����
		DrawChildContour(img,Contour->v_next,Level+1);
	}

}

//�Ҥ��سԤ�����
void DrawChildContour(IplImage *img, CvSeq *Contour, int Level){

	//�سԤ����褹�뿧������
	CvScalar ContoursColor;

	if((Level % 2) == 1){
	//����سԤξ�硢�֤��سԤ�����
	ContoursColor = CV_RGB(255,0,0);
	}
	else{
	//�����سԤξ�硢�֤��سԤ�����
	ContoursColor = CV_RGB(0,0,255);
	}

	//�سԤ�����
	cvDrawContours(img,Contour,ContoursColor,ContoursColor,0,CV_FILLED,8,cvPoint(0,0));

	if(Contour->h_next != NULL)
		//�����سԤ�������ϼ����سԤ�����
		DrawNextContour(img,Contour->h_next,Level);
	if(Contour->v_next != NULL)
		//�Ҥ��سԤ�������ϻҤ��سԤ�����
		DrawChildContour(img,Contour->v_next,Level+1);

}

void cv_Labeling(IplImage *src_img, IplImage *dst_img){
	
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = NULL;

	if(src_img == NULL)
		return;

	//�����Υ��Ͳ�
	cvThreshold(src_img,src_img,0,255,CV_THRESH_BINARY | CV_THRESH_OTSU);
	//�سԤθ���
	int find_contour_num = cvFindContours(src_img,storage,&contours,sizeof(CvContour),CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0));

	printf("find_countour_num is %d\n",find_contour_num);
	printf("cvSeq is %lu\n",sizeof(contours));


	if(contours != NULL){
		//�����������0�ǽ����
		cvSet(dst_img,CV_RGB(0,0,0),0);
		//�سԤ�����
		DrawNextContour(dst_img,contours,1);
	}


	//���ꥹ�ȥ졼���β���
	cvReleaseMemStorage(&storage);
	
}
