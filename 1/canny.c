/* Canny edge detection */

#include "stdio.h"
#include "cv.h"
#include "highgui.h"

#define PI 3.1415926535
/* Scale floating point magnitudes and angles to 8 bits */
#define ORI_SCALE 40.0
#define MAG_SCALE 20.0

/* Biggest possible filter mask */
#define MAX_MASK_SIZE 20

/* Fraction of pixels that should be above the HIGH threshold */
float ratio = 0.1f;
int WIDTH = 0;

/* The image header data structure      */
struct header 
{
	int nr, nc;             /* Rows and columns in the image */
	int oi, oj;             /* Origin */
};

/*      The IMAGE data structure        */
struct image 
{
		struct header *info;            /* Pointer to header */
		unsigned char **data;           /* Pixel values */
};

#define SQRT2 1.414213562
#define BLACK 0
#define WHITE 1

long seed = 132531;
typedef struct image * IMAGE;

#if defined (MAX)
int    PBM_SE_ORIGIN_COL=0, PBM_SE_ORIGIN_ROW=0;
char **arg;
int maxargs;
#else
extern int PBM_SE_ORIGIN_COL, PBM_SE_ORIGIN_ROW;
#endif

int range (IMAGE im, int i, int j);
void print_se (IMAGE p);
IMAGE Input_PBM (char *fn);
IMAGE Output_PBM (IMAGE image, char *filename);
void get_num_pbm (FILE *f, char *b, int *bi, int *res);
void pbm_getln (FILE *f, char *b);
void pbm_param (char *s);
struct image  *newimage (int nr, int nc);
void freeimage (struct image  *z);
void sys_abort (int val, char *mess);
void CopyVarImage (IMAGE *a, IMAGE *b);
void Display (IMAGE x);
float ** f2d (int nr, int nc);
void srand32 (long k);
double drand32 ();
void copy (IMAGE *a, IMAGE b);

int trace (int i, int j, int low, IMAGE im,IMAGE mag, IMAGE ori);
float gauss(float x, float sigma);
float dGauss (float x, float sigma);
float meanGauss (float x, float sigma);
void hysteresis (int high, int low, IMAGE im, IMAGE mag, IMAGE oriim);
void canny (float s, IMAGE im, IMAGE mag, IMAGE ori);
void seperable_convolution (IMAGE im, float *gau, int width, 
		float **smx, float **smy);
void dxy_seperable_convolution (float** im, int nr, int nc, float *gau,  
		int width, float **sm, int which);
void nonmax_suppress (float **dx, float **dy, int nr, int nc, 
		IMAGE mag, IMAGE ori);
void estimate_thresh (IMAGE mag, int *low, int *hi);


int range (IMAGE x, int i, int j)
{
	if ( (i>=0) && (i<x->info->nr) && (j>=0) && (j<x->info->nc) )
		return 1;
	else return 0;
}

float norm (float x, float y)
{
	return (float) sqrt ( (double)(x*x + y*y) );
}

void canny (float s, IMAGE im, IMAGE mag, IMAGE ori)
{
	int width;
	float **smx,**smy;
	float **dx,**dy;
	int i,j,n;
	float gau[MAX_MASK_SIZE], dgau[MAX_MASK_SIZE], z;

/* Create a Gaussian and a derivative of Gaussian filter mask */
	for(i=0; i<MAX_MASK_SIZE; i++)
	{
	  gau[i] = meanGauss ((float)i, s);
	  if (gau[i] < 0.005)
	  {
		width = i;
		break;
	  }
	  dgau[i] = dGauss ((float)i, s);
	}

	n = width+width + 1;
	WIDTH = width/2;
	printf ("Smoothing with a Gaussian (width = %d) ...\n", n);

	smx = f2d (im->info->nr, im->info->nc);
	smy = f2d (im->info->nr, im->info->nc);

/* Convolution of source image with a Gaussian in X and Y directions  */
	seperable_convolution (im, gau, width, smx, smy);

/* Now convolve smoothed data with a derivative */
	printf ("Convolution with the derivative of a Gaussian...\n");
	dx = f2d (im->info->nr, im->info->nc);
	dxy_seperable_convolution (smx, im->info->nr, im->info->nc,
		 dgau, width, dx, 1);
	free(smx[0]); free(smx);

	dy = f2d (im->info->nr, im->info->nc);
	dxy_seperable_convolution (smy, im->info->nr, im->info->nc,
		 dgau, width, dy, 0);
	free(smy[0]); free(smy);

/* Create an image of the norm of dx,dy */
	for (i=0; i<im->info->nr; i++)
	  for (j=0; j<im->info->nc; j++)
	  {
	      z = norm (dx[i][j], dy[i][j]);
	      mag->data[i][j] = (unsigned char)(z*MAG_SCALE);
	  }

/* Non-maximum suppression - edge pixels should be a local max */

	nonmax_suppress (dx, dy, (int)im->info->nr, (int)im->info->nc, mag, ori);

	free(dx[0]); free(dx);
	free(dy[0]); free(dy);
}

/*      Gaussian        */
float gauss(float x, float sigma)
{
    float xx;

    if (sigma == 0) return 0.0;
    xx = (float)exp((double) ((-x*x)/(2*sigma*sigma)));
    return xx;
}

float meanGauss (float x, float sigma)
{
	float z;

	z = (gauss(x,sigma)+gauss(x+0.5f,sigma)+gauss(x-0.5f,sigma))/3.0f;
	z = z/(PI*2.0f*sigma*sigma);
	return z;
}

/*      First derivative of Gaussian    */
float dGauss (float x, float sigma)
{
	return -x/(sigma*sigma) * gauss(x, sigma);
}

/*      HYSTERESIS thersholding of edge pixels. Starting at pixels with a
	value greater than the HIGH threshold, trace a connected sequence
	of pixels that have a value greater than the LOW threhsold.        */

void hysteresis (int high, int low, IMAGE im, IMAGE mag, IMAGE oriim)
{
	int i,j;

	printf ("Beginning hysteresis thresholding...\n");
	for (i=0; i<im->info->nr; i++)
	  for (j=0; j<im->info->nc; j++)
	    im->data[i][j] = 0;

	if (high<low)
	{
	  estimate_thresh (mag, &high, &low);
	  printf ("Hysteresis thresholds (from image): HI %d LOW %D\n",
			high, low);
	}
/* For each edge with a magnitude above the high threshold, begin
   tracing edge pixels that are above the low threshold.                */

	for (i=0; i<im->info->nr; i++)
	  for (j=0; j<im->info->nc; j++)
	    if (mag->data[i][j] >= high)
	      trace (i, j, low, im, mag, oriim);

/* Make the edge black (to be the same as the other methods) */
	for (i=0; i<im->info->nr; i++)
	  for (j=0; j<im->info->nc; j++)
	    if (im->data[i][j] == 0) im->data[i][j] = 255;
	    else im->data[i][j] = 0;
}

/*      TRACE - recursively trace edge pixels that have a threshold > the low
	edge threshold, continuing from the pixel at (i,j).                     */

int trace (int i, int j, int low, IMAGE im,IMAGE mag, IMAGE ori)
{
	int n,m;
	char flag = 0;

	if (im->data[i][j] == 0)
	{
	  im->data[i][j] = 255;
	  flag=0;
	  for (n= -1; n<=1; n++)
	  {
	    for(m= -1; m<=1; m++)
	    {
	      if (i==0 && m==0) continue;
	      if (range(mag, i+n, j+m) && mag->data[i+n][j+m] >= low)
		if (trace(i+n, j+m, low, im, mag, ori))
		{
		    flag=1;
		    break;
		}
	    }
	    if (flag) break;
	  }
	  return(1);
	}
	return(0);
}

void seperable_convolution (IMAGE im, float *gau, int width, 
		float **smx, float **smy)
{
	int i,j,k, I1, I2, nr, nc;
	float x, y;

	nr = im->info->nr;
	nc = im->info->nc;

	for (i=0; i<nr; i++)
	  for (j=0; j<nc; j++)
	  {
	    x = gau[0] * im->data[i][j]; y = gau[0] * im->data[i][j];
	    for (k=1; k<width; k++)
	    {
	      I1 = (i+k)%nr; I2 = (i-k+nr)%nr;
	      y += gau[k]*im->data[I1][j] + gau[k]*im->data[I2][j];
	      I1 = (j+k)%nc; I2 = (j-k+nc)%nc;
	      x += gau[k]*im->data[i][I1] + gau[k]*im->data[i][I2];
	    }
	    smx[i][j] = x; smy[i][j] = y;
	  }
}

void dxy_seperable_convolution (float** im, int nr, int nc,  float *gau, 
		int width, float **sm, int which)
{
	int i,j,k, I1, I2;
	float x;

	for (i=0; i<nr; i++)
	  for (j=0; j<nc; j++)
	  {
	    x = 0.0;
	    for (k=1; k<width; k++)
	    {
	      if (which == 0)
	      {
		I1 = (i+k)%nr; I2 = (i-k+nr)%nr;
		x += -gau[k]*im[I1][j] + gau[k]*im[I2][j];
	      }
	      else
	      {
		I1 = (j+k)%nc; I2 = (j-k+nc)%nc;
		x += -gau[k]*im[i][I1] + gau[k]*im[i][I2];
	      }
	    }
	    sm[i][j] = x;
	  }
}

void nonmax_suppress (float **dx, float **dy, int nr, int nc, 
		IMAGE mag, IMAGE ori)
{
	int i,j;
	float xx, yy, g2, g1, g3, g4, g, xc, yc;

	for (i=1; i<mag->info->nr-1; i++)
	{
	  for (j=1; j<mag->info->nc-1; j++)
	  {
	    mag->data[i][j] = 0;

/* Treat the x and y derivatives as components of a vector */
	    xc = dx[i][j];
	    yc = dy[i][j];
	    if (fabs(xc)<0.01 && fabs(yc)<0.01) continue;

	    g  = norm (xc, yc);

/* Follow the gradient direction, as indicated by the direction of
   the vector (xc, yc); retain pixels that are a local maximum. */

	    if (fabs(yc) > fabs(xc))
	    {

/* The Y component is biggest, so gradient direction is basically UP/DOWN */
	      xx = fabs(xc)/fabs(yc);
	      yy = 1.0;

	      g2 = norm (dx[i-1][j], dy[i-1][j]);
	      g4 = norm (dx[i+1][j], dy[i+1][j]);
	      if (xc*yc > 0.0)
	      {
		g3 = norm (dx[i+1][j+1], dy[i+1][j+1]);
		g1 = norm (dx[i-1][j-1], dy[i-1][j-1]);
	      } else
	      {
		g3 = norm (dx[i+1][j-1], dy[i+1][j-1]);
		g1 = norm (dx[i-1][j+1], dy[i-1][j+1]);
	      }

	    } else
	    {

/* The X component is biggest, so gradient direction is basically LEFT/RIGHT */
	      xx = fabs(yc)/fabs(xc);
	      yy = 1.0;

	      g2 = norm (dx[i][j+1], dy[i][j+1]);
	      g4 = norm (dx[i][j-1], dy[i][j-1]);
	      if (xc*yc > 0.0)
	      {
		g3 = norm (dx[i-1][j-1], dy[i-1][j-1]);
		g1 = norm (dx[i+1][j+1], dy[i+1][j+1]);
	      }
	      else
	      {
		g1 = norm (dx[i-1][j+1], dy[i-1][j+1]);
		g3 = norm (dx[i+1][j-1], dy[i+1][j-1]);
	      }
	    }

/* Compute the interpolated value of the gradient magnitude */
	    if ( (g > (xx*g1 + (yy-xx)*g2)) &&
		 (g > (xx*g3 + (yy-xx)*g4)) )
	    {
	      if (g*MAG_SCALE <= 255)
		mag->data[i][j] = (unsigned char)(g*MAG_SCALE);
	      else
		mag->data[i][j] = 255;
	      ori->data[i][j] = (unsigned char) (atan2 (yc, xc) * ORI_SCALE);
	    } else
	    {
		mag->data[i][j] = 0;
		ori->data[i][j] = 0;
	    }

	  }
	}
}

void estimate_thresh (IMAGE mag, int *hi, int *low)
{
	int i,j,k, hist[256], count;

/* Build a histogram of the magnitude image. */
	for (k=0; k<256; k++) hist[k] = 0;

	for (i=WIDTH; i<mag->info->nr-WIDTH; i++)
	  for (j=WIDTH; j<mag->info->nc-WIDTH; j++)
	    hist[mag->data[i][j]]++;

/* The high threshold should be > 80 or 90% of the pixels 
	j = (int)(ratio*mag->info->nr*mag->info->nc);
*/
	j = mag->info->nr;
	if (j<mag->info->nc) j = mag->info->nc;
	j = (int)(0.9*j);
	k = 255;

	count = hist[255];
	while (count < j)
	{
	  k--;
	  if (k<0) break;
	  count += hist[k];
	}
	*hi = k;

	i=0;
	while (hist[i]==0) i++;

	*low = (*hi+i)/2.0f;
}




/*      PRINT_SE - Print a structuring element to the screen    */
void print_se (IMAGE p)
{
	int i,j;

	printf ("\n=====================================================\n");
	if (p == NULL)
	  printf (" Structuring element is NULL.\n");
	else 
	{
	  printf ("Structuring element: %dx%d origin at (%d,%d)\n",
		p->info->nr, p->info->nc, p->info->oi, p->info->oj);
	  for (i=0; i<p->info->nr; i++)
	  {
	    printf ("	");
	    for (j=0; j<p->info->nc; j++)
	      printf ("%4d ", p->data[i][j]);
	    printf ("\n");
	  }
	}
	printf ("\n=====================================================\n");
}

struct image  *newimage (int nr, int nc)
{
	struct image  *x;                /* New image */
	int i;
	unsigned char *p;

	if (nr < 0 || nc < 0) {
		printf ("Error: Bad image size (%d,%d)\n", nr, nc);
		return 0;
	}

/*      Allocate the image structure    */
	x = (struct image  *) malloc( sizeof (struct image) );
	if (!x) {
		printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}

/*      Allocate and initialize the header      */

	x->info = (struct header *)malloc( sizeof(struct header) );
	if (!(x->info)) 
	{
		printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}
	x->info->nr = nr;       x->info->nc = nc;
	x->info->oi = x->info->oj = 0;

/*      Allocate the pixel array        */

	x->data = (unsigned char **)malloc(sizeof(unsigned char *)*nr); 

/* Pointers to rows */
	if (!(x->data)) 
	{
		printf ("Out of storage in NEWIMAGE.\n");
		return 0;
	}

	x->data[0] = (unsigned char *)malloc (nr*nc);
	p = x->data[0];
	if (x->data[0]==0)
	  {
		printf ("Out of storage. Newimage  \n");
		exit(1);
	  }

	for (i=1; i<nr; i++) 
	{
	  x->data[i] = (p+i*nc);
	}

	return x;
}

void freeimage (struct image  *z)
{
/*      Free the storage associated with the image Z    */

	if (z != 0) 
	{
	   free (z->data[0]);
	   free (z->info);
	   free (z->data);
	   free (z);
	}
}

void sys_abort (int val, char *mess)
{
	fprintf (stderr, "**** System library ABORT %d: %s ****\n", 
			val, mess);
	exit (2);
}

void copy (IMAGE *a, IMAGE b)
{
	CopyVarImage (a, &b);
}

void CopyVarImage (IMAGE *a, IMAGE *b)
{
	int i,j;

	if (a == b) return;
	if (*a) freeimage (*a);
	*a = newimage ((*b)->info->nr, (*b)->info->nc);
	if (*a == 0) sys_abort (0, "No more storage.\n");

	for (i=0; i<(*b)->info->nr; i++)
	  for (j=0; j< (*b)->info->nc; j++)
	    (*a)->data[i][j] = (*b)->data[i][j];
	(*a)->info->oi = (*b)->info->oi;
	(*a)->info->oj = (*b)->info->oj;
}

float ** f2d (int nr, int nc)
{
	float **x;
	int i;

	x = (float **)calloc ( nr, sizeof (float *) );
	if (x == 0)
	{
	  fprintf (stderr, "Out of storage: F2D.\n");
	  exit (1);
	}

	for (i=0; i<nr; i++)
	{  
	  x[i] = (float *) calloc ( nc, sizeof (float)  );
	  if (x[i] == 0)
	  {
	    fprintf (stderr, "Out of storage: F2D %d.\n", i);
	    exit (1);
	  }
	}
	return x;
}

void free2d (float **x, int nr)
{
	int i;

	for (i=0; i<nr; i++)
		free(x[i]);
	free (x);
}

/* Small system random number generator */
double drand32 ()
{
	static long a=16807L, m=2147483647L,
		    q=127773L, r = 2836L;
	long lo, hi, test;

	hi = seed / q;
	lo = seed % q;
	test = a*lo -r*hi;
	if (test>0) seed = test;
	else seed = test + m;

	return (double)seed/(double)m;
}

void srand32 (long k)
{
	seed = k;
}


IplImage *toOpenCV (IMAGE x)
{
	IplImage *img;
	int i=0, j=0;
	CvScalar s;
	
	img = cvCreateImage(cvSize(x->info->nc, x->info->nr),8, 1);
	for (i=0; i<x->info->nr; i++)
	{
		for (j=0; j<x->info->nc; j++)
		{
			s.val[0] = x->data[i][j];
			cvSet2D (img, i,j,s);
		}
	}
	return img;
}

IMAGE fromOpenCV (IplImage *x)
{
	IMAGE img;
	int color=0, i=0;
	int k=0, j=0;
	CvScalar s;
	
	if ((x->depth==IPL_DEPTH_8U) &&(x->nChannels==1))								// 1 Pixel (grey) image
		img = newimage (x->height, x->width);
	else if ((x->depth==8) && (x->nChannels==3)) //Color
	{
		color = 1;
		img = newimage (x->height, x->width);
	}
	else return 0;

	for (i=0; i<x->height; i++)
	{
		for (j=0; j<x->width; j++)
		{
			s = cvGet2D (x, i, j);
			if (color) 
			  k = (unsigned char)((s.val[0] + s.val[1] + s.val[2])/3);
			else k = (unsigned char)(s.val[0]);
			img->data[i][j] = k;
		}
	}
	return img;
}

/* Display an image on th escreen */
void display_image (IMAGE x)
{
	IplImage *image = 0;
	char wn[20];
	int i;

	image = toOpenCV (x);
	if (image <= 0) return;

	for (i=0; i<19; i++) wn[i] = (char)((drand32()*26) + 'a');
	wn[19] = '\0';
	cvNamedWindow( wn, CV_WINDOW_AUTOSIZE );
	cvShowImage( wn, image );
	cvWaitKey(0);
	cvReleaseImage( &image );
}

void save_image (IMAGE x, char *name)
{
	IplImage *image = 0;

	image = toOpenCV (x);
	if (image <0) return;

	cvSaveImage( name, image );
	cvReleaseImage( &image );
}

IMAGE get_image (char *name)
{
	IMAGE x=0;
	IplImage *image = 0;

	image = cvLoadImage(name, 0);
	if (image <= 0) return 0;
	x = fromOpenCV (image);
	cvReleaseImage( &image );
	return x;
}

IMAGE grab_image ()
{
	CvCapture  *camera = 0;
	IplImage *image = 0;
	IMAGE x;

	camera = cvCaptureFromCAM( CV_CAP_ANY );
	if( !camera )								// Get a camera?
	{
		fprintf(stderr, "Can’t initialize camera\n");
		return 0;
	}
	image = cvQueryFrame( camera );						
	x = fromOpenCV (image);
	cvReleaseCapture( &camera );
	return x;
}

int get_RGB (IMAGE *r, IMAGE *g, IMAGE *b, char *name)
{
	IplImage *image = 0;
	IMAGE i1, i2, i3;
	int i,j;

	// Read the image from a file into Ip1Image
	image = cvLoadImage(name, 1);
	if (image <= 0) return 0;

	// Create three AIPCV images of correct size
	i1 = newimage (image->height, image->width);
	i2 = newimage (image->height, image->width);
	i3 = newimage (image->height, image->width);

	// Copy pixels from Ip1Image to AIPCV images
	for (i=0; i<image->height; i++)
		for (j=0; j<image->width; j++)
		{
		  i1->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+0];
		  i2->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+1];
		  i3->data[i][j] = (image->imageData+i*image->widthStep)[j*image->nChannels+2];
		}
	cvReleaseImage(&image);
	*r = i3; 
	*g = i2;
	*b = i1;
	return 1;
}

int save_RGB (IMAGE r, IMAGE g, IMAGE b, char *name)
{
	IplImage *image = 0;
	int i,j,k;

	if ( (r->info->nc != g->info->nc) || (r->info->nr != g->info->nr) ) return 0;
	if ( (r->info->nc != b->info->nc) || (r->info->nr != b->info->nr) ) return 0;

	// Create an  IplImage
	image = cvCreateImage(cvSize(r->info->nc, r->info->nr),IPL_DEPTH_8U,3);
	if (image <= 0) return 0;


	// Copy pixels from AIPCV images into Ip1Image
/*		for (i=0; i<r->info->nr; i++)
		{
			for (j=0; j<r->info->nc; j++)
			{
				s.val[0] = b->data[i][j];
				s.val[1] = g->data[i][j];
				s.val[2] = r->data[i][j];
				cvSet2D (image, i,j,s);
		}
	} */
		
	for (i=0; i<image->height; i++)
		for (j=0; j<image->width; j++)
		{
		  (image->imageData+i*image->widthStep)[j*image->nChannels+0] = b->data[i][j];
		  (image->imageData+i*image->widthStep)[j*image->nChannels+1] = g->data[i][j];
		  (image->imageData+i*image->widthStep)[j*image->nChannels+2] = r->data[i][j];
		} 
	k = cvSaveImage(name, image);
	cvReleaseImage(&image);
	return 1-k;

}

int main ()
{
	int i,j;
	float s=1.0;
	int low= 0,high=-1;
	FILE *params;
	IMAGE im, magim, oriim;
	char name[128];

	// Try to read an image
	printf ("Enter path to the image file to be processed: ");
	scanf ("%s", name);
	printf ("Opening file '%s'\n", name);

/* Read parameters from the file canny.par */
	params = fopen ("canny.par", "r");
	if (params)
	{
	  fscanf (params, "%d", &low);  /* Lower threshold */
	  fscanf (params, "%d", &high); /* High threshold */
	  fscanf (params, "%f", &s);   /* Gaussian standard deviation */
	  printf ("Parameters from canny.par: HIGH: %d LOW %d Sigma %f\n",
			high, low, s);
	  fclose (params);
	}
	else printf ("Parameter file 'canny.par' does not exist.\n");

	im = get_image(name);
	display_image (im);

	/* Create local image space */
	magim = newimage (im->info->nr, im->info->nc);
	if (magim == NULL)
	{
	  printf ("Out of storage: Magnitude\n");
	  exit (1);
	}

	oriim = newimage (im->info->nr, im->info->nc);
	if (oriim == NULL)
	{
	  printf ("Out of storage: Orientation\n");
	  exit (1);
	}

/* Apply the filter */
	canny (s, im, magim, oriim);
    
/* Hysteresis thresholding of edge pixels */
	hysteresis (high, low, im, magim, oriim);

	for (i=0; i<WIDTH; i++)
	  for (j=0; j<im->info->nc; j++)
	    im->data[i][j] = 255;

	for (i=im->info->nr-1; i>im->info->nr-1-WIDTH; i--)
	  for (j=0; j<im->info->nc; j++)
	    im->data[i][j] = 255;

	for (i=0; i<im->info->nr; i++)
	  for (j=0; j<WIDTH; j++)
	    im->data[i][j] = 255;

	for (i=0; i<im->info->nr; i++)
	  for (j=im->info->nc-WIDTH-1; j<im->info->nc; j++)
	    im->data[i][j] = 255;



	display_image (im);
	save_image (im, "canny.jpg");

	return 0;
}