#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"

int _tmain(int argc, _TCHAR* argv[])
{
	
	IplImage* pImg; //聲明IplImage指針
	IplImage* pImgcanny;
	IplImage* pImggray;

        cvNamedWindow( "Camera", 1 );
		cvNamedWindow( "gray", 1 );
		cvNamedWindow( "canny", 1 );

        CvCapture *capture;
	int c; 
    
	capture = cvCaptureFromCAM(0);
	if(!capture)
		return -1;
	else
        printf("Camera is OK! \n");
	
	while(true){	
            
			if(cvGrabFrame(capture)){

				pImg = cvRetrieveFrame(capture);
				pImggray = cvCreateImage( cvGetSize(pImg),
					  IPL_DEPTH_8U,
					  1);
				
				pImgcanny = cvCreateImage( cvGetSize(pImg),
					  IPL_DEPTH_8U,
					  1);

				cvCvtColor (pImg , pImggray ,CV_BGR2GRAY);
				cvCanny(pImggray, pImgcanny, 50, 150, 3);
				
				cvShowImage( "Camera", pImg );
				cvShowImage( "gray", pImggray );
				cvShowImage( "canny", pImgcanny );
			}
		
		//等待ESC按鍵按下則結束
		if(cvWaitKey(10) == 27)
			break;
	} // end of while 
	   

	cvWaitKey(0);
        
	cvDestroyWindow( "Camera" );//銷毀視窗
        
	cvReleaseCapture( &capture );  
    
	return 0;
}

