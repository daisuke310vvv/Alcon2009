#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <float.h>

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


/* レベル2，レベル3は未実装 */
object *my_alg_level2(unsigned char *image, int width, int height, int *n_object) { return NULL; }
object *my_alg_level3(unsigned char *image, int width, int height, int *n_object) { return NULL; }


object *my_alg_level1(unsigned char *image, unsigned char *mask, int width, int height, int *n_object)
{

	int i, j;
	int n;	/* 物体数 */

	int    *area;	/* 面積 */
	double *len;	/* 周囲長 */
	double *circ;	/* 円形度 */

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

	/* 面積を得る */
	calculate_area(obj_id, width, height, n, area);

	/* 周囲長を得る */
	calculate_length(obj_id, width, height, n, len, image);

	/* 円形度の算出 */
	for (i = 0; i < n; i++) {
		circ[i] = 4.0*PI * area[i] / (len[i]*len[i]);
	}

	/* k-means法によるクラスタリング */
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

	/* 最小矩形を見つける */
	find_rect(obj_id, width, height, n, obj);

	for (i = 0; i < height; i++) free(obj_id[i]);
	free(obj_id);

	free(area);
	free(len);
	free(circ);

	*n_object = n;
	return obj;
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


