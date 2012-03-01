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
  
  printf ("update\n");
  
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
  printf ("  bounds: %g %g\n", phimin, phimax);
  
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
  
  printf ("  queuing redraw\n");
  gtk_widget_queue_draw (w_phi);
  printf ("  done\n");
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
  gint sx, sy, x0, y0;
  
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_rectangle (cr, 0, 0, w_phi_width, w_phi_height);
  cairo_fill (cr);
  
  sx = w_phi_width / DIMX;
  if (sx < 1) {
    sx = 1;
  }
  sy = w_phi_height / DIMY;
  if (sy < 1) {
    sy = 1;
  }
  if (sx > sy) {
    sx = sy;
  }
  else {
    sy = sx;
  }
  x0 = (w_phi_width - DIMX * sx) / 2;
  y0 = (w_phi_height - DIMY * sy) / 2;
  
  cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
  cairo_set_line_width (cr, 2.0);
  cairo_rectangle (cr, x0 - 2, y0 - 2, DIMX * sx + 4, DIMY * sy + 4);
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
      cairo_rectangle (cr, x0 + (ii-1) * sx, y0 + (jj-1) * sy, sx, sy);
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
  return TRUE;			// TRUE to stop event propagation
}


gint idle (gpointer data)
{
  if (play) {
    static size_t wtf = 0;
    printf ("play... %zu\n", wtf++);
    update ();
  }
  gtk_timeout_add (50, idle, 0);
  return FALSE;
}


int main (int argc, char ** argv)
{
  GtkBuilder * builder;
  GtkWidget * window;
  GError * error = NULL;
  
  gtk_init (&argc, &argv);
  init ();
  
  builder = gtk_builder_new();
  if ( ! gtk_builder_add_from_file (builder, "dbgpln.glade", &error)) {
    g_warning ("%s", error->message);
    g_free (error);
    return 1;
  }
  
  window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
  w_phi = GTK_WIDGET (gtk_builder_get_object (builder, "phi"));
  if ( ! w_phi) {
    g_warning ("no `phi' widget");
    return 2;
  }
  gtk_builder_connect_signals (builder, NULL);
  g_object_unref (G_OBJECT (builder));
  
  ////  gtk_idle_add (idle, 0);
  gtk_timeout_add (50, idle, 0);
  
  gtk_widget_show (window);
  gtk_main ();
  
  return 0;
}
