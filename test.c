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

#include "pfolsm.h"

#include <err.h>


static void init (pfolsm_t * pp)
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


int main(int argc, char ** argv)
{
  size_t ii;
  
  pfolsm_t obj;
  if (0 != pfolsm_create (&obj, 12, 12)) {
    errx (EXIT_FAILURE, "failed to create LSM data structure");
  }
  init (&obj);
  _pfolsm_diff (&obj);
  _pfolsm_nabla (&obj);
  pfolsm_dump (&obj, stdout);
  pfolsm_destroy (&obj);
  
  printf ("**************************************************\n");
  for (ii = 0; ii < 200; ++ii) {
    printf ("%zu\n", ii);
    pfolsm_update (&obj, 0.1);
    _pfolsm_pdata (&obj, stdout, obj.phi, _pfolsm_pnum6);
    printf ("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  }
  
  return 0;
}
