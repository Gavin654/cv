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

#define width_in  704//720   //width of image
#define height_in 576//480  //height of image
//宣告參數
int back_TH = 255;
int M_TH = 2500;
int af_motoin = 2;
int TH_D = 15;
int total_th_min = 100;//100;
int total_th_max = 360*240*0.8;
int TH_min = 360*240*0.01;

//宣告內部空間矩陣
	unsigned char motion_diff0[height_in][width_in];
	unsigned char motion_diff1[height_in][width_in];
	unsigned char motion_diff2[height_in][width_in];
	unsigned char depth_floor[height_in][width_in];
	unsigned char line[height_in];//深度圖
	unsigned char binary_img[height_in][width_in];//量化二值影像 
	unsigned char Dilation_img[height_in][width_in];//膨脹  (慢
	unsigned char D_reg[height_in][width_in];//輸出D
	unsigned char tt[height_in][width_in];//輸出D   
	unsigned char binary_imgA[height_in][width_in];
	unsigned char binary_imgB[height_in][width_in];
	unsigned char binary_imgC[height_in][width_in];
	unsigned char binary_imgD[height_in][width_in];
	unsigned char D_reg_lo[height_in][width_in];
	
int main(int argc, char** argv)
{
	CvCapture *capture;

	IplImage* frame_in; //聲明IplImage指針
	//開新視窗
    cvNamedWindow( "ori", 1 );
	cvNamedWindow( "DIBR", 1 );
	
	//////////資料輸入
	//capture = cvCaptureFromCAM(0);
	capture = cvCaptureFromAVI("D://Homework//研究影片//Sign_irene-720-480.avi");
	
    frame_in = cvQueryFrame(capture);
	///////找尋影像的大小
	int height     = frame_in->height;
	int width      = frame_in->width;
	int step  =	frame_in->widthStep/sizeof(uchar);//step       = frame_in->widthStep;
	printf("h = %d w = %d s = %d\n",height,width,step);
	////存avi影像
	CvVideoWriter *writer;
	int AviForamt = -1;
    int FPS = 25;
    CvSize AviSize = cvSize(width,height);
    int AviColor = 1;
    writer=cvCreateVideoWriter("Output.avi",AviForamt,FPS,AviSize,AviColor);

	/////宣告暫存器
	clock_t clock_start, clock_end, diff_time; 
	int i,j,k,l,frame_counter = 0,count;
	double now_R[5],now_G[5],now_B[5];		//////切出分割線 背景
	double R_th,G_th,B_th; 
	int check_line_mid = 0,check_line_bom = 0,ch_line_mid = 0,ch_line_bom = 0;
	float linepoint_mid= 0,linepoint_bom= height,ch_linepoint_mid= 0,ch_linepoint_bom= height;
	float depth_color = 0;
	int depth_reg=0,motion;
	int m1,m2,m3,jump_cut = 0,a = 0,depth_rest = 0,D_reg_value,D_avg,D_TH;
	int total_th_min_reg1 = 0,total_th_min_reg2 = 0,total_th_min_reg3 = 0,total_th_min_reg4 = 0,total_th_min_reg5 = 0;
	int total_reg01 = 0,total_reg02 = 0,total_reg03 = 0,total_reg04 = 0;
	int choose = 0,r = 0,li = 1000,top = 0,end = 0,ch_r,ch_li,ch_top,ch_end;
	int reg_A,reg_B,reg_C,reg_D;
	
	/////宣告影像空間CV
	IplImage* frame_A = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_B = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_C = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_D = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_Filter = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_DIBR = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_tt = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_tt2 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	

	while(true)	{	
   		if(cvGrabFrame(capture)){
			///計算時間
			
			clock_end = clock();//結束時間 
			if (frame_counter>1)
			printf("frame_counter = %d time=%d ms \n",frame_counter,(clock_end-clock_start));
			clock_start = clock();
			
			frame_in = cvRetrieveFrame(capture);
			check_line_mid = 0;check_line_bom = 0;linepoint_mid= 0;linepoint_bom= height;
			r = 0,li = 1000,top = 0,end = 0;
			k=0;//縮小四分之一大小
			for( i=0 ; i < height ; i++ ){
				line[i] = 0;
				l=0;//縮小四分之一大小
				for( j=0 ; j < width ; j++ ){	
					//if ((i%2)==1 &&(j%2)==1){
						if ((frame_counter%4) == 0){
							frame_A->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//((int)frame_in->imageData[i*step+j*3+0]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+0]+(int)frame_in->imageData[(i-1)*step+j*3+0]+(int)frame_in->imageData[i*step+(j-1)*3+0])/4;//B
							frame_A->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//((int)frame_in->imageData[i*step+j*3+1]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+1]+(int)frame_in->imageData[(i-1)*step+j*3+1]+(int)frame_in->imageData[i*step+(j-1)*3+1])/4;//G
							frame_A->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//((int)frame_in->imageData[i*step+j*3+2]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+2]+(int)frame_in->imageData[(i-1)*step+j*3+2]+(int)frame_in->imageData[i*step+(j-1)*3+2])/4;//R
							motion_diff0[i][j] = abs(frame_A->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1]);
							motion_diff1[i][j] = abs(frame_A->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1]);
							motion_diff2[i][j] = abs(frame_A->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1]);

						}else if ((frame_counter%4) == 1){
							frame_B->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//((int)frame_in->imageData[i*step+j*3+0]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+0]+(int)frame_in->imageData[(i-1)*step+j*3+0]+(int)frame_in->imageData[i*step+(j-1)*3+0])/4;//B
							frame_B->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//((int)frame_in->imageData[i*step+j*3+1]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+1]+(int)frame_in->imageData[(i-1)*step+j*3+1]+(int)frame_in->imageData[i*step+(j-1)*3+1])/4;//G
							frame_B->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//((int)frame_in->imageData[i*step+j*3+2]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+2]+(int)frame_in->imageData[(i-1)*step+j*3+2]+(int)frame_in->imageData[i*step+(j-1)*3+2])/4;//R
							motion_diff0[i][j] = abs(frame_B->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1]);
							motion_diff1[i][j] = abs(frame_B->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1]);
							motion_diff2[i][j] = abs(frame_B->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1]);

						}else if ((frame_counter%4) == 2){
							frame_C->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//((int)frame_in->imageData[i*step+j*3+0]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+0]+(int)frame_in->imageData[(i-1)*step+j*3+0]+(int)frame_in->imageData[i*step+(j-1)*3+0])/4;//B
							frame_C->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//((int)frame_in->imageData[i*step+j*3+1]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+1]+(int)frame_in->imageData[(i-1)*step+j*3+1]+(int)frame_in->imageData[i*step+(j-1)*3+1])/4;//G
							frame_C->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//((int)frame_in->imageData[i*step+j*3+2]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+2]+(int)frame_in->imageData[(i-1)*step+j*3+2]+(int)frame_in->imageData[i*step+(j-1)*3+2])/4;//R
							motion_diff0[i][j] = abs(frame_C->imageData[k*step+l*3+1] - frame_B->imageData[k*step+l*3+1]);
							motion_diff1[i][j] = abs(frame_C->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1]);
							motion_diff2[i][j] = abs(frame_C->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1]);

						}else if ((frame_counter%4) == 3){
							frame_D->imageData[k*step+l*3+0]=frame_in->imageData[i*step+j*3+0];//((int)frame_in->imageData[i*step+j*3+0]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+0]+(int)frame_in->imageData[(i-1)*step+j*3+0]+(int)frame_in->imageData[i*step+(j-1)*3+0])/4;//B
							frame_D->imageData[k*step+l*3+1]=frame_in->imageData[i*step+j*3+1];//((int)frame_in->imageData[i*step+j*3+1]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+1]+(int)frame_in->imageData[(i-1)*step+j*3+1]+(int)frame_in->imageData[i*step+(j-1)*3+1])/4;//G
							frame_D->imageData[k*step+l*3+2]=frame_in->imageData[i*step+j*3+2];//((int)frame_in->imageData[i*step+j*3+2]+(int)frame_in->imageData[(i-1)*step+(j-1)*3+2]+(int)frame_in->imageData[(i-1)*step+j*3+2]+(int)frame_in->imageData[i*step+(j-1)*3+2])/4;//R
							motion_diff0[i][j] = abs(frame_D->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1]);
							motion_diff1[i][j] = abs(frame_D->imageData[k*step+l*3+1] - frame_B->imageData[k*step+l*3+1]);
							motion_diff2[i][j] = abs(frame_D->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1]);

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
								R_th = ((now_R[0]/5 + now_R[1]/5 + now_R[2]/5 + now_R[3]/5 + now_R[4]/5) )*1.08;//1.5
								G_th = ((now_G[0]/5 + now_G[1]/5 + now_G[2]/5 + now_G[3]/5 + now_G[4]/5) )*1.08;
								B_th = ((now_B[0]/5 + now_B[1]/5 + now_B[2]/5 + now_B[3]/5 + now_B[4]/5) )*1.08;
							
								for (int m = 0 ; m < 5 ; m++ ){
									if ((R_th-now_R[m]) < 0 &&(G_th-now_G[m]) < 0 &&(B_th-now_B[m]) < 0){
										line[i-5+m] = 255;
										if (i>(height/3) && i<((height/3)*2)){
											check_line_mid = 0;
											if (linepoint_mid < i){
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
						
						/*if (motion_diff0[k][l] > 20)
							count++;*/
						if (motion_diff0[k][l] > 5)
							total_reg01++;
						if (motion_diff1[k][l] > 5)
							total_reg02++;
						if (motion_diff2[k][l] > 5)
							total_reg03++;
						if (i == (height-1) && j == (width-1)){///最後一點
							ch_line_mid = check_line_mid;
							ch_line_bom = check_line_bom;
							ch_linepoint_mid = linepoint_mid;
							ch_linepoint_bom = linepoint_bom;
						////前景
						/////////////算最小平均值
						total_th_min_reg5 = total_th_min_reg4;
						total_th_min_reg4 = total_th_min_reg3;
						total_th_min_reg3 = total_th_min_reg2;
						total_th_min_reg2 = total_th_min_reg1;
						if(frame_counter >=5)
						total_th_min = (total_th_min_reg5 + total_th_min_reg4 + total_th_min_reg3 + total_th_min_reg2)/4*0.9;

							if (count > total_th_max){
								jump_cut = 1;
								a=1;
							}else{
								if (total_reg01 > total_th_min  ){
									jump_cut = 0;
									depth_rest = 0;
									a=1;
									total_th_min_reg1 = total_reg01;
								}else if (total_reg02 > total_th_min ){
									jump_cut = 0;
									depth_rest = 0;
									a=2;
									total_th_min_reg1 = total_reg02;
								}else if (total_reg03 > total_th_min ){
									jump_cut = 0;
									depth_rest = 1;
									a=3;
									total_th_min_reg1 = total_reg03/2;
								}
							}
							printf("count = %d TH_min = %d\n",count,TH_min);
							if ((frame_counter%10) == 0)
							choose = 0;//快
							if (count < TH_min ){
								a = 0;
								if ((frame_counter%10) == 0)
								choose = 0;//慢
							}
							count = 0;
						}//end if (i == (height-1) && j == (width-1)){
						if (a==1 || a==0){
							if ((frame_counter%4) == 3){
								D_reg_value = abs(frame_D->imageData[k*step+l*3+0] - frame_C->imageData[k*step+l*3+0])+abs(frame_D->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1])+abs(frame_D->imageData[k*step+l*3+2] - frame_C->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 0){
								D_reg_value = abs(frame_A->imageData[k*step+l*3+0] - frame_D->imageData[k*step+l*3+0])+abs(frame_A->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1])+abs(frame_A->imageData[k*step+l*3+2] - frame_D->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 1){
								D_reg_value = abs(frame_B->imageData[k*step+l*3+0] - frame_A->imageData[k*step+l*3+0])+abs(frame_B->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1])+abs(frame_B->imageData[k*step+l*3+2] - frame_A->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 2){
								D_reg_value = abs(frame_C->imageData[k*step+l*3+0] - frame_B->imageData[k*step+l*3+0])+abs(frame_C->imageData[k*step+l*3+1] - frame_B->imageData[k*step+l*3+1])+abs(frame_C->imageData[k*step+l*3+2] - frame_B->imageData[k*step+l*3+2]);
							}
						}else if (a==2){
							if ((frame_counter%4) == 2){
								D_reg_value = abs(frame_C->imageData[k*step+l*3+0] - frame_A->imageData[k*step+l*3+0])+abs(frame_C->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1])+abs(frame_C->imageData[k*step+l*3+2] - frame_A->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 3){
								D_reg_value = abs(frame_D->imageData[k*step+l*3+0] - frame_B->imageData[k*step+l*3+0])+abs(frame_D->imageData[k*step+l*3+1] - frame_B->imageData[k*step+l*3+1])+abs(frame_D->imageData[k*step+l*3+2] - frame_B->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 0){
								D_reg_value = abs(frame_A->imageData[k*step+l*3+0] - frame_C->imageData[k*step+l*3+0])+abs(frame_A->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1])+abs(frame_A->imageData[k*step+l*3+2] - frame_C->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 1){
								D_reg_value = abs(frame_B->imageData[k*step+l*3+0] - frame_D->imageData[k*step+l*3+0])+abs(frame_B->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1])+abs(frame_B->imageData[k*step+l*3+2] - frame_D->imageData[k*step+l*3+2]);
							}
						}else if (a==3)	{
							if ((frame_counter%4) == 1){
								D_reg_value = abs(frame_B->imageData[k*step+l*3+0] - frame_C->imageData[k*step+l*3+0])+abs(frame_B->imageData[k*step+l*3+1] - frame_C->imageData[k*step+l*3+1])+abs(frame_B->imageData[k*step+l*3+2] - frame_C->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 2){
								D_reg_value = abs(frame_C->imageData[k*step+l*3+0] - frame_D->imageData[k*step+l*3+0])+abs(frame_C->imageData[k*step+l*3+1] - frame_D->imageData[k*step+l*3+1])+abs(frame_C->imageData[k*step+l*3+2] - frame_D->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 3){
								D_reg_value = abs(frame_D->imageData[k*step+l*3+0] - frame_A->imageData[k*step+l*3+0])+abs(frame_D->imageData[k*step+l*3+1] - frame_A->imageData[k*step+l*3+1])+abs(frame_D->imageData[k*step+l*3+2] - frame_A->imageData[k*step+l*3+2]);
							}else if ((frame_counter%4) == 0){
								D_reg_value = abs(frame_A->imageData[k*step+l*3+0] - frame_B->imageData[k*step+l*3+0])+abs(frame_A->imageData[k*step+l*3+1] - frame_B->imageData[k*step+l*3+1])+abs(frame_A->imageData[k*step+l*3+2] - frame_B->imageData[k*step+l*3+2]);
							}
						}
						
						//if (k < (height/2) && l<(width/2))
						if (l>100)
							D_reg[k][l] = D_reg_value;
						//else 
						//	D_reg[k][l] = 0;
						////lowpass
						reg_A = D_reg[k-1][l-1];
						reg_B = D_reg[k][l-1];
						reg_C = D_reg[k-1][l];
						reg_D = D_reg[k][l];
						D_reg[k][l] = (unsigned char)(reg_A+reg_B+reg_C+reg_D)/4;


						if (k>5 && l>3){
							D_avg = (int)(D_reg[k][l]+D_reg[k-4][l-2]+D_reg[k-4][l-1]+D_reg[k-4][l]+D_reg[k-3][l-2]+D_reg[k-3][l-1]+D_reg[k-3][l]+
								D_reg[k-2][l-2]+D_reg[k-2][l-1]+D_reg[k-2][l]+D_reg[k-1][l-2]+D_reg[k-1][l-1]+D_reg[k-1][l]+D_reg[k][l-2]+D_reg[k][l-1]+D_reg[k][l])/16;
							D_TH = 256-(D_avg*12);//256-
							if (D_reg[k][l] < D_TH && a>0){
								binary_imgA[k][l] = 0;	
								binary_img[k][l] = 0;	
							}else if (D_reg[k][l] >= D_TH){
								binary_imgA[k][l] = 1;
								binary_img[k][l] = 255;	
								
							}
							binary_imgD[k][l] = binary_imgC[k][l];
							binary_imgC[k][l] = binary_imgB[k][l];
							binary_imgB[k][l] = binary_imgA[k][l];

							
						}
						if ((binary_imgA[k][l]+binary_imgB[k][l]+binary_imgC[k][l]+binary_imgD[k][l])>=3){
							Dilation_img[i][j] = 255;
							/*Dilation_img[i-1][j-1] = 255;
							Dilation_img[i-1][j] = 255;
							Dilation_img[i][j-1] = 255;*/
							/*Dilation_img[i-2][j-2] = 255;
							Dilation_img[i-1][j-2] = 255;
							Dilation_img[i][j-2] = 255;
							Dilation_img[i-2][j-1] = 255;
							Dilation_img[i-2][j] = 255;*/
						}else{
							Dilation_img[i][j] = 0;
							/*Dilation_img[i-1][j-1] = 0;
							Dilation_img[i-1][j] = 0;
							Dilation_img[i][j-1] = 0;*/
						}
						

						if (i>5 && i<(height-5) && j>5 && j<(width-5)){
							if (binary_img[k][l] == 255 && top == 0)
								top = k;
							if (binary_img[k-1][l] == 255 && binary_imgA[k][l] == 0 && end < k)
								end = k;
							if (binary_img[k][l] == 255 && binary_imgA[k][l-1] == 0 && li>l)
								li = l;
							if (binary_img[k][l] == 0 && binary_imgA[k][l-1] == 255 && r<l)
								r = l;
						}
						if (i == (height-1) && j == (width-1)){
							ch_top = top*2;
							ch_end = end*2;
							ch_r = r*2;
							ch_li = li*2;
							printf("choose = %d\n",choose);
							//printf ("top = %d end = %d  li = %d  r = %d \n",top,end,li,r);
						}

						if (binary_img[i][j] == 255)
							count++;
						l++;
					//}//end l
					/////////////////////劃出深度圖255-0-255
					//檢測分割線深度資訊
					/*ch_line_mid = 1;
					ch_line_bom = 1;
					ch_linepoint_mid = 240;
					ch_linepoint_bom = 300;*/

					//if (ch_line_mid == 1 && ch_line_bom == 0){///凹
					//	if (i >= 0 && i < ch_linepoint_mid){
					//		depth_color = (int)floor((double)((back_TH/ch_linepoint_mid)*(i)));
					//		if (depth_color<=0)
					//			depth_color = 0;
					//		depth_floor[i][j] = back_TH - depth_color;
					//		motion = (int)((10*depth_color/(85+depth_color)));
					//		if ((j-motion)>0){
					//			frame_DIBR->imageData[i*step+(j-motion)*3+2] = frame_in->imageData[i*step+j*3+2];
					//		}
					//		if ((j+motion)<width){
					//			frame_DIBR->imageData[i*step+(j+motion)*3+0] = frame_in->imageData[i*step+j*3+0];
					//			frame_DIBR->imageData[i*step+(j+motion)*3+1] = frame_in->imageData[i*step+j*3+1];
					//		}
					//	}else if (i>=ch_linepoint_mid && i<height){
					//		depth_color = back_TH - (int)floor((double)((back_TH/(height-ch_linepoint_mid))*(i-ch_linepoint_mid)));
					//		if (depth_color<=0)
					//			depth_color = 0;
					//		depth_floor[i][j] = back_TH - depth_color;
					//		motion = (int)((10*depth_color/(85+depth_color)));
					//		if ((j-motion)>0){
					//			frame_DIBR->imageData[i*step+(j-motion)*3+2] = frame_in->imageData[i*step+j*3+2];
					//			
					//		}
					//		if ((j+motion)<width){
					//			frame_DIBR->imageData[i*step+(j+motion)*3+0] = frame_in->imageData[i*step+j*3+0];
					//			frame_DIBR->imageData[i*step+(j+motion)*3+1] = frame_in->imageData[i*step+j*3+1];
					//		}
					//	}
					//}else{
					//	//////////////0-255
					//	if (i >= 5 && i < (ch_linepoint_bom-3))
					//	{
					//		depth_color = back_TH - (int)floor((double)((back_TH/(ch_linepoint_bom-5))*(i)));
					//		depth_floor[i][j] = depth_color;
					//		motion = (int)((10*depth_color/(85+depth_color)));
					//		if ((j-motion)>0){
					//			frame_DIBR->imageData[i*step+(j-motion)*3+2] = frame_in->imageData[i*step+j*3+2];
					//		}
					//		if ((j+motion)<width){
					//			frame_DIBR->imageData[i*step+(j+motion)*3+0] = frame_in->imageData[i*step+j*3+0];
					//			frame_DIBR->imageData[i*step+(j+motion)*3+1] = frame_in->imageData[i*step+j*3+1];
					//		}
					//	}
					//	else if (i >= (ch_linepoint_bom-3) && i<height)
					//	{
					//		depth_color = 0;
					//		depth_floor[i][j] = depth_color;
					//		motion = (int)((10*depth_color/(85+depth_color)));
					//		if ((j-motion)>0){
					//			frame_DIBR->imageData[i*step+(j-motion)*3+2] = frame_in->imageData[i*step+j*3+2];
					//			
					//		}
					//		if ((j+motion)<width){
					//			frame_DIBR->imageData[i*step+(j+motion)*3+0] = frame_in->imageData[i*step+j*3+0];
					//			frame_DIBR->imageData[i*step+(j+motion)*3+1] = frame_in->imageData[i*step+j*3+1];
					//		}
					//	}
					//}
						if (ch_line_mid == 1 && ch_line_bom == 0){
						if (i >= 0 && i < ch_linepoint_mid){
							depth_color = (int)floor((double)((back_TH/ch_linepoint_mid)*(i)));
							if (depth_color<=0)
								depth_color = 0;
							depth_floor[i][j] = back_TH - depth_color;
							motion = (int)((10*depth_color/(85+depth_color)));							
						}else if (i>=ch_linepoint_mid && i<height){
							depth_color = back_TH - (int)floor((double)((back_TH/(height-ch_linepoint_mid))*(i-ch_linepoint_mid)));
							if (depth_color<=0)
								depth_color = 0;
							depth_floor[i][j] = back_TH - depth_color;
							motion = (int)((10*depth_color/(85+depth_color)));							
						}
						if ((j+motion)<width){
								frame_DIBR->imageData[i*step+(j+motion)*3+2] = frame_in->imageData[i*step+j*3+2];
						}
						if ((j-motion)>0){
							frame_DIBR->imageData[i*step+(j-motion)*3+0] = frame_in->imageData[i*step+j*3+0];
							frame_DIBR->imageData[i*step+(j-motion)*3+1] = frame_in->imageData[i*step+j*3+1];
						}
					}else{
						//////////////0-255
						if (i >= 5 && i < (ch_linepoint_bom-3))
						{
							depth_color = back_TH - (int)floor((double)((back_TH/(ch_linepoint_bom-5))*(i)));
							depth_floor[i][j] = depth_color;
							motion = (int)((10*depth_color/(85+depth_color)));							
						}
						else if (i >= (ch_linepoint_bom-3) && i<height)
						{
							depth_color = 0;
							depth_floor[i][j] = depth_color;
							motion = (int)((10*depth_color/(85+depth_color)));							
						}
						if (i >= 5 && i<height)	{
							if ((j+motion)<width){
								frame_DIBR->imageData[i*step+(j+motion)*3+2] = frame_in->imageData[i*step+j*3+2];
							}
							if ((j-motion)>0){
								frame_DIBR->imageData[i*step+(j-motion)*3+0] = frame_in->imageData[i*step+j*3+0];
								frame_DIBR->imageData[i*step+(j-motion)*3+1] = frame_in->imageData[i*step+j*3+1];
							}
						}
					}
					/////前景DIBR
					tt[i][j] = 0;
					//if (choose == 0 && jump_cut == 0 && i>ch_top && i< ch_end && j>ch_li && j<ch_r){
					//	tt[i][j] = 255;
					//	if ((j-motion)>0){
					//		frame_DIBR->imageData[i*step+(j-af_motoin)*3+2] = frame_in->imageData[i*step+j*3+2];
					//			
					//	}
					//	if ((j+motion)<width){
					//		frame_DIBR->imageData[i*step+(j+af_motoin)*3+0] = frame_in->imageData[i*step+j*3+0];
					//		frame_DIBR->imageData[i*step+(j+af_motoin)*3+1] = frame_in->imageData[i*step+j*3+1];
					//	}
					//}else if (Dilation_img[i][j] == 255 && jump_cut == 0 && choose == 1){//同背景 凹
					//	if ((j-motion)>0){
					//		frame_DIBR->imageData[i*step+(j-af_motoin)*3+2] = frame_in->imageData[i*step+j*3+2];
					//			
					//	}
					//	if ((j+motion)<width){
					//		frame_DIBR->imageData[i*step+(j+af_motoin)*3+0] = frame_in->imageData[i*step+j*3+0];
					//		frame_DIBR->imageData[i*step+(j+af_motoin)*3+1] = frame_in->imageData[i*step+j*3+1];
					//	}
					//}
					if (Dilation_img[i][j] == 255 && jump_cut == 0){//不同背景 凸
						if ((j+motion)<width){
							frame_DIBR->imageData[i*step+(j+af_motoin)*3+2] = frame_in->imageData[i*step+j*3+2];
								
						}
						if ((j-motion)>0){
							frame_DIBR->imageData[i*step+(j-af_motoin)*3+0] = frame_in->imageData[i*step+j*3+0];
							frame_DIBR->imageData[i*step+(j-af_motoin)*3+1] = frame_in->imageData[i*step+j*3+1];
						}
					}
				}//end j

				//if ((i%2)==1)
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
				
				frame_tt->imageData[i*step+j*3+0] = binary_img[i][j];//B
				frame_tt->imageData[i*step+j*3+1] = binary_img[i][j];//G
				frame_tt->imageData[i*step+j*3+2] = binary_img[i][j];//R
				
			}
			///存影像
			cvWriteFrame(writer,frame_DIBR);

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
    
	cvReleaseVideoWriter(&writer);

	cvReleaseCapture( &capture );  
    
	return 0;
}

