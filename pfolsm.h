#ifndef PFOLSM_H
#define PFOLSM_H

#include <stdlib.h>
#include <stdio.h>


struct pfolsm_s {
  double * speed;
  double * phi;
  double * phinext;
  double * diffx;
  double * diffy;
  double * gradx;
  double * grady;
  double * nabla;
  double * data;
  size_t dimx;
  size_t dimy;
  size_t nx, ny, ntt;
};

typedef struct pfolsm_s pfolsm_t;


int pfolsm_create (pfolsm_t * pp,
		   size_t dimx,
		   size_t dimy);

void pfolsm_destroy (pfolsm_t * pp);

void pfolsm_update (pfolsm_t * pp, double dt);

void pfolsm_dump (pfolsm_t * pp,
		  FILE * fp);


void _pfolsm_cbounds (pfolsm_t * pp);

void _pfolsm_diff (pfolsm_t * pp);

void _pfolsm_nabla (pfolsm_t * pp);

void _pfolsm_cphinext (pfolsm_t * pp, double dt);

void _pfolsm_pdata (pfolsm_t * pp,
		    FILE * fp,
		    double * dbase,
		    void (*pfunc)(FILE *, double));

void _pfolsm_pnum5 (FILE * fp, double num);

void _pfolsm_pnum6 (FILE * fp, double num);


#endif
