#include "gaussfit.h"
#include <math.h>
#include <malloc.h>


extern "C" int mylmdif_(int (*fcn)(int *, int *, double *, double *, int *), int *m, int *n, double *x, double *fvec, double *ftol, double *xtol, double *gtol, int *maxfev, 
	double *epsfcn, double *diag, int *mode, double *factor, int *nprint, int *info, int *nfev, double *fjac, int *ldfjac, int *ipvt, 
	double *qtf, double *wa1, double *wa2, double *wa3, double *wa4);
//
//static std::vector <glm::vec3> _worldPts;
//static std::vector <glm::vec2> _imgPts;
//
///*****************************************************************************
//*****************************************************************************/

cv::Mat* img_g;
int current_row_g;
cv::Mat* non_zero_vals_g;

static int
lmdifError_(int *m_ptr, int *n_ptr, double *params, double *error, int *)
{
	int nparms = *n_ptr;
	int nerrors = *m_ptr;

	double *val = (double*)malloc(sizeof(double) * nerrors);

	double mean = params[0];
	double std_dev = params[1];
	double k = params[2];
	//double c = params[3];
	double c = 0;

	// calc error
	for (int i = 0; i < nerrors; ++i) {
		cv::Vec2i pt = non_zero_vals_g->at<cv::Vec2i>(i, 0);
		int x = pt[0];
		val[i] = (k * exp(-0.5 * pow((x - mean)/std_dev, 2.0))) + c;
		error[i] = pow((val[i] - img_g->at<unsigned char>(current_row_g, x)), 2);
	}

	free(val);

	return 1;
}


/*****************************************************************************
*****************************************************************************/
/* Parameters controlling MINPACK's lmdif() optimization routine. */
/* See the file lmdif.f for definitions of each parameter.        */
#define REL_SENSOR_TOLERANCE_ftol    1.0E-6      /* [pix] */
#define REL_PARAM_TOLERANCE_xtol     1.0E-7
#define ORTHO_TOLERANCE_gtol         0.0
#define MAXFEV                       (1000*n)
#define EPSFCN                       1.0E-10 /* was E-16 Do not set to 0! */
#define MODE                         2       /* variables scaled internally */
#define FACTOR                       100.0 


int fit_gauss(cv::Mat& curr_img, int row, cv::Mat & non_zero_vals, int& mid_point)
{
    /* Parameters needed by MINPACK's lmdif() */
	int     n = 3;
	int     m = non_zero_vals.rows;
    double *x;
    double *fvec;
    double  ftol = REL_SENSOR_TOLERANCE_ftol;
    double  xtol = REL_PARAM_TOLERANCE_xtol;
    double  gtol = ORTHO_TOLERANCE_gtol;
    int     maxfev = MAXFEV;
    double  epsfcn = EPSFCN;
    double *diag;
    int     mode = MODE;
    double  factor = FACTOR;
    int     ldfjac = m;
    int     nprint = 0;
    int     info;
    int     nfev;
    double *fjac;
    int    *ipvt;
    double *qtf;
    double *wa1;
    double *wa2;
    double *wa3;
    double *wa4;
	 double worldSize = 256;

	//double mean = params[0];
	//double std_dev = params[1];
	//double k = params[2];
	//double c = params[3];

	 const int num = 3;
	 double params[num];

	 cv::Vec2i pt = non_zero_vals.at<cv::Vec2i>(0, 0);
	 params[0] = pt[0];
	 params[1] = 1.0;
	 params[2] = 1.0;
	 //params[3] = 0.0;

	 /* copy to globals */
	 img_g = &curr_img;
	 current_row_g = row;
	 non_zero_vals_g = &non_zero_vals;

    /* allocate stuff dependent on n */
    x    = (double *)calloc(n, sizeof(double));
    diag = (double *)calloc(n, sizeof(double));
    qtf  = (double *)calloc(n, sizeof(double));
    wa1  = (double *)calloc(n, sizeof(double));
    wa2  = (double *)calloc(n, sizeof(double));
    wa3  = (double *)calloc(n, sizeof(double));
    ipvt = (int    *)calloc(n, sizeof(int));

    /* allocate some workspace */
    if (( fvec = (double *) calloc ((unsigned int) m, 
                                    (unsigned int) sizeof(double))) == NULL ) {
       fprintf(stderr,"calloc: Cannot allocate workspace fvec\n");
       exit(-1);
    }

    if (( fjac = (double *) calloc ((unsigned int) m*n,
                                    (unsigned int) sizeof(double))) == NULL ) {
       fprintf(stderr,"calloc: Cannot allocate workspace fjac\n");
       exit(-1);
    }

    if (( wa4 = (double *) calloc ((unsigned int) m, 
                                   (unsigned int) sizeof(double))) == NULL ) {
       fprintf(stderr,"calloc: Cannot allocate workspace wa4\n");
       exit(-1);
    }


    /* copy parameters in as initial values */
	//std::cout << "initial val. :";
	for (int i = 0; i < n; i++) {
       x[i] = params[i];
	    //std::cout << x[i] << ", ";
	}
	//std::cout << std::endl;

    /* define optional scale factors for the parameters */
    if ( mode == 2 ) {
		int offset = 0;
		for (int offset = 0; offset<n; offset++) {
			diag[offset] = 1.0;
		}
    }

    /* perform the optimization */ 
    //printf("Starting optimization step...\n");
    mylmdif_ (lmdifError_,
            &m, &n, x, fvec, &ftol, &xtol, &gtol, &maxfev, &epsfcn,
            diag, &mode, &factor, &nprint, &info, &nfev, fjac, &ldfjac,
            ipvt, qtf, wa1, wa2, wa3, wa4);
    double totalerror = 0;
    for (int i=0; i<m; i++)
       totalerror += fvec[i];
    printf("\tnum function calls = %i\n", nfev);
    printf("\tremaining total error value = %f\n", totalerror);
    printf("\tor %1.2f per point\n", std::sqrt(totalerror) / m);
    printf("...ended optimization step.\n");

    /* copy result back to parameters array */
	//std::cout << "final val. :";
 //   for (int i=0; i<n; i++) {
 //      params[i] = x[i];
	//    std::cout << x[i] << ", ";
	//}
	//std::cout << std::endl;

	//double mean = params[0];
	//double std_dev = params[1];
	//double k = params[2];
	//double c = params[3];

	// returning mid point of gaussian, which is the highest x value
	mid_point = params[0];



    /* release allocated workspace */
    free (fvec);
    free (fjac);
    free (wa4);
    free (ipvt);
    free (wa1);
    free (wa2);
    free (wa3);
    free (qtf);
    free (diag);
    free (x);

	 return (1);
}