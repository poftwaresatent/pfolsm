#ifndef PFOLSM_H
#define PFOLSM_H

#include <stdlib.h>
#include <stdio.h>


struct pfolsm {
  double * phi;
  double * phinext;
  double * diffx;
  double * diffy;
  double * nabla;
  double * data;
  size_t dimx;
  size_t dimy;
  size_t nx, ny, ntt;
};

int pfolsm_create (struct pfolsm * pp,
		   size_t dimx,
		   size_t dimy);

void pfolsm_destroy (struct pfolsm * pp);

void pfolsm_init (struct pfolsm * pp);

void pfolsm_update (struct pfolsm * pp, double dt);

void pfolsm_dump (struct pfolsm * pp,
		  FILE * fp);


void _pfolsm_diff (struct pfolsm * pp);

void _pfolsm_nabla (struct pfolsm * pp);

void _pfolsm_pdata (struct pfolsm * pp,
		    FILE * fp,
		    double * dbase,
		    void (*pfunc)(FILE *, double));

void _pfolsm_pnum5 (FILE * fp, double num);

void _pfolsm_pnum6 (FILE * fp, double num);


#endif
