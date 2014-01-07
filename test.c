#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "cvblob.h"

int main(void)
{
  IplImage *img = cvLoadImage("lebel1-1_mask.pgm", 1);

  cvThreshold(img, img, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

  IplImage *chB = cvCreateImage(cvGetSize(img), 8, 1);

  cvSplit(img, chB, NULL, NULL, NULL);

  IplImage *labelImg = cvCreateImage(cvGetSize(img), IPL_DEPTH_LABEL, 1);

  cvb::CvBlobs blobs;
  unsigned int result = cvLabel(chB, labelImg, blobs);

  cvRenderBlobs(labelImg, blobs, img, img);

  cvNamedWindow("test", 1);
  cvShowImage("test", img);
  cvWaitKey(0);
  cvDestroyWindow("test");

  cvSaveImage("blob.jpg", img);

  cvReleaseImage(&chB);
  cvReleaseImage(&labelImg);
  cvReleaseImage(&img);

  return 0;
}