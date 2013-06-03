////曾郭二值化
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
//宣告參數
int back_TH = 255;
int M_TH = 1000;
int af_motoin = 2;
int TH_min = 720*480*0.1;

//宣告內部空間矩陣
	unsigned char motion_diff0[height_in][width_in];
	unsigned char motion_diff1[height_in][width_in];
	unsigned char motion_diff2[height_in][width_in];
	unsigned char depth_floor[height_in][width_in];
	unsigned char line[height_in];//深度圖
	unsigned char binary_img[height_in][width_in];//量化二值影像 
	unsigned char Dilation_img[height_in][width_in];//膨脹  (慢
	
int main(int argc, char** argv)
{
	CvCapture *capture;

	IplImage* frame_in; //聲明IplImage指針
	//開新視窗
    cvNamedWindow( "ori", 1 );
	cvNamedWindow( "DIBR", 1 );
	
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
	clock_t start, end, diff_time; 
	int i,j,k,l,frame_counter = 0;
	int total_reg01 = 0,total_reg02 = 0,total_reg03 = 0,total_reg04 = 0;
	double now_R[5],now_G[5],now_B[5];		//////切出分割線 背景
	double R_th,G_th,B_th; 
	int check_line_mid = 0,check_line_bom = 0,ch_line_mid = 0,ch_line_bom = 0;
	float linepoint_mid= 0,linepoint_bom= 576,ch_linepoint_mid= 0,ch_linepoint_bom= 576;
	float depth_color = 0;
	int depth_reg=0,motion;
	int m1,m2,m3,count = 0,ch_count = 0;
	
	/////宣告影像空間CV
	IplImage* frame_A = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_B = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_C = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_D = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_DIBR = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_tt = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	

	while(true)	{	
   		if(cvGrabFrame(capture)){
			///計算時間
			
			end = clock();//結束時間 
			if (frame_counter>1)
			printf("frame_counter = %d time=%d ms \n",frame_counter,(end-start));
			start = clock();
			
			frame_in = cvRetrieveFrame(capture);
			check_line_mid = 0;check_line_bom = 0;linepoint_mid= 0;linepoint_bom= 576;
			k=0;//縮小四分之一大小
			for( i=0 ; i < height ; i++ ){
				line[i] = 0;
				l=0;//縮小四分之一大小
				for( j=0 ; j < width ; j++ ){	
					if ((i%2)==1 &&(j%2)==1){
					
						if ((frame_counter%4) == 0){
							frame_A->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//+frame_in->imageData[(i-1)*step+(j)*3+0])/2;//+frame_in->imageData[(i-1)*step+j*3+0]+frame_in->imageData[i*step+(j-1)*3+0])/4;//B
							frame_A->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//+frame_in->imageData[(i-1)*step+(j)*3+1])/2;//+frame_in->imageData[(i-1)*step+j*3+1]+frame_in->imageData[i*step+(j-1)*3+1])/4;//G
							frame_A->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//+frame_in->imageData[(i-1)*step+(j)*3+2])/2;//+frame_in->imageData[(i-1)*step+j*3+2]+frame_in->imageData[i*step+(j-1)*3+2])/4;//R
							motion_diff0[k][l] = abs(frame_A->imageData[k*step+l*3+1] - frame_B->imageData[k*step+l*3+1]);
							motion_diff1[k][l] = abs(frame_A->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1]);
							motion_diff2[k][l] = abs(frame_A->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1]);
							
						}else if ((frame_counter%4) == 1){
							frame_B->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//B
							frame_B->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//G
							frame_B->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//R
							motion_diff0[k][l] = abs(frame_B->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1]);
							motion_diff1[k][l] = abs(frame_B->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1]);
							motion_diff2[k][l] = abs(frame_B->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1]);

						}else if ((frame_counter%4) == 2){
							frame_C->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//B
							frame_C->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//G
							frame_C->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//R
							motion_diff0[k][l] = abs(frame_C->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1]);
							motion_diff1[k][l] = abs(frame_C->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1]);
							motion_diff2[k][l] = abs(frame_C->imageData[k*step+l*3+1] - frame_B->imageData[k*step+l*3+1]);

						}else if ((frame_counter%4) == 3){
							frame_D->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//B
							frame_D->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//G
							frame_D->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//R
							motion_diff0[k][l] = abs(frame_D->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1]);
							motion_diff1[k][l] = abs(frame_D->imageData[k*step+l*3+1] - frame_B->imageData[k*step+l*3+1]);
							motion_diff2[k][l] = abs(frame_D->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1]);

						}
						if (k > 3 && l > 5){
							m1 = motion_diff0[k-2][l-4]+motion_diff0[k-2][l-3]+motion_diff0[k-2][l-2]+motion_diff0[k-2][l-1]+motion_diff0[k-2][l]+
								motion_diff0[k-1][l-4]+motion_diff0[k-1][l-3]+motion_diff0[k-1][l-2]+motion_diff0[k-1][l-1]+motion_diff0[k-1][l]+
								motion_diff0[k][l-4]+motion_diff0[k][l-3]+motion_diff0[k][l-2]+motion_diff0[k][l-1]+motion_diff0[k][l];
							m2 = motion_diff1[k-2][l-4]+motion_diff1[k-2][l-3]+motion_diff1[k-2][l-2]+motion_diff1[k-2][l-1]+motion_diff1[k-2][l]+
								motion_diff1[k-1][l-4]+motion_diff1[k-1][l-3]+motion_diff1[k-1][l-2]+motion_diff1[k-1][l-1]+motion_diff1[k-1][l]+
								motion_diff1[k][l-4]+motion_diff1[k][l-3]+motion_diff1[k][l-2]+motion_diff1[k][l-1]+motion_diff1[k][l];
							m3 = motion_diff2[k-2][l-4]+motion_diff2[k-2][l-3]+motion_diff2[k-2][l-2]+motion_diff2[k-2][l-1]+motion_diff2[k-2][l]+
								motion_diff2[k-1][l-4]+motion_diff2[k-1][l-3]+motion_diff2[k-1][l-2]+motion_diff2[k-1][l-1]+motion_diff2[k-1][l]+
								motion_diff2[k][l-4]+motion_diff2[k][l-3]+motion_diff2[k][l-2]+motion_diff2[k][l-1]+motion_diff2[k][l];
							if (((m1+m2+m3) <= M_TH) ){
								binary_img[k][l] = 0;
								Dilation_img[i-1][j-1] = 0;Dilation_img[i-1][j] = 0;Dilation_img[i][j-1] = 0;Dilation_img[i][j] = 0;
							}else{
								binary_img[k][l] = 255;
								Dilation_img[i-1][j-1] = 255;Dilation_img[i-1][j] = 0;Dilation_img[i][j-1] = 0;Dilation_img[i][j] = 255;
								count++;
							}
						}

						/////////自動判斷背景深度
						if (i>(height/3) && i<(height/6*5)){
							if (l == 0 ){
								now_R[k%5] = 0;
								now_G[k%5] = 0;
								now_B[k%5] = 0;
							}
							now_B[k%5] += frame_in->imageData[i*step+j*3+0];
							now_G[k%5] += frame_in->imageData[i*step+j*3+1];
							now_R[k%5] += frame_in->imageData[i*step+j*3+2];
							if ((k%5) == 4 && j == width-1){
								R_th = ((now_R[0]/5 + now_R[1]/5 + now_R[2]/5 + now_R[3]/5 + now_R[4]/5) )*1.5;
								G_th = ((now_G[0]/5 + now_G[1]/5 + now_G[2]/5 + now_G[3]/5 + now_G[4]/5) )*1.5;
								B_th = ((now_B[0]/5 + now_B[1]/5 + now_B[2]/5 + now_B[3]/5 + now_B[4]/5) )*1.5;
							
								for (int m = 0 ; m < 5 ; m++ ){
									if ((R_th-now_R[m]) < 0 &&(G_th-now_G[m]) < 0 &&(B_th-now_B[m]) < 0){
										line[i-5+m] = 255;
										if (i>(height/3) && i<((height/3)*2)){
											check_line_mid = 1;
											if (linepoint_mid == 0){
												linepoint_mid = (float)(i-5+m);
											}

										}else if (i>((height/3)*2)){
											check_line_bom = 1;
											if (i > linepoint_bom){
												linepoint_bom = (float)(i-5+m);
												//line[i-5+m] = 255;
											}
										}				
									}
						
								}//end for m
							}
						}//end if (i>(height/3)){
						if (i == (height-1) && j == (width-1)){
							ch_line_mid = check_line_mid;
							ch_line_bom = check_line_bom;
							ch_linepoint_mid = linepoint_mid;
							ch_linepoint_bom = linepoint_bom;
							ch_count = count;
							count = 0;
						}
						if (motion_diff0[k][l] > 5)
							total_reg01++;
						if (motion_diff1[k][l] > 5)
							total_reg02++;
						if (motion_diff2[k][l] > 5)
							total_reg03++;
				
						l++;
					}//end l
					/////////////////////劃出深度圖255-0-255
					//檢測分割線深度資訊
					/*ch_line_mid = 1;
					ch_line_bom = 1;
					ch_linepoint_mid = 240;
					ch_linepoint_bom = 300;*/

					if (ch_line_mid == 1 && ch_line_bom == 0){
						if (i >= 0 && i < ch_linepoint_mid){
							depth_color = (int)floor((double)((back_TH/ch_linepoint_mid)*(i)));
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
						}else if (i>=ch_linepoint_mid && i<height){
							depth_color = back_TH - (int)floor((double)((back_TH/(height-ch_linepoint_mid))*(i-ch_linepoint_mid)));
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
						if (i >= 5 && i < (ch_linepoint_bom-3))
						{
							depth_color = back_TH - (int)floor((double)((back_TH/(ch_linepoint_bom-5))*(i)));
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
						else if (i >= (ch_linepoint_bom-3) && i<height)
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
					/////前景DIBR
					if (Dilation_img[i][j] == 255){//同背景 凹
						if ((j-motion)>0){
							frame_DIBR->imageData[i*step+(j-af_motoin)*3+2] = frame_in->imageData[i*step+j*3+2];
								
						}
						if ((j+motion)<width){
							frame_DIBR->imageData[i*step+(j+af_motoin)*3+0] = frame_in->imageData[i*step+j*3+0];
							frame_DIBR->imageData[i*step+(j+af_motoin)*3+1] = frame_in->imageData[i*step+j*3+1];
						}
					}
					//if (Dilation_img[i][j] == 255){//不同背景 凸
					//	if ((j+motion)<width){
					//		frame_DIBR->imageData[i*step+(j+af_motoin)*3+2] = frame_in->imageData[i*step+j*3+2];
					//			
					//	}
					//	if ((j-motion)>0){
					//		frame_DIBR->imageData[i*step+(j-af_motoin)*3+0] = frame_in->imageData[i*step+j*3+0];
					//		frame_DIBR->imageData[i*step+(j-af_motoin)*3+1] = frame_in->imageData[i*step+j*3+1];
					//	}
					//}
				}//end j

				if ((i%2)==1)
				k++;
			}//end i
			for( i=0 ; i < height ; i++ )
			for( j=0 ; j < width ; j++ ){	
				//if (line[i] == 255){
				if (linepoint_mid == i || linepoint_bom == i){
					frame_in->imageData[i*step+j*3+0] = 255;//B
					frame_in->imageData[i*step+j*3+1] = 255;//G
					frame_in->imageData[i*step+j*3+2] = 255;//R
				}
				
				frame_tt->imageData[i*step+j*3+0] = Dilation_img[i][j];//B
				frame_tt->imageData[i*step+j*3+1] = Dilation_img[i][j];//G
				frame_tt->imageData[i*step+j*3+2] = Dilation_img[i][j];//R
			}
			
			cvShowImage( "ori", frame_tt );
			cvShowImage( "DIBR", frame_DIBR );
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
        
	cvReleaseCapture( &capture );  
    
	return 0;
}

