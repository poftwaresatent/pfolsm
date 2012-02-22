#include <gtk/gtk.h>
#include <err.h>
#include <math.h>
#include <string.h>

#define DIM 100
#define NN (DIM + 2)

static double phi[NN];
static double nextphi[NN];
static double diff[NN];
static double nabla[NN];

static GtkWidget * w_phi;
static double w_phi_width, w_phi_height;
static GtkWidget * w_diff;
static double w_diff_width, w_diff_height;
static GtkComboBoxText * w_shape;
static int play;


void init ()
{
  int ii;
  for (ii = 0; ii < NN; ++ii) {
    phi[ii] = NAN;
    nextphi[ii] = NAN;
    diff[ii] = NAN;
    nabla[ii] = NAN;
  }
  for (ii = 1; ii < DIM / 2; ++ii) {
    phi[ii] = ii - DIM / 4;
    nextphi[ii] = phi[ii];
  }
  for (ii = DIM / 2; ii <= DIM; ++ii) {
    phi[ii] = 3 * DIM / 4 - ii;
    nextphi[ii] = phi[ii];
  }
}


void update ()
{
  static const double dt = 0.005;
  int ii;
  
  // 0. get phi from previous nextphi
  // 1. update boundary condition
  // 2. update finite difference operators
  // 3. update upwind gradient
  // 4. compute next phi

  // 0. get phi from previous nextphi
  for (ii = 1; ii <= DIM; ++ii) {
    phi[ii] = nextphi[ii];
  }
  
  // 1. update boundary condition
  phi[0] = phi[2];
  phi[DIM+1] = phi[DIM-1];
  
  // 2. update finite difference operators
  for (ii = 0; ii <= DIM; ++ii) {
    diff[ii+1] = phi[ii+1] - phi[ii];
  }
  
  // 3. update upwind gradient
  for (ii = 1; ii <= DIM; ++ii) {
    double dd = diff[ii];
    nabla[ii] = 0.0;
    if (dd > 0.0) {
      nabla[ii] = dd;
    }
    dd = -diff[ii+1];
    if (dd > nabla[ii]) {
      nabla[ii] = dd;
    }
  }
  
  // 4. compute next phi
  for (ii = 1; ii <= DIM; ++ii) {
    nextphi[ii] = phi[ii] - dt * nabla[ii];
  }
  
  gtk_widget_queue_draw (w_phi);
  gtk_widget_queue_draw (w_diff);
}


void cb_apply (GtkWidget * ww, gpointer data)
{
  gchar * shape = gtk_combo_box_text_get_active_text (w_shape);
  if ( ! shape) {
    g_warning ("no shape selected");
    return;
  }
  if (0 == strncmp("tri-up", shape, 8)) {
    int ii;
    for (ii = 1; ii < DIM / 2; ++ii) {
      phi[ii] = ii - DIM / 4;
      nextphi[ii] = phi[ii];
    }
    for (ii = DIM / 2; ii <= DIM; ++ii) {
      phi[ii] = 3 * DIM / 4 - ii;
      nextphi[ii] = phi[ii];
    }
    update ();
  }
  else if (0 == strncmp("tri-down", shape, 8)) {
    int ii;
    for (ii = 1; ii < DIM / 2; ++ii) {
      phi[ii] = DIM / 4 - ii;
      nextphi[ii] = phi[ii];
    }
    for (ii = DIM / 2; ii <= DIM; ++ii) {
      phi[ii] = ii - 3 * DIM / 4;
      nextphi[ii] = phi[ii];
    }
    update ();
  }
  else if (0 == strncmp("sin", shape, 4)) {
    int ii;
    for (ii = 1; ii <= DIM; ++ii) {
      phi[ii] = 40 * sin(2 * M_PI * (ii-1) / DIM);
      nextphi[ii] = phi[ii];
    }
    update ();
  }
  else if (0 == strncmp("cos", shape, 4)) {
    int ii;
    for (ii = 1; ii <= DIM; ++ii) {
      phi[ii] = 40 * cos(2 * M_PI * (ii-1) / DIM);
      nextphi[ii] = phi[ii];
    }
    update ();
  }
  else {
    g_warning ("cannot apply `%s' shape", shape);
  }
  g_free (shape);
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
  cairo_t * cr = gdk_cairo_create (ee->window);
  size_t ii;
  double const xoff = 0.0;
  double const yoff = w_diff_height / 2.0;
  double const xscale = w_diff_width / DIM;
  double const yscale = - w_diff_height / 2.0 / DIM;
  
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_rectangle (cr, 0, 0, w_phi_width, w_phi_height);
  cairo_fill (cr);
  
  cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
  cairo_move_to (cr, xoff, yoff);
  cairo_line_to (cr, xoff + xscale * DIM, yoff);
  cairo_set_line_width (cr, 1.0);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
  cairo_stroke (cr);
  
  cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
  cairo_move_to (cr, xoff + xscale, yoff + yscale * phi[1]);
  for (ii = 2; ii <= DIM; ++ii) {
    cairo_line_to (cr, xoff + xscale * ii, yoff + yscale * phi[ii]);
  }
  cairo_set_line_width (cr, 2.0);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
  cairo_stroke (cr);
  
  cairo_destroy (cr);
  
  return TRUE;			// TRUE to stop event propagation
}


gint cb_diff_expose (GtkWidget * ww,
		     GdkEventExpose * ee,
		     gpointer data)
{
  cairo_t * cr = gdk_cairo_create (ee->window);
  size_t ii;
  double const xoff = 0.0;
  double const yoff = w_diff_height / 2.0;
  double const xscale = w_diff_width / DIM;
  double const yscale = - w_diff_height / 2.0 / 3.0;
  
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_rectangle (cr, 0, 0, w_diff_width, w_diff_height);
  cairo_fill (cr);
  
  cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);
  cairo_move_to (cr, xoff, yoff);
  cairo_line_to (cr, xoff + xscale * DIM, yoff);
  cairo_set_line_width (cr, 1.0);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
  cairo_stroke (cr);
  
  cairo_set_source_rgb (cr, 0.5, 1.0, 0.5);
  cairo_move_to (cr, xoff + xscale, yoff + yscale * diff[1]);
  for (ii = 2; ii <= DIM; ++ii) {
    cairo_line_to (cr, xoff + xscale * ii, yoff + yscale * diff[ii]);
  }
  cairo_set_line_width (cr, 2.0);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
  cairo_stroke (cr);
  
  cairo_set_source_rgb (cr, 1.0, 0.5, 0.5);
  cairo_move_to (cr, xoff + xscale, yoff + yscale * nabla[1]);
  for (ii = 2; ii <= DIM; ++ii) {
    cairo_line_to (cr, xoff + xscale * ii, yoff + yscale * nabla[ii]);
  }
  cairo_set_line_width (cr, 2.0);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);
  cairo_stroke (cr);
  
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


gint cb_diff_size_allocate (GtkWidget * ww,
			   GtkAllocation * aa,
			   gpointer data)
{
  w_diff_width = aa->width;
  w_diff_height = aa->height;
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
  GtkBox * blah;
  
  gtk_init (&argc, &argv);
  init ();
  
  builder = gtk_builder_new();
  if ( ! gtk_builder_add_from_file (builder, "dbglin.glade", &error)) {
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
  w_diff = GTK_WIDGET (gtk_builder_get_object (builder, "diff"));
  if ( ! w_diff) {
    g_warning ("no `diff' widget");
    return 2;
  }
  blah = (GtkBox*) GTK_WIDGET (gtk_builder_get_object (builder, "blah"));
  if ( ! blah) {
    g_warning ("no `blah' widget");
    return 2;
  }
  gtk_builder_connect_signals (builder, NULL);
  g_object_unref (G_OBJECT (builder));
  
  w_shape = (GtkComboBoxText*) gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (w_shape, "tri-up");
  gtk_combo_box_text_append_text (w_shape, "tri-down");
  gtk_combo_box_text_append_text (w_shape, "sin");
  gtk_combo_box_text_append_text (w_shape, "cos");
  gtk_box_pack_start (blah, GTK_WIDGET (w_shape), TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (w_shape));
  
  if ( ! gtk_idle_add (idle, 0)) {
    g_warning ("failed to register idle function");
    return 2;
  }
  
  gtk_widget_show (window);
  gtk_main ();
  
  return 0;
}
