#include <stdio.h>
#include <math.h>


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
   Compute sailboat speed by using the piecewise-cubic interpolation
   provided by hcspline.  The caller specifies a table of angles and
   velocities: for each atab value, the boat would go with the speed
   of the corresponding vtab value.  This function only considers the
   absolute value of the angle.  For angles smaller than the first
   atab value, it simply returns the first vtab value.  Likewise, if
   the angle is bigger than the last atab value, the last vtab value
   is returned.
*/
static double computeSpeed (double angle,
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
  
  angle = fabs(angle);
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


#define D2R (M_PI / 180.0)

int main (int argc, char ** argv) 
{
  double theta, windDirection;
  char separator = ' ';
  
  /* BEWARE of deg2rad below double atab[] = { 0.0, M_PI/2, M_PI }; */
  /* double vtab[] = { 0.7,    1.0, 0.95 }; */
  
  /* BEWARE of deg2rad below double atab[] = { M_PI/4, M_PI/4+0.01, M_PI/2, 5*M_PI/6, M_PI }; */
  /* double vtab[] = {    0.2,         0.7,    1.0,      0.8,  0.6 }; */

  double atab[] = { 30.0 *D2R,  45.0 *D2R,  100.0 *D2R,  150.0 *D2R, 180.0 *D2R };
  double vtab[] = {  0.4,        0.9,         1.0,         0.9,        0.7      };

  int tablen = sizeof(atab) / sizeof(atab[0]);
  
  //  for (windDirection = 0; windDirection <= M_PI/2; windDirection += M_PI/8) {
  windDirection = 0;
  {
    printf ("#wind direction %f rad = %f deg\n", windDirection, windDirection * 180/M_PI);
    for (theta = -M_PI; theta < M_PI; theta += M_PI/100.0) {
      double angle = theta - windDirection;
      
      angle = fmod(angle, 2 * M_PI);
      if (angle > M_PI) {
	angle -= 2*M_PI;
      }
      else if (angle < -M_PI) {
	angle += 2*M_PI;
      }
      
      printf ("%f%c %f%c %f\n",
	      theta, separator,
	      angle, separator,
	      computeSpeed(angle, atab, vtab, tablen)
	      );
    }
    printf ("\n\n");
  }
  
  return 0;
}
