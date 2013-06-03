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

///�ŧi�v���Ŷ�
	unsigned char line[height_in];//���νu
	unsigned char depth_floor[height_in][width_in];//�`�׹�
	unsigned char Y[height_in][width_in];//Y
	unsigned char R[height_in][width_in];//Y
	unsigned char G[height_in][width_in];//Y
	unsigned char B[height_in][width_in];//Y
	int avg[height_in][width_in];

//�ŧi�Ѽ�
int back_TH = 255;


int main(int argc, char** argv)
{
	FILE *fp;

	IplImage* frame_in = 0; //�n��IplImage���w
	//�}�s����
    cvNamedWindow( "ori", 1 );
	cvNamedWindow( "DIBR", 1 );
	
	//////////��ƿ�J
	frame_in = cvLoadImage("D:\\cove.jpg");
	
   	///////��M�v�����j�p
	int height     = frame_in->height;
	int width      = frame_in->width;
	int step  =	frame_in->widthStep/sizeof(uchar);//step       = frame_in->widthStep;
	printf("h = %d w = %d s = %d\n",height,width,step);
	/////�ŧi�Ȧs��
	int i,j,k,l;
	
	/////�ŧi�v���Ŷ�CV
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
	/////////////////�t��k

	
	///////////////��X
		for( i=0 ; i < height ; i++ )
			for( j=0 ; j < width ; j++ ){	
				//if (line[i] == 255){
								
				frame_avg->imageData[i*step+j*3+0] = Y[i][j];//B
				frame_avg->imageData[i*step+j*3+1] = Y[i][j];//G
				frame_avg->imageData[i*step+j*3+2] = Y[i][j];//R
			}
	fp=fopen("D:/tt.raw","wb"); //�g��
	fwrite(avg,height,width,fp);
	fclose(fp);

	   cvShowImage( "ori", frame_in );
	   cvShowImage( "DIBR", frame_avg );
	   //�s�v��
	   cvSaveImage("D:\\3-4.jpg",frame_avg);
	 
	cvWaitKey(0);
        
	cvDestroyWindow( "ori" );//�P������
	cvDestroyWindow( "DIBR" );//�P������
	
	return 0;
}

