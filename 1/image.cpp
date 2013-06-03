#include "stdafx.h"
#include "cv.h"
#include "highgui.h"

int main (int argc, char* argv[])
{
	IplImage *image = 0;
	image = cvLoadImage("D:\\cove.jpg");
	if (image)
	{
		cvNamedWindow("Input Image:",1);
		cvShowImage("Input Image", image);
		printf("press a key to exit\n");
		cvWaitKey(0);
		cvDestroyWindow("Input image");
	}
	else
		fprintf( stderr ,"Error reading image\n");
	return 0;
}