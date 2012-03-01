#include <gtk/gtk.h>
#include <err.h>
#include <math.h>
#include <string.h>

#define DIMX 40
#define DIMY 40
#define NX (DIMX + 2)
#define NY (DIMY + 2)
#define NTT (NX * NY)


static double phi[NTT];
static double nextphi[NTT];
static double diffx[NTT];
static double diffy[NTT];
static double gradx[NTT];
static double grady[NTT];
static double nabla[NTT];
static double speed[NTT];

static GtkWidget * w_phi;
static gint w_phi_width, w_phi_height;
static gint phi_sx, phi_sy, phi_x0, phi_y0;
static double phimin, phimax;
static int play;


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
    gradx[ii] = NAN;
    grady[ii] = NAN;
    nabla[ii] = NAN;
    speed[ii] = 1.0;
  }
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      size_t const idx = cidx(ii, jj);
      phi[idx] = sqrt(ii*ii + jj*jj) - DIMX / 2;
      nextphi[idx] = phi[idx];
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


void update ()
{
  static const double dt = 0.005;
  size_t ii, jj;
  
  ////  printf ("update\n");
  
  // 0. get phi from previous nextphi
  // 1. update boundary condition
  // 2. update finite difference operators
  // 3. update upwind gradient
  // 4. compute next phi

  // 0. get phi from previous nextphi
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
  
  // 1. update boundary condition
  for (ii = 1; ii <= DIMX; ++ii) {
    phi[cidx(ii, 0)] = phi[cidx(ii, 2)];
    phi[cidx(ii, DIMY+1)] = phi[cidx(ii, DIMY-1)];
  }
  for (jj = 1; jj <= DIMY; ++jj) {
    phi[cidx(0, jj)] = phi[cidx(2, jj)];
    phi[cidx(DIMX+1, jj)] = phi[cidx(DIMX-1, jj)];
  }
  
  // 2. update finite difference
  for (ii = 1; ii < NX; ++ii) {
    for (jj = 1; jj < NY; ++jj) {
      const size_t idx = cidx(ii, jj);
      diffx[idx] = phi[idx] - phi[cidx(ii-1, jj)];
      diffy[idx] = phi[idx] - phi[cidx(ii,   jj-1)];
    }
  }
  
  // 3. update upwind gradient
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      const size_t idx = cidx(ii, jj);
      if (speed[ii] > 0.0) {
	gradx[idx] = max3(diffx[idx], -diffx[cidx(ii+1, jj)], 0.0);
	grady[idx] = max3(diffy[idx], -diffy[cidx(ii, jj+1)], 0.0);
      }
      else {
	gradx[idx] = max3(-diffx[idx], diffx[cidx(ii+1, jj)], 0.0);
	grady[idx] = max3(-diffy[idx], diffy[cidx(ii, jj+1)], 0.0);
      }
      nabla[idx] = sqrt(pow(diffx[idx], 2.0) + pow(diffy[idx], 2.0));
    }
  }
  
  // 4. compute next phi
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      const size_t idx = cidx(ii, jj);
      nextphi[idx] = phi[idx] - dt * speed[idx] * nabla[idx];
    }
  }
  
  gtk_widget_queue_draw (w_phi);
}


void cb_reverse (GtkWidget * ww, gpointer data)
{
  int ii;
  const double ss = - speed[0];
  for (ii = 0; ii < NTT; ++ii) {
    speed[ii] = ss;
  }
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
  cairo_rectangle (cr, phi_x0 - 2, phi_y0 - 2, DIMX * phi_sx + 4, DIMY * phi_sy + 4);
  cairo_stroke (cr);
  
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      const double pp = phi[cidx(ii, jj)];
      if (pp >= 0.0) {
  	cairo_set_source_rgb (cr, 0.0, 1.0 - pp / phimax, 0.0);
      }
      else {
  	cairo_set_source_rgb (cr, 1.0 + pp / phimax, 0.0, 0.0);
      }
      cairo_rectangle (cr, phi_x0 + (ii-1) * phi_sx, phi_y0 + (jj-1) * phi_sy, phi_sx, phi_sy);
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
  phi_sy = w_phi_height / DIMY;
  if (phi_sy < 1) {
    phi_sy = 1;
  }
  if (phi_sx > phi_sy) {
    phi_sx = phi_sy;
  }
  else {
    phi_sy = phi_sx;
  }
  phi_x0 = (w_phi_width - DIMX * phi_sx) / 2;
  phi_y0 = (w_phi_height - DIMY * phi_sy) / 2;
  
  return TRUE;			// TRUE to stop event propagation
}


gint cb_phi_button_release (GtkWidget * ww,
			    GdkEventButton * bb,
			    gpointer data)
{
  printf ("WTF???\n");
  
  size_t ii, jj;
  gdouble const cx = bb->x / phi_sx - phi_x0;
  gdouble const cy = bb->y / phi_sy - phi_y0;
  
  for (ii = 1; ii <= DIMX; ++ii) {
    for (jj = 1; jj <= DIMY; ++jj) {
      double const dd = sqrt (pow (cx - ii, 2.0) + pow (cy - jj, 2.0));
      double aa = 1.0;
      if (dd > 2) {
	if (dd > 10) {
	  continue;
	}
	if (dd > 6.0) {
	  aa = 0.5 * pow ((10.0 - dd) / 4.0, 2.0);
	}
	else {
	  aa = 1.0 - 0.5 * pow((dd - 2.0) / 4.0, 2.0);
	}
      }
      size_t const idx = cidx(ii, jj);
      phi[idx] = aa * dd + (1.0 - aa) * phi[idx];
      nextphi[idx] = aa * dd + (1.0 - aa) * nextphi[idx];
    }
  }
  
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

#error 'why does this not end up getting called?'
  g_signal_connect (w_phi, "button_press_event", G_CALLBACK (cb_phi_button_release), NULL);

  gtk_widget_show (w_phi);
  
  gtk_widget_set_size_request (w_phi, 400, 500);
  gtk_box_pack_start (GTK_BOX (vbox), w_phi, TRUE, TRUE, 0);
  
  hbox = gtk_hbox_new (TRUE, 3);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  gtk_widget_show (hbox);

  
  btn = gtk_button_new_with_label ("reverse");
  g_signal_connect (btn, "clicked", G_CALLBACK (cb_reverse), NULL);
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
