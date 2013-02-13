/*
 * Planar First-Order Level Set Method.
 * 
 * Copyright (C) 2012 Roland Philippsen. All rights reserved.
 *
 * Released under the BSD 3-Clause License.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * 
 * - Neither the name of the copyright holder nor the names of
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
