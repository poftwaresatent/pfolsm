#include <gtk/gtk.h>
#include <err.h>
#include <math.h>

#define DIM 100
#define NN (DIM + 2)

static double phi[NN];
static double diff[NN];
static double nabla[NN];

static GtkWidget * plane;
static int play;


void init ()
{
  int ii;
  for (ii = 0; ii < NN; ++ii) {
    phi[ii] = NAN;
    diff[ii] = NAN;
    nabla[ii] = NAN;
  }
  for (ii = 1; ii <= DIM; ++ii) {
    phi[ii] = ii - 50;
  }
}


void update ()
{
  static const double dt = 0.01;
  int ii;
  
  // 1. update boundary condition
  // 2. update finite difference operators
  // 3. update upwind gradient
  // 4. compute next phi
  
  // 1. update boundary condition
  phi[0] = phi[2];
  phi[NN-1] = phi[NN-3];
  
  // 2. update finite difference operators
  printf ("diff");
  for (ii = 0; ii <= DIM; ++ii) {
    diff[ii] = phi[ii+1] - phi[ii];
    printf (" % 5.1f", diff[ii]);
  }
  printf ("\n");
  
  // 3. update upwind gradient
  printf ("nabla");
  for (ii = 1; ii <= DIM; ++ii) {
    if (diff[ii] > 0.0) {
      nabla[ii] = pow(diff[ii], 2.0);
    }
    else {
      nabla[ii] = 0.0;
    }
    if (diff[ii-1] < 0.0) {
      nabla[ii] += pow(diff[ii-1], 2.0);
    }
    nabla[ii] = sqrt(nabla[ii]);
    printf (" % 5.1f", nabla[ii]);
  }
  printf ("\n");
  
  // 4. compute next phi
  printf ("phi");
  for (ii = 1; ii <= DIM; ++ii) {
    phi[ii] = phi[ii] - dt * nabla[ii];
    printf (" % 5.1f", phi[ii]);
  }
  printf ("\n");
  
  if (plane) {
    gtk_widget_queue_draw (plane);
  }
  else {
    g_warning ("no plane widget to draw");
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


gint cb_plane_expose (GtkWidget * ww,
		      GdkEventExpose * ee,
		      gpointer data)
{
  static double yoff = 200.0;
  cairo_t * cr = gdk_cairo_create (ee->window);
  size_t ii;
  
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_rectangle (cr, 0, 0, 2 * yoff, 2 * yoff);
  cairo_fill (cr);
  
  cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
  cairo_move_to (cr, 0.0, yoff);
  cairo_line_to (cr, DIM, yoff);
  cairo_set_line_width (cr, 1.0);
  cairo_stroke (cr);
  
  cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
  cairo_move_to (cr, 1.0, yoff - phi[1]);
  for (ii = 2; ii <= DIM; ++ii) {
    cairo_line_to (cr, ii, yoff - phi[ii]);
  }
  cairo_set_line_width (cr, 2.0);
  cairo_stroke (cr);
  
  cairo_destroy (cr);
  
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
  GtkBuilder * builder;
  GtkWidget * window;
  GError * error = NULL;
  
  gtk_init (&argc, &argv);
  init ();
  
  builder = gtk_builder_new();
  if ( ! gtk_builder_add_from_file (builder, "gui.glade", &error)) {
    g_warning ("%s", error->message);
    g_free (error);
    return 1;
  }
  
  window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
  plane = GTK_WIDGET (gtk_builder_get_object (builder, "plane"));
  gtk_builder_connect_signals (builder, NULL);
  g_object_unref (G_OBJECT (builder));
  
  if ( ! gtk_idle_add (idle, 0)) {
    g_warning ("failed to register idle function");
    return 2;
  }
  
  gtk_widget_show (window);
  gtk_main ();
  
  return 0;
}
