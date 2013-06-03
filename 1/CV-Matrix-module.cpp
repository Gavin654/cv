#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "conio.h"

#define width_in  367   //width of image
#define height_in 380  //height of image

///宣告影像空間
	unsigned char line[height_in];//分割線
	unsigned char depth_floor[height_in][width_in];//深度圖
	unsigned char Y[height_in][width_in];//Y
	unsigned char R[height_in][width_in];//Y
	unsigned char G[height_in][width_in];//Y
	unsigned char B[height_in][width_in];//Y
	int avg[height_in][width_in];

//宣告參數
int back_TH = 255;


int main(int argc, char** argv)
{
	FILE *fp;

	IplImage* frame_in = 0; //聲明IplImage指針
	//開新視窗
    cvNamedWindow( "ori", 1 );
	cvNamedWindow( "DIBR", 1 );
	
	//////////資料輸入
	frame_in = cvLoadImage("D:\\cove.jpg");
	
   	///////找尋影像的大小
	int height     = frame_in->height;
	int width      = frame_in->width;
	int step  =	frame_in->widthStep/sizeof(uchar);//step       = frame_in->widthStep;
	printf("h = %d w = %d s = %d\n",height,width,step);
	/////宣告暫存器
	int i,j,k,l;
	
	/////宣告影像空間CV
	IplImage* frame_DIBR = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
    IplImage* frame_gray = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	IplImage* frame_avg = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	
	for( i=0 ; i < height ; i++ ){
		for( j=0 ; j < width ; j++ ){	
			//Y[i][j] = frame_gray->imageData[i*width+j];
			B[i][j] = frame_in->imageData[i*step+j*3+0];
			G[i][j] = frame_in->imageData[i*step+j*3+1];
			R[i][j] = frame_in->imageData[i*step+j*3+2];
			Y[i][j] = R[i][j]*(0.299) + G[i][j]*(0.587) + B[i][j]*(0.114);  

		}//end j		
	}//end i
	/////////////////演算法

	
	///////////////輸出
		for( i=0 ; i < height ; i++ )
			for( j=0 ; j < width ; j++ ){	
				//if (line[i] == 255){
								
				frame_avg->imageData[i*step+j*3+0] = Y[i][j];//B
				frame_avg->imageData[i*step+j*3+1] = Y[i][j];//G
				frame_avg->imageData[i*step+j*3+2] = Y[i][j];//R
			}
	fp=fopen("D:/tt.raw","wb"); //寫圖
	fwrite(avg,height,width,fp);
	fclose(fp);

	   cvShowImage( "ori", frame_in );
	   cvShowImage( "DIBR", frame_avg );
	   //存影像
	   cvSaveImage("D:\\3-4.jpg",frame_avg);
	 
	cvWaitKey(0);
        
	cvDestroyWindow( "ori" );//銷毀視窗
	cvDestroyWindow( "DIBR" );//銷毀視窗
	
	return 0;
}

