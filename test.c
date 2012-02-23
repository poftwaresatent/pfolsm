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
