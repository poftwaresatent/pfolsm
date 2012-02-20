#include "pfolsm.h"

#include <gtk/gtk.h>
#include <err.h>


static pfolsm_t * lsm;
static GtkWidget * plane;


void init ()
{
  lsm = malloc (sizeof(*lsm));
  if ( ! lsm) {
    errx (EXIT_FAILURE, "out of memory");
  }
  if (pfolsm_create (lsm, 20, 20)) {
    errx (EXIT_FAILURE, "failed to create LSM");
  }
  pfolsm_init (lsm);
}


void cleanup ()
{
  if (lsm) {
    pfolsm_destroy (lsm);
    free (lsm);
  }
}


void cb_play (GtkWidget * ww, gpointer data)
{
  g_print("play is not implemented yet\n");
}


void cb_next (GtkWidget * ww, gpointer data)
{
  if ( ! lsm) {
    init ();
  }
  pfolsm_update (lsm, 0.1);
  _pfolsm_pdata (lsm, stdout, lsm->phi, _pfolsm_pnum6);
  
  if (plane) {
    gtk_widget_queue_draw (plane);
  }
  else {
    g_warning ("cb_next: no plane widget to draw");
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
  if (lsm) {
    cairo_t * cr = gdk_cairo_create (ee->window);
    size_t ii, jj;
    for (ii = 1; ii <= lsm->dimx; ++ii) {
      for (jj = 1; jj <= lsm->dimy; ++jj) {
	double const phi = lsm->phi[ii + jj * lsm->nx];
	if (phi < -1.0) {
	  cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
	}
	else if (phi < -0.1) {
	  cairo_set_source_rgb (cr, 1.0, -phi, -phi);
	}
	else if (phi <= 0.1) {
	  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	}
	else if (phi <= 1.0) {
	  cairo_set_source_rgb (cr, phi, 1.0, phi);
	}
	else {
	  cairo_set_source_rgb (cr, 0.0, 1.0, 0.0);
	}
	cairo_rectangle (cr, 4*(ii-1), 4*(jj-1), 4*ii, 4*jj);
	cairo_fill (cr);
      }
    }
    cairo_destroy (cr);
  }
  return TRUE;			// TRUE to stop event propagation
}



int main (int argc, char ** argv)
{
  GtkBuilder * builder;
  GtkWidget * window;
  GError * error = NULL;
  
  gtk_init (&argc, &argv);
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
  
  gtk_widget_show (window);
  gtk_main ();
  
  return 0;
}
