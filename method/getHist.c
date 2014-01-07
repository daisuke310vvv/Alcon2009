#include <stdio.h>
#include <opencv/cv.h> 
#include <opencv/cxcore.h> 
#include <opencv/highgui.h> 
#pragma comment(lib,"cv.lib")
#pragma comment(lib,"cxcore.lib")
#pragma comment(lib,"highgui.lib")

#define KIDO_SIZE 256
#define GRAPH_HEIGHT 512

void histgram(unsigned int *out, unsigned char *in, int inHeight, int inWidth, int inChannel);

int main(int argc, char **argv){
	IplImage *in, *graph;
	unsigned int *outData;
	int i,j;
	double maxRate;

	char *img;
	img = argv[1];

	IplImage *outputImg;

	in = cvLoadImage(img, 0);
	graph = cvCreateImage(cvSize(KIDO_SIZE, GRAPH_HEIGHT), in->depth, in->nChannels);

	outputImg = cvCreateImage(cvGetSize(in),IPL_DEPTH_8U,1);

	outData = (unsigned int *)malloc(sizeof(unsigned int) * (KIDO_SIZE + 1));

//	0初期化
		memset(outData, 0, sizeof(unsigned int) * (KIDO_SIZE + 1));
	histgram(outData, (unsigned char*)in->imageData, in->height, in->width, in->nChannels);

	// 最大数の要素の値がグラフの最大値に来るように調整
	maxRate = (double)graph->height / (double)outData[KIDO_SIZE];

	int temp= 0;
	//最頻値
	/*
	for(i = 0;i<KIDO_SIZE;i++){
		if(outData[temp] < outData[i]){
			temp = i;
		}
	}
	*/
	
	//平均値
	int sum = 0;
	for(i = 0;i<KIDO_SIZE;i++){
		sum += outData[i];
	}
	temp = sum/2550;
	printf("sum = %d, temp = %d\n",sum,temp);

	//エッジ抽出
	cvCanny(in,outputImg,150,200,3);

	cvSaveImage(argv[2],outputImg,0);

	for(i = 0; i < KIDO_SIZE; ++i){
		cvLine(graph, cvPoint(i, graph->height - outData[i] * maxRate), cvPoint(i, graph->height), CV_RGB(255, 255, 255), 1,4,0);
	}
	

	//画像を出力する
	//cvSaveImage("hsv2-1_151.jpg",graph,0);


	cvReleaseImage( &in );
	//cvReleaseImage( &graph );
	cvReleaseImage( &outputImg );

	free(outData);


	return 0;
}


void histgram(unsigned int *out, unsigned char *in, int inHeight, int inWidth, int inChannel){
	int i, j;
	int index;

	for(i = 0; i < inHeight; ++i){
		for(j = 0; j < inWidth; ++j){
			// 入力画像の輝度値と等しい配列番号の要素をインクリメント
			index = in[i * inWidth + j];
			++out[ index ];

			// 最大値を格納する
			if(out[KIDO_SIZE] < out[ index ]){
				out[KIDO_SIZE] = out[ index ];
			}
		}
	}
}
