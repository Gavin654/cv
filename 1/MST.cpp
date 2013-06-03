#include "stdafx.h"
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "conio.h"

#define width_in  1600   //width of image
#define height_in 1200  //height of image

///宣告影像空間
	unsigned char line[height_in];//分割線
	unsigned char depth_floor[height_in][width_in];//深度圖
	unsigned char Y[height_in][width_in];//Y
	unsigned char R[height_in][width_in];//Y
	unsigned char G[height_in][width_in];//Y
	unsigned char B[height_in][width_in];//Y
	unsigned char FF[height_in][width_in];//旗標
	unsigned char depth_out[height_in][width_in];//輸出
	int Y_reg[height_in][width_in];//Y
	int avg[height_in][width_in];

//宣告參數
int back_TH = 255;
int TH_diff = 30;

int main(int argc, char** argv)
{
	FILE *fp;

	IplImage* frame_in = 0; //聲明IplImage指針
	//開新視窗
    cvNamedWindow( "ori", 1 );
	cvNamedWindow( "DIBR", 1 );
	
	//////////資料輸入
	frame_in = cvLoadImage("D:\\[013850].jpg");
	 
   	///////找尋影像的大小
	int height     = frame_in->height;
	int width      = frame_in->width;
	int step  =	frame_in->widthStep/sizeof(uchar);//step       = frame_in->widthStep;
	printf("h = %d w = %d s = %d\n",height,width,step);
	/////宣告暫存器
	int i,j,k,l;
	int avg_reg = 0,diff_bom,diff_right,diff_top,diff_li;
	
	/////宣告影像空間CV
	IplImage* frame_DIBR = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
    IplImage* frame_gray = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	IplImage* frame_avg = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	IplImage* frame_reg = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	
	for( i=0 ; i < height ; i++ ){
		for( j=0 ; j < width ; j++ ){	
			//Y[i][j] = frame_gray->imageData[i*width+j];
			B[i][j] = frame_in->imageData[i*step+j*3+0];
			G[i][j] = frame_in->imageData[i*step+j*3+1];
			R[i][j] = frame_in->imageData[i*step+j*3+2];
			Y[i][j] = R[i][j]*(0.299) + G[i][j]*(0.587) + B[i][j]*(0.114);  
			/*if (i>=0)
			Y[i][j]=10;
			if (i>=12)
			Y[i][j]=100;
			if (i>=50)
			Y[i][j]=200;		*/	

			depth_floor[i][j] = back_TH - (int)floor((double)((back_TH*i/height)));
			FF[i][j] = 0;
		}//end j		
	}//end i
	/////////////////演算法
	for( i=0 ; i < height ; i++ ){
		for( j=0 ; j < width ; j++ ){				
			if ((i%4)==0 && (j%4)==0){
			avg[i][j] = Y[i][j]+Y[i][j+1]+Y[i][j+2]+Y[i][j+3]+
				Y[i+1][j]+Y[i+1][j+1]+Y[i+1][j+2]+Y[i+1][j+3]+
				Y[i+2][j]+Y[i+2][j+1]+Y[i+2][j+2]+Y[i+2][j+3]+
				Y[i+3][j]+Y[i+3][j+1]+Y[i+3][j+2]+Y[i+3][j+3];	
			for( k=0 ; k < 4 ; k++ )
				for( l=0 ; l < 4 ; l++ ){
					avg[i+k][j+l] = avg[i][j];
				}
			}

		}//end j		
	}//end i

	for( i=0 ; i < height ; i=i+4 ){
		for( j=0 ; j < width ; j=j+4 ){	
			if ((i%4)==0 && (j%4)==0){
				diff_top = abs(avg[i][j]-avg[i-4][j]);
				diff_li = abs(avg[i][j]-avg[i][j-4]);
				diff_bom = abs(avg[i][j]-avg[i+4][j]);
				diff_right = abs(avg[i][j]-avg[i][j+4]);
			//	printf("[%d][%d] avg = %d avg_right = %d avg_bom = %d bom = %d right = %d\n",i+k,j+l,avg[i][j],avg[i][j+4],avg[i+4][j],diff_bom,diff_right);
			//	printf("FF = %d FF_R = %d FF_B = %d\n",FF[i][j],FF[i][j+4],FF[i+4][j]);
			}
				/*printf("top = %d li = %d\n",diff_top,diff_li);
				_getch();*/
				for( k=0 ; k <= 3 ; k++ )
					for( l=0 ; l <= 3 ; l++ ){
					//	printf("[%d][%d] bom = %d right = %d\n",i+k,j+l,diff_bom,diff_right);
						
					/*if (i == 0 ){///第一行皆為0
						depth_out[i+k][j+l] = 0;
						FF[i+k][j+l] = 1;
						printf("1\n");

					}else*/ if (diff_bom<=TH_diff && diff_right<=TH_diff && FF[i+4][j] == 0 && FF[i][j+4] == 0 ){	//下右皆為0，直接給值
						if (FF[i][j] == 0){
							depth_out[i+k][j+l] = depth_floor[i][j] ;
							if ((k%4)==3 && (l%4)==3){
								FF[i][j] = 255;
							//printf("  0  ");
							//_getch();
							}
						}
						depth_out[i+k+4][j+l] = depth_out[i][j] ;
						depth_out[i+k][j+l+4] = depth_out[i][j] ;

						if ((k%4)==3 && (l%4)==3){
							FF[i+4][j] = 255;						
							FF[i][j+4] = 255;
						//	printf("  2  ");
							//_getch();
						}	
//
					}else if (diff_right<=TH_diff && FF[i][j] == 0 && FF[i][j+4] == 255 ){	//原點為0，右邊點為1，右→原點
						depth_out[i+k][j+l] = depth_out[i][j+4];
						//printf("3");
						//printf("FF = %d FF_R = %d\n",FF[i][j],FF[i][j+4]);
					//	_getch();

					}else if (diff_bom<=TH_diff && FF[i][j] == 0 && FF[i+4][j] == 255 ){	//原點為0，下邊點為1，下→原點
						depth_out[i+k][j+l] = depth_out[i+4][j];
						//printf("4\n");

					}else if (diff_right<=TH_diff && FF[i][j+4] == 0 ){	//給右邊點				
						if (FF[i][j+4] == 0)
							depth_out[i+k][j+l] = depth_floor[i][j];

							depth_out[i+k][j+l+4] = depth_out[i][j];
							if ((k%4)==3 && (l%4)==3){
								FF[i][j+4] = 255;					
								//printf("5\n");
							}

					}else if (diff_bom<=TH_diff && FF[i+4][j] == 0 ){		//給下邊點
						if (FF[i][j] == 0)
							depth_out[i+k][j+l] = depth_floor[i][j];
								
							depth_out[i+k+4][j+l] = depth_out[i][j];
						
						if ((k%4)==3 && (l%4)==3){
							FF[i+4][j] = 255;
							//printf("6\n");
						}
					}else if (depth_out[i+k][j+l] <= 0 && FF[i][j] == 0){
						depth_out[i+k][j+l]=depth_floor[i][j];
					}
				}
				//	printf("[%d][%d] bom = %d right = %d",i+k,j+l,diff_bom,diff_right);
					//_getch();
			
		}//end j		
	}//end i
	
	///////////////輸出
		for( i=0 ; i < height ; i++ )
			for( j=0 ; j < width ; j++ ){	
				//if (line[i] == 255){
								
				frame_avg->imageData[i*step+j*3+0] = depth_out[i][j];//B
				frame_avg->imageData[i*step+j*3+1] = depth_out[i][j];//G
				frame_avg->imageData[i*step+j*3+2] = depth_out[i][j];//R

				frame_reg->imageData[i*step+j*3+0] = Y[i][j];//B
				frame_reg->imageData[i*step+j*3+1] = Y[i][j];//G
				frame_reg->imageData[i*step+j*3+2] = Y[i][j];//R
			}
			
	fp=fopen("D:/tt.raw","wb"); //寫圖
	fwrite(avg,height,width,fp);
	fclose(fp);

	   cvShowImage( "ori", frame_reg );
	   cvShowImage( "DIBR", frame_avg );
	   //存影像
	   cvSaveImage("D:\\3-4.jpg",frame_avg);
	 
	cvWaitKey(0);
        
	cvDestroyWindow( "ori" );//銷毀視窗
	cvDestroyWindow( "DIBR" );//銷毀視窗
	
	return 0;
}

