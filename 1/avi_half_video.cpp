////本論文演算法
#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "conio.h"

#define width_in  720   //width of image
#define height_in 480  //height of image
//宣告參數


//宣告內部空間矩陣
	unsigned char R[height_in][width_in];
	unsigned char G[height_in][width_in];
	unsigned char B[height_in][width_in];
		
int main(int argc, char** argv)
{
	CvCapture *capture;

	IplImage* frame_in; //聲明IplImage指針
	//開新視窗
    cvNamedWindow( "ori", 1 );
	cvNamedWindow( "DIBR", 1 );
	
	//////////資料輸入
	//capture = cvCaptureFromCAM(0);
	capture = cvCaptureFromAVI("D://Homework//研究影片//Dance01.avi");
	
    frame_in = cvQueryFrame(capture);
	///////找尋影像的大小
	int height     = frame_in->height;
	int width      = frame_in->width;
	int step  =	frame_in->widthStep/sizeof(uchar);//step       = frame_in->widthStep;
	printf("h = %d w = %d s = %d\n",height,width,step);
	////存avi影像
	CvVideoWriter *writer;
	int AviForamt = -1;
    int FPS = 30;
    CvSize AviSize = cvSize(width,height);
    int AviColor = 1;
    writer=cvCreateVideoWriter("3ality Digital MaxQ.avi",AviForamt,FPS,AviSize,AviColor);

	/////宣告暫存器
	
	int i,j,k,l,frame_counter = 0,count;
		
	/////宣告影像空間CV
	//IplImage* frame_A = cvCreateImage(cvGetSize(frame_in),IPL_DEPTH_8U,3);
	IplImage* frame_A = cvCreateImage(cvSize(width/2,height),IPL_DEPTH_8U,3);
	
	int step_half = frame_A->widthStep/sizeof(uchar);//step       = frame_in->widthStep;
	while(true)	{	
   		if(cvGrabFrame(capture)){
			///計算時間
			frame_in = cvRetrieveFrame(capture);
		//	cvCopy(frame_in, frame_A);
			k = 0;
			for( i=0 ; i < height ; i++ ){			
				for( j=0 ; j < width/2 ; j++ ){	
					frame_A->imageData[i*step_half+(j)*3+0]=frame_in->imageData[i*step+j*3+0];
					frame_A->imageData[i*step_half+(j)*3+1]=frame_in->imageData[i*step+j*3+1];
					frame_A->imageData[i*step_half+(j)*3+2]=frame_in->imageData[i*step+j*3+2];
				}
				//k++;
			}//end i

			//for( i=0 ; i < height ; i++ ){			
			//	for( j=0 ; j < width/2 ; j++ ){	
			//		if (j<(width/2))
			//		frame_A->imageData[i*step/2+j*3+0]=B[i][j];
			//		frame_A->imageData[i*step/2+j*3+1]=G[i][j];
			//		frame_A->imageData[i*step/2+j*3+2]=R[i][j];
			//		
			//	}
			//}//end i
			
			///存影像
			cvWriteFrame(writer,frame_A);

			cvShowImage( "ori", frame_in );
			cvShowImage( "out", frame_A );
			frame_counter++;
		}//end if(cvGrabFrame(capture))
		
		//等待ESC按鍵按下則結束
		if(cvWaitKey(10) == 27)
			break;
		//釋放記憶體
		/*cvReleaseImage( &frame_DIBR );
		cvReleaseImage( &frame_tt );*/

	} // end of while 

	cvWaitKey(0);
        
	cvDestroyWindow( "ori" );//銷毀視窗
	cvDestroyWindow( "DIBR" );//銷毀視窗
    
	cvReleaseVideoWriter(&writer);

	cvReleaseCapture( &capture );  
    
	return 0;
}

