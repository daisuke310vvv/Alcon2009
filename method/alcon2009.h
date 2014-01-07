#ifndef __ALCON2009_H__
#define __ALCON2009_H__

/* 矩形領域を囲む関数で引く線の太さ */
#define LINE_WIDTH 4


typedef struct object_t {
	int label;	/* ラベル */
	int rep;	/* 代表例か(1:代表例、0:非代表例) */
	int x1, y1; /* 左上座標   */
	int x2, y2; /* 右下座標   */
} object;

unsigned char *load_ppm(const char *filename, int *width, int *height);
unsigned char *load_pgm(const char *filename, int *width, int *height);
int save_ppm(const char *filename, int width, int height, unsigned char *data);
int save_pgm(const char *filename, int width, int height, unsigned char *data);


object *my_alg_level1(unsigned char *image, unsigned char *mask, int width, int height, int *n_object);
object *my_alg_level2(unsigned char *image, int width, int height, int *n_object);
object *my_alg_level3(unsigned char *image, int width, int height, int *n_object);

void evaluate(object *c, int n_c, const char *filename);

#endif

