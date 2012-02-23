#include "pfolsm.h"

#include <math.h>


int pfolsm_create (pfolsm_t * pp,
		   size_t dimx,
		   size_t dimy)
{
  size_t ii;
  double * dd;
  
  if (dimx < 2) {
    dimx = 2;
  }
  if (dimy < 2) {
    dimy = 2;
  }
  
  pp->dimx = dimx;
  pp->dimy = dimy;
  pp->nx   = dimx + 2;
  pp->ny   = dimy + 2;
  pp->ntt  = pp->nx * pp->ny;
  
  pp->data = calloc (8 * pp->ntt, sizeof(*(pp->data)));
  if (0 == pp->data) {
    return -1;
  }
  dd = pp->data;
  for (ii = 0; ii < 8 * pp->ntt; ++ii) {
    *(dd++) = NAN;
  }
  
  pp->speed   = pp->data;
  pp->phi     = pp->data    + pp->ntt;
  pp->phinext = pp->phi     + pp->ntt;
  pp->diffx   = pp->phinext + pp->ntt;
  pp->diffy   = pp->diffx   + pp->ntt;
  pp->gradx   = pp->diffy   + pp->ntt;
  pp->grady   = pp->gradx   + pp->ntt;
  pp->nabla   = pp->grady   + pp->ntt;
  
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
  double * srcbl = dstbl + 2 * pp->nx;
  double * srctr = pp->phi + pp->nx * (pp->dimy - 1) + 1;
  double * dsttr = srctr + 2 * pp->nx;
  
  for (ii = 1; ii <= pp->dimx; ++ii) {
    *(dstbl++) = *(srcbl++);
    *(dsttr++) = *(srctr++);
  }
  
  // go along left and right boundaries
  
  dstbl = pp->phi + pp->nx;
  srcbl = dstbl + 2;
  srctr = dstbl + pp->dimx - 1;
  dsttr = srctr + 2;
  
  for (ii = 1; ii <= pp->dimy; ++ii) {
    *dstbl = *srcbl;
    *dsttr = *srctr;
    dstbl += pp->nx;
    srcbl += pp->nx;
    dsttr += pp->nx;
    srctr += pp->nx;
  }
}


void _pfolsm_diff (pfolsm_t * pp)
{
  size_t ii, jj;
  
  // compute dx
  
  for (jj = 1; jj <= pp->dimy; ++jj) {
    double * dm = pp->phi + jj * pp->nx;
    double * dp = dm + 1;
    double * dst = pp->diffx + jj * pp->nx + 1;
    for (ii = 0; ii <= pp->dimx; ++ii) {
      *(dst++) = *(dp++) - *(dm++);
    }
  }
  
  // compute dy
  
  for (ii = 1; ii <= pp->dimx; ++ii) {
    double * dm = pp->phi + ii;
    double * dp = dm + pp->nx;
    double * dst = pp->diffy + ii + 1;
    for (jj = 0; jj <= pp->dimy; ++jj) {
      *dst = *dp - *dm;
      dst += pp->nx;
      dp += pp->nx;
      dm += pp->nx;
    }
  }
}


static double max3 (double aa, double bb, double cc)
{
  if (aa > bb) {
    return aa > cc ? aa : cc;
  }
  return bb > cc ? bb : cc;
}


void _pfolsm_nabla (pfolsm_t * pp)
{
  size_t ii, jj;
  
  // XXXX to do: reformulate with pointer arithmetic for faster
  // computations.
  
  for (jj = 1; jj <= pp->dimy; ++jj) {
    for (ii = 1; ii <= pp->dimx; ++ii) {
      const size_t idx = ii + jj * pp->nx;
      if (pp->speed[idx] > 0.0) {
	pp->gradx[idx] = max3 (pp->diffx[idx], - pp->diffx[idx+1], 0.0);
	pp->grady[idx] = max3 (pp->diffy[idx], - pp->diffy[idx+pp->nx], 0.0);
      }
      else {
	pp->gradx[idx] = max3 ( - pp->diffx[idx], pp->diffx[idx+1], 0.0);
	pp->grady[idx] = max3 ( - pp->diffy[idx], pp->diffy[idx+pp->nx], 0.0);
      }
      pp->nabla[idx] = sqrt (pow(pp->gradx[idx], 2.0) + pow(pp->grady[idx], 2.0));
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
