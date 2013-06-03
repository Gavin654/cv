#include "stdafx.h"

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

int main(int argc, char* argv[])
{

        IplImage *image = 0;
		image = cvLoadImage("D:\\cove.jpg");
        if (image) {  
        cvNamedWindow("Image:", CV_WINDOW_AUTOSIZE);
        cvShowImage("Image:", image);
        cvWaitKey(0);
		cvDestroyWindow("String");}
         
		else 
        printf("Error: Couldn't open the image file.\n");
		
        return 0;
}
