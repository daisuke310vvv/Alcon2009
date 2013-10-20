#ifndef __ALCON2009_H__
#define __ALCON2009_H__

/* ����ΰ��Ϥ�ؿ��ǰ����������� */
#define LINE_WIDTH 4


typedef struct object_t {
	int label;	/* ��٥� */
	int rep;	/* ��ɽ�㤫(1:��ɽ�㡢0:����ɽ��) */
	int x1, y1; /* �����ɸ   */
	int x2, y2; /* ������ɸ   */
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

