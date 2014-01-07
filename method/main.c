#include"bitmap.h"
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[])
{
	if(argc != 3){
		fprintf(stderr, "Usage: program <inputfile> <outputfile>\n");
		exit(1);
	}

	Image *colorimg;
	Image *hist;

	if((colorimg = Read_Bmp(argv[1])) == NULL){
		exit(1);
	}

	hist = Histogram(colorimg);

	if(Write_Bmp(argv[2], hist)){
		exit(1);
	}

	Free_Image(hist);

	Free_Image(colorimg);

	return 0;
}

