#include "pfolsm.h"

#include <gtk/gtk.h>
#include <err.h>
#include <string.h>
#include <math.h>


static pfolsm_t * lsm;
static GtkWidget * w_phi;
static GtkComboBoxText * w_init;
static GtkComboBoxText * w_speed;

static double timestep;
static int play;


void cleanup ()
{
  if (lsm) {
    pfolsm_destroy (lsm);
    free (lsm);
  }
}


static void update ()
{
  double * tmp;
  
  // Here we first swap phi and nextphi, because we want to be able to
  // plot the derivatives and the gradient of the /current/ phi. In
  // normal operations, it probably makes more sense to always swap
  // phi and phinext immediately after the update (ie at the end of
  // this function, instead of at the beginning).
  
  tmp = lsm->phi;
  lsm->phi = lsm->phinext;
  lsm->phinext = tmp;
  
  // The following LSM internal operations are just copy-pasted from
  // pfolsm_update.

  _pfolsm_cbounds (lsm);
  _pfolsm_diff (lsm);
  _pfolsm_nabla (lsm);
  _pfolsm_cphinext (lsm, timestep);
  
  // Tell GTK that it needs to redraw.
  
  gtk_widget_queue_draw (w_phi);
}


static void init_circle (double cx, double cy, double rr, double scale, double speed)
{
  size_t ii, jj;
  for (ii = 1; ii <= lsm->dimx; ++ii) {
    for (jj = 1; jj <= lsm->dimy; ++jj) {
      const size_t idx = ii + jj * lsm->nx;
      lsm->phi[idx] = scale * (sqrt(pow(ii - cx, 2.0) + pow(jj - cy, 2.0)) - rr);
      lsm->phinext[idx] = lsm->phi[idx];
      lsm->speed[idx] = speed;
    }
  }
}


void cb_init (GtkWidget * ww, gpointer data)
{
  gchar * shape;

  shape = gtk_combo_box_text_get_active_text (w_init);
  if ( ! shape) {
    g_warning ("no shape selected");
    return;
  }
  
  if (0 == strncmp("circle-up", shape, 9)) {
    init_circle (1.0 + lsm->dimx / 2.0, 1.0 + lsm->dimy / 2.0, lsm->dimx / 4.0, -1.0, 1.0);
    update ();
  }
  else if (0 == strncmp("circle-down", shape, 11)) {
    init_circle (1.0 + lsm->dimx / 2.0, 1.0 + lsm->dimy / 2.0, lsm->dimx / 4.0, 1.0, 1.0);
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
  size_t ii, jj;
  for (ii = 1; ii <= lsm->dimx; ++ii) {
    for (jj = 1; jj <= lsm->dimy; ++jj) {
      double const phi = lsm->phi[ii + jj * lsm->nx];
      // if (phi < -1.0) {
      // 	cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
      // }
      // else if (phi < -0.1) {
      // 	cairo_set_source_rgb (cr, 1.0, -phi, -phi);
      // }
      // else if (phi <= 0.1) {
      // 	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
      // }
      // else if (phi <= 1.0) {
      // 	cairo_set_source_rgb (cr, phi, 1.0, phi);
      // }
      // else {
      // 	cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
      // }
      if (phi < -1e-3) {
	cairo_set_source_rgb (cr, 0.5, 0.0, 0.0);
      }
      else if (phi > 1e-3) {
	cairo_set_source_rgb (cr, 0.5, 1.0, 0.5);
      }
      else {
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
      }
      cairo_rectangle (cr, 4*(ii-1), 4*(jj-1), 4*ii, 4*jj);
      cairo_fill (cr);
    }
  }
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
  GtkBox * hb_init;
  GtkBox * hb_speed;
  
  if (0 != atexit(cleanup)) {
    err (EXIT_FAILURE, "failed to add cleanup function");
  }
  
  gtk_init (&argc, &argv);
  
  timestep = 0.01;
  lsm = malloc (sizeof(*lsm));
  if ( ! lsm) {
    errx (EXIT_FAILURE, "out of memory");
  }
  if (pfolsm_create (lsm, 40, 60)) {
    errx (EXIT_FAILURE, "failed to create LSM");
  }
  init_circle (1.0 + lsm->dimx / 2.0, 1.0 + lsm->dimy / 2.0, lsm->dimx / 4.0, 1.0, 1.0);
  
  builder = gtk_builder_new();
  if ( ! gtk_builder_add_from_file (builder, "gui.glade", &error)) {
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
  hb_init = (GtkBox*) GTK_WIDGET (gtk_builder_get_object (builder, "hb_init"));
  if ( ! hb_init) {
    g_warning ("no `hb_init' widget");
    return 2;
  }
  hb_speed = (GtkBox*) GTK_WIDGET (gtk_builder_get_object (builder, "hb_speed"));
  if ( ! hb_speed) {
    g_warning ("no `hb_speed' widget");
    return 2;
  }
  gtk_builder_connect_signals (builder, NULL);
  g_object_unref (G_OBJECT (builder));
  
  w_init = (GtkComboBoxText*) gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (w_init, "circle-up");
  gtk_combo_box_text_append_text (w_init, "circle-down");
  gtk_box_pack_start (hb_init, GTK_WIDGET (w_init), TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (w_init));
  
  w_speed = (GtkComboBoxText*) gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (w_speed, "1.0");
  gtk_combo_box_text_append_text (w_speed, "-1.0");
  gtk_box_pack_start (hb_speed, GTK_WIDGET (w_speed), TRUE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (w_speed));
  
  if ( ! gtk_idle_add (idle, 0)) {
    g_warning ("failed to register idle function");
    return 2;
  }
  
  gtk_widget_show (window);
  gtk_main ();
  
  return 0;
}
