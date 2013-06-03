#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define width  352   //width of image
#define height 576  //height of image

unsigned char in_img[height*2][width];

unsigned char R[height][width];
unsigned char G[height][width];
unsigned char B[height][width];

unsigned char Y[height][width];
unsigned char U[height][width];
unsigned char V[height][width];

int main(int argc, char** argv)
{
	FILE *fp;
	
    cvNamedWindow( "ori", 1 );
	
	int i,j,z=0,m,n,a;
	int mid_height = height/2;
	float u,v,y,r,g,b;
	
	////存avi影像
	CvVideoWriter *writer;
	int AviForamt = -1;
    int FPS = 25;
    CvSize AviSize = cvSize(width,height);
    int AviColor = 1;
    writer=cvCreateVideoWriter("Output.avi",AviForamt,FPS,AviSize,AviColor);

	/////宣告影像空間CV
	IplImage* frame_out = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);

	int step  =	frame_out->widthStep/sizeof(uchar);
    
	//////////資料輸入
	fp=fopen("D:/test4.yuv","rb");   //讀圖 左圖
	for(int frame=0 ; frame< 350 ; frame++)
	{   
		printf("frame = %d\n",frame);
		frame_out = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
		fread(in_img,height*2,width,fp);
		//===================================================  422-444 //定義YUV _F
		for(i=0 ; i<height ; i++)
		{
			m = 0;
			n = 0;
			a = 0;
			for(j=0 ; j<width ; j++)
			{
				Y[i][j] = in_img[i][j];

				if ((width/2-1) < j)
				{
					n = 1;
					a = width;
				}
				if (i<(height/2))
				{
					U[(i+i+n)][(j+m-a)] = in_img[i+height][j];
					U[(i+i+n)][(j+1+m-a)] = in_img[i+height][j];
					V[(i+i+n)][(j+m-a)] = in_img[i+height+mid_height][j];
					V[(i+i+n)][(j+1+m-a)] = in_img[i+height+mid_height][j];
				}
				m++;
			}
		}
		//===================================================  YUV轉成RGB
		for( i=0 ; i<height ; i++ )
		{
		    for( j=0 ; j<width ; j++ )
			{
			    r = (Y[i][j]+(V[i][j]-128)*(1.4075));
                g = (Y[i][j]-(U[i][j]-128)*(0.3455)-(V[i][j]-128)*(0.7169));
                b = (Y[i][j]+(U[i][j]-128)*(1.7790));

				if(r>255)
			        r=255;
			    else if(r<0)
                    r=0;
			    else
                    r=r;

				if(g>255)
			        g=255;
			    else if(g<0)
                    g=0;
			    else
                    g=g;

				if(b>255)
			        b=255;
			    else if(b<0)
                    b=0;
			    else
                    b=b;
				R[i][j] = r;
				G[i][j] = g;
				B[i][j] = b;
			}
		}
		for( i=0 ; i<height ; i++ )
		{
		    for( j=0 ; j<width ; j++ )
			{
				frame_out->imageData[i*step+j*3+0]=B[i][j];
				frame_out->imageData[i*step+j*3+1]=G[i][j];
				frame_out->imageData[i*step+j*3+2]=R[i][j];
			}
		}
		///存影像
			cvWriteFrame(writer,frame_out);
		cvShowImage( "ori", frame_out );
		cvReleaseImage( &frame_out );
	}
				
			
	
		
		
		//等待ESC按鍵按下則結束
	

	cvWaitKey(0);
        
	cvDestroyWindow( "ori" );//銷毀視窗
    cvReleaseVideoWriter(&writer);    
	
    
	return 0;
}

