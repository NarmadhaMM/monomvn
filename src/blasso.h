/******************************************************************************** 
 *
 * Estimation for Multivariate Normal Data with Monotone Missingness
 * Copyright (C) 2007, University of Cambridge
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Questions? Contact Robert B. Gramacy (bobby@statslab.cam.ac.uk)
 *
 ********************************************************************************/


#ifndef __BLASSO_H__
#define __BLASSO_H__ 

#include <fstream>

typedef enum REG_MODEL {LASSO=901,OLS=902,RIDGE=903} REG_MODEL;
typedef enum MAT_STATE {NOINIT=1001,COV=1002,CHOLCOV=1003} MAT_STATE;

 /* regression utility structure */
typedef struct bayesreg
{
  unsigned int m;           /* dimension of these matrices and vectors */
  double *bmu;              /* posterior mean for beta */
  double BtAB;              /* in R syntax: t(bmu) %*% A %*% bmu */
  double *XtX_diag;         /* diagonal entries of XtX */
  double **A;               /* inverse of Vb unscaled by s2 */
  double **A_util;          /* utility for inverting A */
  double **Ai;              /* inverse of A */
  double *ABmu;             /* in R syntax: A %*% bmu */
  double **Vb;              /* posterior covariance matrix for beta */
  MAT_STATE Vb_state;       /* state of the Vb matrix */

} BayesReg;


/*
 * CLASS for the implementation of a Bayesian Lasso sampler
 * together with an R interface written in plain C
 */

class Blasso
{
 private:

  REG_MODEL reg_model;
  
  /* design matrix dimension */
  unsigned int M;           /* number of columns in X */
  unsigned int n;           /* number of rows in X */

  /* the original design matrix and transformations */
  double **Xorig;           /* the (original) design matrix */
  bool normalize;           /* normalize X (when TRUE) */
  double *Xnorm;            /* normalization constants for the cols of X */
  double Xnorm_scale;       /* scaling factor for Xnorm normalization constants */
  double *Xmean;            /* mean of each column of X for centering */
  unsigned int ldx;         /* leading dimension of X and Xorig */
  bool copies;              /* indicaties whether the above are copies (TRUE) or
			       memory duplicates of X, Xnorm and Xorig, or just
			       pointers to memory allocated elsewere (FALSE) outside
			       this module */

  /* reversible-jump controlled columns of X in Xp */
  unsigned int m;           /* number of columns/rows of current XtX (breg->A) */
  bool RJ;                  /* indicated whether to do RJ moves or not */
  bool *pb;                 /* booleans indicating the m columns of X that are in use */
  int *pin;                 /* integer list of the columns of X that are in use */  
  int *pout;                /* integer list of the columns of X that are not in use */
  unsigned int Mmax;        /* maximum number of allowable columns in Xp */
  double **Xp;              /* the (normd/centered) design matrix -- only cols in use*/
                            /* in R syntax Xp = X[,pin] */

  /* stuff to do with the response vector Y */
  double *Y;                /* the (centered) response vector */
  double Ymean;             /* the mean of the response vector used for centering */
  double *XtY;              /* in R syntax: t(X) %*% Y */
  double YtY;               /* in R syntax: t(Y) %*% Y, or SSy */
  double *resid;            /* in R syntax: Y - X %*% beta */

  /* model parameters except beta */
  double lambda2;           /* the lasso penalty parameter */
  double s2;                /* the noise variance */
  bool rao_s2;              /* indicates not to use beta when drawing s2 (when TRUE) */
  double a;                 /* IG alpha (scale) prior parameter for s2 */
  double b;                 /* IG beta (inverse-scale) prior parameter for s2 */
  double *tau2i;            /* inverse of diagonal of beta normal prior cov matrix */
  double r;                 /* Gamma alpha (shape) prior parameter for lambda */
  double delta;             /* Gamma beta (scale) prior parameter for lambda */

  BayesReg *breg;           /* matrices and vectors of regression quantities */
  double *beta;             /* sampled regression coefficients, conditional on breg */
  double beta_lprob;        /* the (log) probability of the sampled beta coeffs 
			       (density) of beta ~ N(bmu,Vb) */

  double lpost;             /* log posterior of parameters *not including mu) */

  /* other useful vectors */
  double *rn;               /* vector for N(0,1) draws used to sample beta */
  double *Xbeta_v;          /* untility vector for unnorming beta coefficients */
  double *BtDi;             /* in R syntax: t(B) %*% inv(D) %*% B */
  
  /* printing */
  unsigned int verb;

 protected:

  /* parameter manipulation  */
  void GetParams(double *beta, int *m, double *s2, double *tau2i, 
		 double *lambda2) const;
  void InitIndicators(const unsigned int M, const unsigned int Mmax, 
		      double *beta);
  void InitParams(const REG_MODEL reg_model, double *beta, double s2, double lambda2);
  void InitParams(double * beta, const double lambda2, const double s2, 
		  double *tau2i);
  void InitRegress(void);
  void InitXY(const unsigned int n, double **X, double *Y, const bool normalize);
  void InitXY(const unsigned int n, double **Xorig, double *Xnorm, 
	      const double Xnorm_scale,double *Xmean, const unsigned int ldx, 
	      double *Y, const bool normalize);
  void InitY(const unsigned int n, double *Y);

  /* MCMC rounds */
  void Draw(const unsigned int thin);

  /* RJ moves */
  void RJmove(void);
  void RJdown(double q);
  void RJup(double q);
  void add_col(unsigned int i, unsigned int col);
  void remove_col(unsigned int i, unsigned int col);

 public:

  /* constructors and destructors */
  Blasso(const unsigned int m, const unsigned int n, double **X, double *Y, 
	 const bool RJ, const unsigned int Mmax, double *beta, const double lambda2, 
	 const double s2, double *tau2i, const double r, const double delta, 
	 const double a, const double b, const bool rao_s2, const bool normalize, 
	 const unsigned int verb);
  Blasso(const unsigned int m, const unsigned int n, double **Xorig, double *Xnorm,
	 const double Xnorm_scale, double *Xmean, const unsigned int ldx, double *Y, 
	 const bool RJ, unsigned int Mmax, double *beta_start, const double s2, 
	 const double lambda2_start, const double r, const double delta, 
	 const REG_MODEL reg_model, bool rao_s2, const unsigned int verb);
  ~Blasso();
  void Economize(void);

  /* initialization */
  void Init(void);

  /* access functions */
  REG_MODEL RegModel(void);
  bool UsesRJ(void);

  /* MCMC sampling */
  void Rounds(const unsigned int T, const unsigned int thin, 
	      double *lambda2, double *mu, double **beta, int *m,
	      double *s2, double **tau2i, double *lpost);
  void Draw(const unsigned int thin, double *lambda2, double *mu, double *beta, 
	    int *m, double *s2, double *tau2i, double *lpost);

  /* Bayesian lasso sampling from the full conditionals */
  void DrawBeta(void);
  void DrawS2(void);
  void DrawS2Margin(void);
  void DrawTau2i(void);
  bool Compute(void);
  void DrawLambda2(void);

  /* likelihood and posterior */
  double logPosterior();
    
  /* printing and summary information */
  void PrintInputs(FILE *outfile) const;
  void PrintParams(FILE *outfile) const;
  int Method(void);
  unsigned int Thin(unsigned int thin);
  int Verb(void);
};


/* random number generation */
void mvnrnd(double *x, double *mu, double **cov, double *rn, 
	    const unsigned int n);
double rinvgauss(const double mu, const double lambda);

/* for the inverse gamma distribution */
double Igamma_inv(const double a, const double y, const int lower, 
		  const int ulog);
double Cgamma(const double a, const int log);
double Rgamma_inv(const double a, const double y, const int lower, 
		  const int log);

/* log pdf */
double mvnpdf_log_dup(double *x, double *mu, double **cov, 
		      const unsigned int n);
double mvnpdf_log(double *x, double *mu, double **cov, 
		  const unsigned int n);
double log_determinant_chol(double **M, const unsigned int n);

/* for modular reversible jump */
double mh_accep_ratio(unsigned int n, double *resid, double *x, double bnew, 
		      double t2i, double mub, double vb, double s2);
double draw_beta(const unsigned int m, double *beta, BayesReg* breg, 
	       const double s2, double *rn, const bool lprob);
double log_posterior(const unsigned int n, const unsigned int m, 
		     double *resid, double *beta, const double s2, 
		     double *tau2i, const double lambda2, 
		     const double a, const double b, const double r,
		     const double delta);

/* regression utility structure */
BayesReg* new_BayesReg(const unsigned int m, const unsigned int n, 
		       double **Xp);
void delete_BayesReg(BayesReg* breg);
bool compute_BayesReg(const unsigned int m, double *XtY, double *tau2i, 
		      const double lambda2, const double s2, BayesReg *breg);
void refresh_Vb(BayesReg *breg, const double s2);
double beta_lprob(const unsigned int m, double *beta, BayesReg *breg);
void alloc_rest_BayesReg(BayesReg* breg);
BayesReg* plus1_BayesReg(const unsigned int m, const unsigned int n,
			 BayesReg *old, double *xnew, double **Xp);

#endif
