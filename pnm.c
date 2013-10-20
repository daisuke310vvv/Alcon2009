#include "pnm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* �ե������ɤ߹��ߤ����Ѥ���Хåե��Υ����� */
#define BUF_SIZE 256


/*------------------------------------------------------------
 PNM�ե�����Υإå��ɤ߹���
 -------------------------------------------------------------*/
static int read_pnm_header(FILE *f, int *w, int *h, int *max)
{
	char buf[BUF_SIZE];
	
	int ret;

	fgets(buf, BUF_SIZE, f);
	if (strcmp(buf, "P6\n") == 0) {
		ret = 6;
	} else if (strcmp(buf, "P5\n") == 0) {
		ret = 5;
	} else {
		return 0;
	}

	do {
		fgets(buf, BUF_SIZE, f);
	} while (buf[0] == '#');

	sscanf(buf, "%d %d", w, h);
	if (*w < 0 || *h < 0) return 0;

	fgets(buf, BUF_SIZE, f);
	sscanf(buf, "%d", max);
	if (*max > 255) return 0;
	
	return ret;
}


/*************************************************************
 * �Х��ʥ�PPM�ե�������ɤ߹��ߡ����ͥݥ��󥿤β�����ɬ�ס� *
 *************************************************************/
unsigned char *load_ppm(const char *filename, int *width, int *height)
{
	unsigned char *data = NULL;
	int max, w, h;
	size_t c;
	
	FILE *f = fopen(filename, "rb");
	if (!f) return NULL;

	if (read_pnm_header(f, &w, &h, &max) != 6) {
		fclose(f);
		return NULL;
	}

	data = (unsigned char *)malloc(3*w*h);
	c = fread(data, 1, 3*w*h, f);

	if (c != w*h*3) {
		free(data);
		data = NULL;
	}

	*width  = w;
	*height = h;
	
	return data;
}


/*************************************************************
 * �Х��ʥ�PGM�ե�������ɤ߹��ߡ����ͥݥ��󥿤β�����ɬ�ס� *
 *************************************************************/
unsigned char *load_pgm(const char *filename, int *width, int *height)
{
	unsigned char *data = NULL;
	int max, w, h;
	size_t c;
	
	FILE *f = fopen(filename, "rb");
	if (!f) return NULL;

	if (read_pnm_header(f, &w, &h, &max) != 5) {
		fclose(f);
		return NULL;
	}

	data = (unsigned char *)malloc(w*h);
	c = fread(data, 1, w*h, f);

	if (c != w*h) {
		free(data);
		data = NULL;
	}

	*width  = w;
	*height = h;
	
	return data;
}


/*************************************************************
 * �Х��ʥ�PPM�ե�����ν񤭹���                             *
 *************************************************************/
int save_ppm(const char *filename, int width, int height, unsigned char *data)
{
	FILE *f = fopen(filename, "wb");
	if (!f) return 0;

	fprintf(f, "P6\n%d %d\n255\n", width, height);
	if (fwrite(data, 1, 3*width*height, f) != 3*width*height)
		return 0;

	return 1;
}


/*************************************************************
 * �Х��ʥ�PGM�ե�����ν񤭹���                             *
 *************************************************************/
int save_pgm(const char *filename, int width, int height, unsigned char *data)
{
	FILE *f = fopen(filename, "wb");
	if (!f) return 0;

	fprintf(f, "P5\n%d %d\n255\n", width, height);
	if (fwrite(data, 1, width*height, f) != width*height)
		return 0;

	return 1;
}
