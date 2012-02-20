#include "pfolsm.h"

#include <math.h>


int pfolsm_create (pfolsm_t * pp,
		   size_t dimx,
		   size_t dimy)
{
  size_t ii;
  double * dd;
  
  pp->dimx = dimx;
  pp->dimy = dimy;
  pp->nx = dimx + 2;
  pp->ny = dimy + 2;
  pp->ntt = pp->nx * pp->ny;
  
  pp->data = calloc (5 * pp->ntt, sizeof(*(pp->data)));
  if (0 == pp->data) {
    return -1;
  }
  dd = pp->data;
  for (ii = 0; ii < 5 * pp->ntt; ++ii) {
    *(dd++) = NAN;
  }
  
  pp->phi = pp->data;
  pp->phinext = pp->phi + pp->ntt;
  pp->diffx = pp->phinext + pp->ntt;
  pp->diffy = pp->diffx + pp->ntt;
  pp->nabla = pp->diffy + pp->ntt;
  
  return 0;
}


void pfolsm_destroy (pfolsm_t * pp)
{
  free (pp->data);
}


void _pfolsm_cbounds (pfolsm_t * pp)
{
  size_t ii;
  
  // go along bottom and top boundaries
  
  double * dstbl = pp->phi + 1;
  double * srcbl = dstbl + pp->nx;
  double * srctr = pp->phi + pp->nx * pp->dimy + 1;
  double * dsttr = srctr + pp->nx;
  
  for (ii = 1; ii <= pp->dimx; ++ii) {
    *(dstbl++) = *(srcbl++);
    *(dsttr++) = *(srctr++);
  }
  
  // go along left and right boundaries
  
  dstbl = pp->phi + pp->nx;
  srcbl = dstbl + 1;
  srctr = dstbl + pp->dimx;
  dsttr = srctr + 1;
  
  for (ii = 1; ii <= pp->dimy; ++ii) {
    *dstbl = *srcbl;
    *dsttr = *srctr;
    dstbl += pp->nx;
    srcbl += pp->nx;
    dsttr += pp->nx;
    srctr += pp->nx;
  }
}


void pfolsm_init (pfolsm_t * pp)
{
  size_t ii, jj;
  
  for (jj = 1; jj <= pp->dimy; ++jj) {
    size_t const off = pp->nx * jj;
    for (ii = 1; ii <= pp->dimx; ++ii) {
      pp->phi[ii + off] = jj - 1.0;
      //      pp->phi[ii + off] = sqrt(pow(ii - 1.0, 2.0) + pow(jj - 1.0, 2.0)) - 3.0;
    }
  }
  
  _pfolsm_cbounds (pp);
}


void _pfolsm_diff (pfolsm_t * pp)
{
  size_t ii, jj;
  
  // compute D+x (also on left boundary so we can use it for D-x by
  // shifting)
  
  for (jj = 1; jj <= pp->dimy; ++jj) {
    double * dm = pp->phi + jj * pp->nx;
    double * dp = dm + 1;
    double * dst = pp->diffx + jj * pp->nx;
    for (ii = 0; ii <= pp->dimx; ++ii) {
      *(dst++) = *(dp++) - *(dm++);
    }
  }
  
  // compute D+y (also on bottom boundary so we can use it for D-y by
  // shifting)
  
  for (ii = 1; ii <= pp->dimx; ++ii) {
    double * dm = pp->phi + ii;
    double * dp = dm + pp->nx;
    double * dst = pp->diffy + ii;
    for (jj = 0; jj <= pp->dimy; ++jj) {
      *dst = *dp - *dm;
      dst += pp->nx;
      dp += pp->nx;
      dm += pp->nx;
    }
  }
}


void _pfolsm_nabla (pfolsm_t * pp)
{
  size_t ii, jj;
  
  for (jj = 1; jj <= pp->dimy; ++jj) {
    
    size_t const off0 = jj * pp->nx;
    size_t const off1 = off0 + 1;
    double * dmx = pp->diffx + off0;
    double * dpx = dmx + 1;
    double * dpy = pp->diffy + off1;
    double * dmy = dpy - pp->nx;
    double * nn = pp->nabla + off1;
    
    for (ii = 1; ii <= pp->dimx; ++ii) {
      
      if (*dpx > 0.0) {
	*nn = pow(*dpx, 2.0);
      }
      else {
	*nn = 0.0;
      }
      if (*dmx < 0.0) {
	*nn += pow(*dmx, 2.0);
      }
      if (*dpy > 0.0) {
	*nn += pow(*dpy, 2.0);
      }
      if (*dmy < 0.0) {
	*nn += pow(*dmy, 2.0);
      }
      
      *nn = sqrt(*nn);
      
      ++dmx;
      ++dpx;
      ++dmy;
      ++dpy;
      ++nn;
    }
  }
}


void _pfolsm_cphinext (pfolsm_t * pp, double dt)
{
  size_t ii, jj;
  for (jj = 1; jj <= pp->dimy; ++jj) {
    size_t const off = jj * pp->nx + 1;
    double * nn = pp->nabla + off;
    double * phi = pp->phi + off;
    double * next = pp->phinext + off;
    for (ii = 1; ii <= pp->dimx; ++ii) {
      *(next++) = *(phi++) - dt * (*(nn++)); // unit speed... should come from outside via function or array
    }
  }
}


void pfolsm_update (pfolsm_t * pp, double dt)
{
  double * tmp;
  
  _pfolsm_cbounds (pp);
  _pfolsm_diff (pp);
  _pfolsm_nabla (pp);
  _pfolsm_cphinext (pp, dt);
  
  tmp = pp->phi;
  pp->phi = pp->phinext;
  pp->phinext = tmp;
}


void _pfolsm_pnum5 (FILE * fp, double num)
{
  if (isinf(num)) {
    fprintf(fp, "  inf");
  }
  else if (isnan(num)) {
    fprintf(fp, "  nan");
  }
  else if (fabs(fmod(num, 1)) < 1e-6) {
    fprintf(fp, " % 2d  ", (int) rint(num));
  }
  else {
    fprintf(fp, " % 4.1f", num);
  }
}


void _pfolsm_pnum6 (FILE * fp, double num)
{
  if (isinf(num)) {
    fprintf(fp, "   inf");
  }
  else if (isnan(num)) {
    fprintf(fp, "   nan");
  }
  else if (fabs(fmod(num, 1)) < 1e-6) {
    fprintf(fp, " % 3d  ", (int) rint(num));
  }
  else {
    fprintf(fp, " % 5.1f", num);
  }
}


void _pfolsm_pdata (pfolsm_t * pp,
		    FILE * fp,
		    double * dbase,
		    void (*pfunc)(FILE *, double))
{
  size_t ii, jj;
  double * dd;
  
  for (jj = pp->dimy + 1; jj <= pp->dimy + 1 /* until overflow */; --jj) {
    dd = dbase + pp->nx * jj;
    for (ii = 0; ii < pp->nx; ++ii) {
      pfunc (fp, *(dd++));
    }
    fprintf (fp, "\n");
  }
}


void pfolsm_dump (pfolsm_t * pp,
		  FILE * fp)
{
  fprintf (fp, "==================================================\n");
  fprintf (fp, "phi\n");
  _pfolsm_pdata (pp, fp, pp->phi, _pfolsm_pnum6);
  
  fprintf (fp, "--------------------------------------------------\n");
  fprintf (fp, "diffx\n");
  _pfolsm_pdata (pp, fp, pp->diffx, _pfolsm_pnum6);
  
  fprintf (fp, "--------------------------------------------------\n");
  fprintf (fp, "diffy\n");
  _pfolsm_pdata (pp, fp, pp->diffy, _pfolsm_pnum6);
  
  fprintf (fp, "--------------------------------------------------\n");
  fprintf (fp, "nabla\n");
  _pfolsm_pdata (pp, fp, pp->nabla, _pfolsm_pnum6);
}
