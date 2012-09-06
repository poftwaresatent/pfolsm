#include <gtk/gtk.h>
#include <err.h>
#include <math.h>
#include <string.h>

#define DIMX 100
#define DIMY 100
#define NX (DIMX + 2)
#define NY (DIMY + 2)
#define NTT (NX * NY)

#define D2R (M_PI / 180.0)

typedef enum {
  PHI,
  SPEED,
  NGFXMODES
} gfxmode_t;

static double phi[NTT];
static double nextphi[NTT];
static double diffx[NTT];
static double diffy[NTT];
static double gradxp[NTT];	/* gradient x component for positive speeds */
static double gradyp[NTT];	/* gradient y component for positive speeds */
static double nablap[NTT];	/* gradient magnitude for positive speeds */
static double gradxm[NTT];	/* gradient x component for negative speeds */
static double gradym[NTT];	/* gradient y component for negative speeds */
static double nablam[NTT];	/* gradient magnitude for negative speeds */
static double speed[NTT];

static GtkWidget * w_phi;
static gint w_phi_width, w_phi_height;
static gint phi_sx, phi_sy, phi_x0, phi_y0;
static double phimin, phimax;
static double speedmin, speedmax;
static int play;
static gfxmode_t gfxmode;


static size_t cidx (size_t ii, size_t jj)
{
  return ii + jj * NX;
}


static void init ()
{
  size_t ii, jj;
  for (ii = 0; ii < NTT; ++ii) {
    phi[ii] = NAN;
    nextphi[ii] = NAN;
    diffx[ii] = NAN;
    diffy[ii] = NAN;
    gradxp[ii] = NAN;
    gradyp[ii] = NAN;
    nablap[ii] = NAN;
    gradxm[ii] = NAN;
    gradym[ii] = NAN;
    nablam[ii] = NAN;
    speed[ii] = 1.0;
  }
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      size_t const idx = cidx(ii, jj);
      phi[idx] = sqrt (pow (10.0 - ii, 2.0) + pow (10.0 - jj, 2.0)) - 4.0;
      nextphi[idx] = phi[idx];
    }
  }
  play = 0;
  gfxmode = PHI;
}


static double upwind (double d0, double d1)
{
  if (d0 > 0.0) {
    if (d1 < 0.0) {
      return d0 > -d1 ? d0 : d1;
    }
    return d0;
  }
  return d1 < 0.0 ? d1 : 0.0;
}


static double downwind (double d0, double d1)
{
  if (d0 < 0.0) {
    if (d1 > 0.0) {
      return -d0 > d1 ? d0 : d1;
    }
    return d0;
  }
  return d1 > 0.0 ? d1 : 0.0;
}


static void update_boundaries ()
{
  size_t ii, jj;
  
  for (ii = 1; ii <= DIMX; ++ii) {
    phi[cidx(ii, 0)] = phi[cidx(ii, 2)];
    phi[cidx(ii, DIMY+1)] = phi[cidx(ii, DIMY-1)];
  }
  for (jj = 1; jj <= DIMY; ++jj) {
    phi[cidx(0, jj)] = phi[cidx(2, jj)];
    phi[cidx(DIMX+1, jj)] = phi[cidx(DIMX-1, jj)];
  }
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
   
   \note Make sure that angle is in the range [-pi,+pi] otherwise
   you'll probably get just the first or last table value.
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


static double max_speed (double gradx, double grady)
{
  /* gentle "egg" */
  /* static double atab[] = { 0.0, M_PI/2, M_PI }; */
  /* static double vtab[] = { 0.7,    1.0, 0.95 }; */
  
  /* static double atab[] = { M_PI/4, M_PI/4+0.01, M_PI/2, 5*M_PI/6, M_PI }; */
  /* static double vtab[] = {    0.2,         0.7,    1.0,      0.8,  0.6 }; */
  
  /* static double atab[] = { 30.0 *D2R,  45.0 *D2R,  100.0 *D2R,  150.0 *D2R, 180.0 *D2R }; */
  /* static double vtab[] = {  -0.1,        0.9,         1.0,         0.9,        0.7      }; */
  
  static double atab_vel[] = { 45.0 *D2R,  100.0 *D2R,  150.0 *D2R, 180.0 *D2R };
  static double vtab_vel[] = {  0.0,         1.0,         0.8,        0.3      };
  static int const len_vel = sizeof(atab_vel) / sizeof(atab_vel[0]);

  static double atab_pen[] = { 15.0 *D2R,  100.0 *D2R };
  static double vtab_pen[] = {  1.0,        0.0      };
  static int const len_pen = sizeof(atab_pen) / sizeof(atab_pen[0]);
  
  // first do a stupid brute force approximation, later implement
  // something smarter like Newton's minimization or maybe we can even
  // find a closed-form solution (influences choice of velocity
  // diagram model).
  double phi;
  double gradl;
  double fmax;
  double alpha;
  
  gradl = sqrt(pow(gradx, 2) + pow(grady, 2));
  if (gradl < 1e-4) {
    return 0;
  }
  gradx /= gradl;
  grady /= gradl;
  alpha = atan2(grady, gradx);
  
  fmax
    = sym_polar_hcspline(-M_PI, atab_vel, vtab_vel, len_vel)
    * gradx
    * sym_polar_hcspline(alpha + M_PI, atab_pen, vtab_pen, len_pen);
  for (phi = -M_PI + M_PI/18; phi < M_PI; phi += M_PI/18) {
    double ff
      = sym_polar_hcspline(phi, atab_vel, vtab_vel, len_vel)
      * (cos(phi) * gradx + sin(phi) * grady)
      * sym_polar_hcspline(alpha - phi, atab_pen, vtab_pen, len_pen);
    if (ff > fmax) {
      fmax = ff;
    }
  }
  
  return fmax;
}


static double simple_speed (double gradx, double grady)
{
  /* gentle "egg" */
  /* static double atab_vel[] = { 0.0, M_PI/2, M_PI }; */
  /* static double vtab_vel[] = { 0.7,    1.0, 0.95 }; */

  /* egg with slight concavity */
  static double atab_vel[] = { 0.0, M_PI/2, M_PI };
  static double vtab_vel[] = { 0.4,    1.0, 0.95 };
  
  /* more realistic but extreme concavities and discontinuities */
  /* static double atab_vel[] = { M_PI/4, M_PI/4+0.01, M_PI/2, 5*M_PI/6, M_PI }; */
  /* static double vtab_vel[] = {    0.2,         0.7,    1.0,      0.8,  0.6 }; */
  
  static int const len_vel = sizeof(atab_vel) / sizeof(atab_vel[0]);
  
  double gradl;
  double alpha;
  
  gradl = sqrt(pow(gradx, 2) + pow(grady, 2));
  if (gradl < 1e-4) {
    return 0;
  }
  gradx /= gradl;
  grady /= gradl;
  alpha = atan2(grady, gradx);
  
  return sym_polar_hcspline(alpha, atab_vel, vtab_vel, len_vel);
}


static void update_speed ()
{
  size_t ii, jj;
  
  speedmax = NAN;
  speedmin = NAN;
  
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      const size_t idx = cidx(ii, jj);
      double speedp, speedm;
      
      /* cute little test */
      /* speedp = 0.4 + gradxp[idx]; */
      /* speedm = 0.4 + gradxm[idx]; */
      
      /* the original trials (and tribulations) and they still behave weirdly */
      /* speedp = 0.5 + 0.5 * cos (2.0 * atan2(gradyp[idx], gradxp[idx])); */
      /* speedm = 0.5 + 0.5 * cos (2.0 * atan2(gradym[idx], gradxm[idx])); */
      
      /* speedp = max_speed(gradxp[idx], gradyp[idx]); */
      /* speedm = max_speed(gradxm[idx], gradym[idx]); */
      speedp = simple_speed(gradxp[idx], gradyp[idx]);
      speedm = simple_speed(gradxm[idx], gradym[idx]);
      
      if (speedp > 0.0) {
	if (speedm > 0.0 || fabs(speedm) < speedp) {
	  speed[idx] = speedp;
	}
	else {
	  speed[idx] = speedm;
	}
      }
      else {
	if (speedm < 0.0) {
	  speed[idx] = speedm;
	}
	else {
	  speed[idx] = 0.0;
	}
      }
      
      if (isnan(speedmax) || speed[idx] > speedmax) {
	speedmax = speed[idx];
      }
      if (isnan(speedmin) || speed[idx] < speedmin) {
	speedmin = speed[idx];
      }
      
    }
  }
  
  printf ("dbg update_speed(): min %f  max %f\n", speedmin, speedmax);
}


static void update ()
{
  static const double dphimax = 0.1;
  double snmax;
  double dt;
  size_t ii, jj;
  
  //////////////////////////////////////////////////
  // get phi from previous nextphi
  
  phimin = NAN;
  phimax = NAN;
  for (ii = 0; ii < NTT; ++ii) {
    phi[ii] = nextphi[ii];
    if (isnan(phi[ii])) {
      continue;
    }
    if (isnan(phimin) || (phimin > phi[ii])) {
      phimin = phi[ii];
    }
    if (isnan(phimax) || (phimax < phi[ii])) {
      phimax = phi[ii];
    }
  }
  
  //////////////////////////////////////////////////
  
  update_boundaries ();
  
  //////////////////////////////////////////////////
  // update finite difference
  
  for (ii = 1; ii < NX; ++ii) {
    for (jj = 1; jj < NY; ++jj) {
      const size_t idx = cidx(ii, jj);
      diffx[idx] = phi[idx] - phi[cidx(ii-1, jj)];
      diffy[idx] = phi[idx] - phi[cidx(ii,   jj-1)];
    }
  }
  
  //////////////////////////////////////////////////  
  // update upwind gradients (for positive and negative speeds)
  
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      const size_t idx = cidx(ii, jj);
      gradxp[idx] = upwind(diffx[idx], diffx[cidx(ii+1, jj)]);
      gradyp[idx] = upwind(diffy[idx], diffy[cidx(ii, jj+1)]);
      gradxm[idx] = downwind(diffx[idx], diffx[cidx(ii+1, jj)]);
      gradym[idx] = downwind(diffy[idx], diffy[cidx(ii, jj+1)]);
      nablap[idx] = sqrt(pow(gradxp[idx], 2.0) + pow(gradyp[idx], 2.0));
      nablam[idx] = sqrt(pow(gradxm[idx], 2.0) + pow(gradym[idx], 2.0));
    }
  }
  
  //////////////////////////////////////////////////
  // Note that the speed compuations depend on gradxp, gradyp, gradxm,
  // gradym above (at least for direction-dependend speed maps).
  
  update_speed ();
  
  //////////////////////////////////////////////////
  // Update rubber time factor
  
  snmax = 0.0;
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      const size_t idx = cidx(ii, jj);
      double sn;
      if (speed[idx] > 0.0) {
	sn = nablap[idx] * speed[idx];
      }
      else {
	sn = - nablam[idx] * speed[idx];
      }
      if (sn > snmax) {
	snmax = sn;
      }
    }
  }
  
  //////////////////////////////////////////////////  
  // compute next phi
  
  if (snmax > 0.0) {		/* should always happen, otherwise we're stuck */
    dt = dphimax / snmax;	/* rubber time */
    for (ii = 1; ii <= DIMX; ++ii) {
      for (jj = 1; jj <= DIMY; ++jj) {
	const size_t idx = cidx(ii, jj);
	if (speed[idx] > 0.0) {
	  nextphi[idx] = phi[idx] - dt * speed[idx] * nablap[idx];
	}
	else {
	  nextphi[idx] = phi[idx] - dt * speed[idx] * nablam[idx];
	}
      }
    }
  }
  
  gtk_widget_queue_draw (w_phi);
}


void cb_gfxmode (GtkWidget * ww, gpointer data)
{
  ++gfxmode;
  gfxmode %= NGFXMODES;
  
  gtk_widget_queue_draw (w_phi);
}


void cb_play (GtkWidget * ww, gpointer data)
{
  if (play) {
    play = 0;
    g_print("PAUSE\n");    
  }
  else {
    play = 1;
    g_print("PLAY\n");    
  }
}


void cb_next (GtkWidget * ww, gpointer data)
{
  if (play) {
    play = 0;
    g_print("PAUSE\n");    
  }
  else {
    update ();
  }    
}


void cb_quit (GtkWidget * ww, gpointer data)
{
  g_print("quit\n");
  gtk_main_quit();
}


gint cb_phi_expose (GtkWidget * ww,
		    GdkEventExpose * ee,
		    gpointer data)
{
  size_t ii, jj;
  cairo_t * cr = gdk_cairo_create (ee->window);
  
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_rectangle (cr, 0, 0, w_phi_width, w_phi_height);
  cairo_fill (cr);
  
  cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
  cairo_set_line_width (cr, 2.0);
  cairo_rectangle (cr, phi_x0 - 2, phi_y0 + 2, DIMX * phi_sx + 4, DIMY * phi_sy - 4);
  cairo_stroke (cr);
  
  double valmin, valmax;
  double * val;
  switch (gfxmode) {
  case PHI:
    valmin = phimin;
    valmax = phimax;
    val = phi;
    break;
  case SPEED:
    valmin = speedmin;
    valmax = speedmax;
    val = speed;
    break;
  default:
    errx (42, "bug: invalid gfxmode");
  };
  
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      const double vv = val[cidx(ii, jj)];
      if (vv >= 0.0) {
  	cairo_set_source_rgb (cr, 0.0, 1.0 - vv / valmax, 0.0);
      }
      else {
  	cairo_set_source_rgb (cr, 1.0 + vv / valmax, 0.0, 0.0);
      }
      cairo_rectangle (cr, phi_x0 + (ii-1) * phi_sx, phi_y0 + jj * phi_sy, phi_sx, - phi_sy);
      cairo_fill (cr);
    }
  }
  
  cairo_destroy (cr);
  
  return TRUE;			// TRUE to stop event propagation
}


gint cb_phi_size_allocate (GtkWidget * ww,
			   GtkAllocation * aa,
			   gpointer data)
{
  w_phi_width = aa->width;
  w_phi_height = aa->height;
  
  phi_sx = w_phi_width / DIMX;
  if (phi_sx < 1) {
    phi_sx = 1;
  }
  phi_sy = - w_phi_height / DIMY;
  if ( - phi_sy < 1) {
    phi_sy = -1;
  }
  if (phi_sx > - phi_sy) {
    phi_sx = - phi_sy;
  }
  else {
    phi_sy = - phi_sx;
  }
  phi_x0 = (w_phi_width - DIMX * phi_sx) / 2;
  phi_y0 = w_phi_height - (w_phi_height + DIMY * phi_sy) / 2;
  
  return TRUE;			// TRUE to stop event propagation
}


gint cb_phi_click (GtkWidget * ww,
		   GdkEventButton * bb,
		   gpointer data)
{
  size_t ii, jj;
  gdouble const cx = (bb->x - phi_x0) / phi_sx + 0.5;
  gdouble const cy = (bb->y - phi_y0) / phi_sy + 0.5;
  
  //// option 1: just reinitialize at click
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      double const dd = sqrt (pow (cx - ii, 2.0) + pow (cy - jj, 2.0)) - 4.0;
      size_t const idx = cidx(ii, jj);
      phi[idx]  = dd;
      nextphi[idx]  = dd;
    }
  }
  
  //// option 2: use min of existing phi and one centered around the click
  // for (ii = 1; ii <= DIMX; ++ii) {
  //   for (jj = 1; jj <= DIMY; ++jj) {
  //     double const dd = sqrt (pow (cx - ii, 2.0) + pow (cy - jj, 2.0)) - 2.0;
  //     size_t const idx = cidx(ii, jj);
  //     if (nextphi[idx] > dd) {
  // 	phi[idx]  = dd;
  // 	nextphi[idx]  = dd;
  //     }
  //   }
  // }
  
  //// first try... was a bit naive maybe. but it did something at least
  // for (ii = 1; ii <= DIMX; ++ii) {
  //   for (jj = 1; jj <= DIMY; ++jj) {
  //     double const dd = sqrt (pow (cx - ii, 2.0) + pow (cy - jj, 2.0));
  //     double aa = 1.0;
  //     if (dd > 2) {
  // 	if (dd > 10) {
  // 	  ++dbg4;
  // 	  continue;
  // 	}
  // 	if (dd > 6.0) {
  // 	  ++dbg3;
  // 	  aa = 0.5 * pow ((10.0 - dd) / 4.0, 2.0);
  // 	}
  // 	else {
  // 	  ++dbg2;
  // 	  aa = 1.0 - 0.5 * pow((dd - 2.0) / 4.0, 2.0);
  // 	}
  //     }
  //     else {
  // 	++dbg1;
  //     }
  //     size_t const idx = cidx(ii, jj);
  //     phi[idx] = aa * (dd - 2.0) + (1.0 - aa) * phi[idx];
  //     nextphi[idx] = aa * (dd - 2.0) + (1.0 - aa) * nextphi[idx];
  //   }
  // }
  
  gtk_widget_queue_draw (w_phi);
  
  return TRUE;			// TRUE to stop event propagation
}


gint idle (gpointer data)
{
  if (play) {
    update ();
  }
  return TRUE;
}


int main (int argc, char ** argv)
{
  GtkWidget *window, *vbox, *hbox, *btn;
  
  gtk_init (&argc, &argv);
  init ();
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);
  
  w_phi = gtk_drawing_area_new ();
  g_signal_connect (w_phi, "expose_event", G_CALLBACK (cb_phi_expose), NULL);
  g_signal_connect (w_phi, "size_allocate", G_CALLBACK (cb_phi_size_allocate), NULL);
  g_signal_connect (w_phi, "button_press_event", G_CALLBACK (cb_phi_click), NULL);
  gtk_widget_set_events (w_phi, GDK_BUTTON_PRESS_MASK);
  
  gtk_widget_show (w_phi);
  
  gtk_widget_set_size_request (w_phi, 400, 500);
  gtk_box_pack_start (GTK_BOX (vbox), w_phi, TRUE, TRUE, 0);
  
  hbox = gtk_hbox_new (TRUE, 3);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_show (hbox);

  
  btn = gtk_button_new_with_label ("gfxmode");
  g_signal_connect (btn, "clicked", G_CALLBACK (cb_gfxmode), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, TRUE, TRUE, 0);
  gtk_widget_show (btn);
  
  btn = gtk_button_new_with_label ("play");
  g_signal_connect (btn, "clicked", G_CALLBACK (cb_play), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, TRUE, TRUE, 0);
  gtk_widget_show (btn);
  
  btn = gtk_button_new_with_label ("next");
  g_signal_connect (btn, "clicked", G_CALLBACK (cb_next), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, TRUE, TRUE, 0);
  gtk_widget_show (btn);
  
  btn = gtk_button_new_with_label ("quit");
  g_signal_connect (btn, "clicked", G_CALLBACK (cb_quit), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, TRUE, TRUE, 0);
  gtk_widget_show (btn);
  
  gtk_idle_add (idle, 0);
  
  gtk_widget_show (window);
  gtk_main ();
  
  return 0;
}
