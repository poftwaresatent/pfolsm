#include "pfolsm.h"

#include <err.h>


int main(int argc, char ** argv)
{
  size_t ii;
  
  struct pfolsm obj;
  if (0 != pfolsm_create (&obj, 12, 12)) {
    errx (EXIT_FAILURE, "failed to create LSM data structure");
  }
  pfolsm_init (&obj);
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
