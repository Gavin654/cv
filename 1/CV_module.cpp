#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define width_in  720   //width of image
#define height_in 480  //height of image

int main(int argc, char** argv)
{
	CvCapture *capture;

	IplImage* frame_in; //聲明IplImage指針
	//開新視窗
    cvNamedWindow( "ori", 1 );
	
	//////////資料輸入
	capture = cvCaptureFromCAM(0);
	//capture = cvCaptureFromAVI("D://Output.avi");
	
    frame_in = cvQueryFrame(capture);
	///////找尋影像的大小
	int height     = frame_in->height;
	int width      = frame_in->width;
	int step  =	frame_in->widthStep/sizeof(uchar);//step       = frame_in->widthStep;
	printf("h = %d w = %d s = %d\n",height,width,step);
	/////宣告暫存器
	int i,j,k,l,frame_counter = 0;
	int total_reg01 = 0,total_reg02 = 0,total_reg03 = 0,total_reg04 = 0;
	int a=1,sub01 = 0,sub02 = 0,sub03 = 0,sub04 = 0;
	/////宣告影像空間CV
	IplImage* frame_A = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_B = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_C = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_D = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
    
	while(true)	{	
		if(cvGrabFrame(capture)){
			frame_in = cvRetrieveFrame(capture);
			k=0;//縮小四分之一大小
			for( i=0 ; i < height ; i++ ){
				l=0;//縮小四分之一大小
				for( j=0 ; j < width ; j++ ){	
					if ((i%2)==1 &&(j%2)==1){
					
						if ((frame_counter%4) == 0){
							frame_A->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//+frame_in->imageData[(i-1)*step+(j)*3+0])/2;//+frame_in->imageData[(i-1)*step+j*3+0]+frame_in->imageData[i*step+(j-1)*3+0])/4;//B
							frame_A->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//+frame_in->imageData[(i-1)*step+(j)*3+1])/2;//+frame_in->imageData[(i-1)*step+j*3+1]+frame_in->imageData[i*step+(j-1)*3+1])/4;//G
							frame_A->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//+frame_in->imageData[(i-1)*step+(j)*3+2])/2;//+frame_in->imageData[(i-1)*step+j*3+2]+frame_in->imageData[i*step+(j-1)*3+2])/4;//R
							sub01 = abs(frame_A->imageData[k*step+l*3+0] - frame_B->imageData[i*step+j*3+0]);
							sub02 = abs(frame_A->imageData[k*step+l*3+0] - frame_C->imageData[i*step+j*3+0]);
							sub03 = abs(frame_A->imageData[k*step+l*3+0] - frame_D->imageData[i*step+j*3+0]);
							
						}else if ((frame_counter%4) == 1){
							frame_B->imageData[i*step+j*3+0]=frame_in->imageData[i*step+j*3+0];//B
							frame_B->imageData[i*step+j*3+1]=frame_in->imageData[i*step+j*3+1];//G
							frame_B->imageData[i*step+j*3+2]=frame_in->imageData[i*step+j*3+2];//R
						}else if ((frame_counter%4) == 2){
							frame_C->imageData[i*step+j*3+0]=frame_in->imageData[i*step+j*3+0];//B
							frame_C->imageData[i*step+j*3+1]=frame_in->imageData[i*step+j*3+1];//G
							frame_C->imageData[i*step+j*3+2]=frame_in->imageData[i*step+j*3+2];//R
						}else if ((frame_counter%4) == 3){
							frame_D->imageData[i*step+j*3+0]=frame_in->imageData[i*step+j*3+0];//B
							frame_D->imageData[i*step+j*3+1]=frame_in->imageData[i*step+j*3+1];//G
							frame_D->imageData[i*step+j*3+2]=frame_in->imageData[i*step+j*3+2];//R
						}
						if (sub01 > 5)
							total_reg01++;
						if (sub02 > 5)
							total_reg02++;
						if (sub03 > 5)
							total_reg03++;
						l++;
					}//end l
					
				}//end j
				if ((i%2)==1)
				k++;
			}//end i
				
			cvShowImage( "ori", frame_A );
			frame_counter++;
		}//end if(cvGrabFrame(capture))
		
		//等待ESC按鍵按下則結束
		if(cvWaitKey(10) == 27)
			break;
	} // end of while 
	   

	cvWaitKey(0);
        
	cvDestroyWindow( "ori" );//銷毀視窗
        
	cvReleaseCapture( &capture );  
    
	return 0;
}

