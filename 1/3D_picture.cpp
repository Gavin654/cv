#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "conio.h"

#define width_in  704//1600   //width of image
#define height_in 576//1200  //height of image

///宣告影像空間
	unsigned char line[height_in];//分割線
	unsigned char depth_floor[height_in][width_in];//深度圖
	unsigned char in[height_in][width_in];//深度圖

//宣告參數
int back_TH = 2000;


int main(int argc, char** argv)
{
	FILE *fp;

	IplImage* frame_in = 0; //聲明IplImage指針
	//開新視窗
    cvNamedWindow( "ori", 1 );
	cvNamedWindow( "DIBR", 1 );
	
	//////////資料輸入
	frame_in = cvLoadImage("D:\\3-2(d).bmp");///3-4水平線.jpg
	
   	///////找尋影像的大小
	int height     = frame_in->height;
	int width      = frame_in->width;
	int step  =	frame_in->widthStep/sizeof(uchar);//step       = frame_in->widthStep;
	printf("h = %d w = %d s = %d\n",height,width,step);
	/////宣告暫存器
	int i,j,k,l;
	double now_R[5],now_G[5],now_B[5];		//////切出分割線 背景
	double R_th,G_th,B_th; 
	int check_line_mid = 0,check_line_bom = 0;
	float linepoint_mid= 0,linepoint_bom= 0;
	float depth_color = 0;
	int depth_reg=0,motion;

	/////宣告影像空間CV
	IplImage* frame_DIBR = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
    IplImage* frame_tt = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	
	for( i=0 ; i < height ; i++ ){
		for( j=0 ; j < width ; j++ ){	
			if (i>(height/3) && i<(height/10*9)){
				if (j == 0 ){
					now_R[i%5] = 0;
					now_G[i%5] = 0;
					now_B[i%5] = 0;
				}
				now_B[i%5] += frame_in->imageData[i*step+j*3+0];
				now_G[i%5] += frame_in->imageData[i*step+j*3+1];
				now_R[i%5] += frame_in->imageData[i*step+j*3+2];
				if ((i%5) == 4 && j == width-1){
					R_th = ((now_R[0]/5 + now_R[1]/5 + now_R[2]/5 + now_R[3]/5 + now_R[4]/5) )*1.08;
					G_th = ((now_G[0]/5 + now_G[1]/5 + now_G[2]/5 + now_G[3]/5 + now_G[4]/5) )*1.08;
					B_th = ((now_B[0]/5 + now_B[1]/5 + now_B[2]/5 + now_B[3]/5 + now_B[4]/5) )*1.08;
							
					for (int m = 0 ; m < 5 ; m++ ){
						if ((R_th-now_R[m]) < 0 &&(G_th-now_G[m]) < 0 &&(B_th-now_B[m]) < 0){
							line[i-5+m] = 255;
							if (i>(height/3) && i<((height/3)*2)){								
								if (linepoint_mid < i){
									check_line_mid = 1;
									linepoint_mid = (float)(i-5+m);
								}
							}else if (i>((height/3)*2)){								
								if (i > linepoint_bom){
									check_line_bom = 1;
									linepoint_bom = (float)(i-5+m);
									//line[i-5+m] = 255;
								}
							}				
						}
					}//end for m
				}
			}//end if (i>(height/3)){
		}//end j		
	}//end i
	printf("linepoint_mid = %f, linepoint_bom = %f\n",linepoint_mid,linepoint_bom);
	printf("check_line_mid = %d, check_line_bom = %d\n",check_line_mid,check_line_bom);
	for( i=0 ; i < height ; i++ )
		for( j=0 ; j < width ; j++ ){	
			if (check_line_mid == 1 && check_line_bom == 0){
				if (i >= 0 && i < linepoint_mid){
					depth_color = (int)floor((double)((back_TH/linepoint_mid)*(i)));
					if (depth_color<=0)
						depth_color = 0;
					depth_floor[i][j] = back_TH - depth_color;
					motion = (int)((10*depth_color/(85+depth_color)));
					if ((j-motion)>0){
						frame_DIBR->imageData[i*step+(j-motion)*3+2] = frame_in->imageData[i*step+j*3+2];
					}
					if ((j+motion)<width){
						frame_DIBR->imageData[i*step+(j+motion)*3+0] = frame_in->imageData[i*step+j*3+0];
						frame_DIBR->imageData[i*step+(j+motion)*3+1] = frame_in->imageData[i*step+j*3+1];
					}
				}else if (i>=linepoint_mid && i<height){
					depth_color = back_TH - (int)floor((double)((back_TH/(height-linepoint_mid))*(i-linepoint_mid)));
					if (depth_color<=0)
						depth_color = 0;
					depth_floor[i][j] = back_TH - depth_color;
					motion = (int)((10*depth_color/(85+depth_color)));
					if ((j-motion)>0){
						frame_DIBR->imageData[i*step+(j-motion)*3+2] = frame_in->imageData[i*step+j*3+2];
						
					}
					if ((j+motion)<width){
						frame_DIBR->imageData[i*step+(j+motion)*3+0] = frame_in->imageData[i*step+j*3+0];
						frame_DIBR->imageData[i*step+(j+motion)*3+1] = frame_in->imageData[i*step+j*3+1];
					}
				}
			}else{
				//////////////0-255
				if (i >= 5 && i < (linepoint_bom-3))
				{
					depth_color = back_TH - (int)floor((double)((back_TH/(linepoint_bom-5))*(i)));
					depth_floor[i][j] = depth_color;
					motion = (int)((10*depth_color/(85+depth_color)));
					if ((j-motion)>0){
						frame_DIBR->imageData[i*step+(j-motion)*3+2] = frame_in->imageData[i*step+j*3+2];
					}
					if ((j+motion)<width){
						frame_DIBR->imageData[i*step+(j+motion)*3+0] = frame_in->imageData[i*step+j*3+0];
						frame_DIBR->imageData[i*step+(j+motion)*3+1] = frame_in->imageData[i*step+j*3+1];
					}
				}
				else if (i >= (linepoint_bom-3) && i<height)
				{
					depth_color = 0;
					depth_floor[i][j] = depth_color;
					motion = (int)((10*depth_color/(85+depth_color)));
					if ((j-motion)>0){
						frame_DIBR->imageData[i*step+(j-motion)*3+2] = frame_in->imageData[i*step+j*3+2];
						
					}
					if ((j+motion)<width){
						frame_DIBR->imageData[i*step+(j+motion)*3+0] = frame_in->imageData[i*step+j*3+0];
						frame_DIBR->imageData[i*step+(j+motion)*3+1] = frame_in->imageData[i*step+j*3+1];
					}
				}
			}
		}

		for( i=0 ; i < height ; i++ )
			for( j=0 ; j < width ; j++ ){	
				if (line[i] == 255){
				//if (linepoint_mid == i || linepoint_bom == i){
					frame_in->imageData[i*step+j*3+0] = 255;//B
					frame_in->imageData[i*step+j*3+1] = 255;//G
					frame_in->imageData[i*step+j*3+2] = 0;//R

					
				}
				in[i][j] = frame_in->imageData[i*step+j*3+1];//G
				//frame_tt->imageData[i*step+j*3+0] = binary_img[i][j];//B
				//frame_tt->imageData[i*step+j*3+1] = binary_img[i][j];//G
				//frame_tt->imageData[i*step+j*3+2] = tt[i][j];//R
			}
	fp=fopen("D:/tt.raw","wb"); //寫圖
	fwrite(in,height,width,fp);
	fclose(fp);

	   cvShowImage( "ori", frame_in );
	   cvShowImage( "DIBR", frame_DIBR );
	   //存影像
	   cvSaveImage("D:\\3-4.jpg",frame_DIBR);
	 
	cvWaitKey(0);
        
	cvDestroyWindow( "ori" );//銷毀視窗
	cvDestroyWindow( "DIBR" );//銷毀視窗
	
	return 0;
}

