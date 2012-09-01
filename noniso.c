#include <stdio.h>
#include <math.h>

#define D2R (M_PI / 180.0)


static double modangle (double angle)
{
  angle = fmod(angle, 2 * M_PI);
  if (angle > M_PI) {
    angle -= 2*M_PI;
  }
  else if (angle < -M_PI) {
    angle += 2*M_PI;
  }
  return angle;
}


/**
   C-spline with horizontal tangents between two interpolation points.
   p0 is the value at x0, p1 is the value at x1, and the function
   returns the interpolated value at xx.  The caller is responsible
   for ensuring that x1 > x0.  Also, if xx lies outside that range,
   you'll probably get useless values.
*/
static double hcspline (double p0, double p1, double x0, double x1, double xx)
{
  double const tt = (xx - x0) / (x1 - x0);
  return (2 * tt - 3) * (p0 - p1) * tt * tt + p0;
}


/**
   Symmetric polar piecewise-cubic interpolation based on hcspline.
   The caller specifies a table of angles and values.  This function
   only considers the absolute value of the angle.  For angles smaller
   than the first atab value, it simply returns the first vtab value.
   Likewise, if the angle is bigger than the last atab value, the last
   vtab value is returned.
   
   \note The angle is also put into the range [-pi,+pi] by this
   function, before taking the absolute value.
*/
static double sym_polar_hcspline (double angle,
				  double * atab,
				  double * vtab,
				  int tablen)
{
  int ii;
  if (tablen < 1) {
    return 0.0;
  }
  if (tablen < 2) {
    return vtab[0];
  }
  
  angle = fabs(modangle(angle));
  if (angle < atab[0]) {
    return vtab[0];
  }
  
  for (ii = 1; ii < tablen; ++ii) {
    if (angle <= atab[ii]) {
      return hcspline(vtab[ii-1], vtab[ii], atab[ii-1], atab[ii], angle);
    }
  }
  
  return vtab[tablen - 1];
}


static double compute_speed (double alpha, double phi)
{
  static double atab_vel[] = { 45.0 *D2R,  100.0 *D2R,  150.0 *D2R, 180.0 *D2R };
  static double vtab_vel[] = {  0.0,         1.0,         0.8,        0.3      };
  static int const len_vel = sizeof(atab_vel) / sizeof(atab_vel[0]);

  static double atab_pen[] = { 15.0 *D2R,  100.0 *D2R };
  static double vtab_pen[] = {  1.0,        0.0      };
  static int const len_pen = sizeof(atab_pen) / sizeof(atab_pen[0]);
  
  double gradx, grady;
  gradx = cos(alpha);
  grady = sin(alpha);
  
  return
    sym_polar_hcspline(phi, atab_vel, vtab_vel, len_vel)
    * (cos(phi) * gradx + sin(phi) * grady)
    * sym_polar_hcspline(alpha - phi, atab_pen, vtab_pen, len_pen);
}


int main (int argc, char ** argv) 
{
  double phi, alpha;
  char separator = ' ';
  
  for (alpha = 0.0; alpha < M_PI; alpha += 10 *D2R) {
    printf ("#gradient angle %f rad = %f deg\n", alpha, alpha/D2R);
    for (phi = -M_PI; phi < M_PI; phi += M_PI/100.0) {
      printf ("%f%c %f%c %f\n",
	      alpha, separator,
	      phi, separator,
	      compute_speed (alpha, phi)
	      );
    }
    printf ("\n\n");
  }
  
  return 0;
}
