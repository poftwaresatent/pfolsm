#include <gtk/gtk.h>


void cb_play (GtkWidget * ww, gpointer data)
{
  g_print("play\n");
}


void cb_next (GtkWidget * ww, gpointer data)
{
  g_print("next\n");
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
  cairo_t * cr = gdk_cairo_create (ee->window);
  size_t ii, jj;
  for (ii = 0; ii < 100; ii += 10) {
    for (jj = 0; jj < 100; jj += 10) {
      cairo_set_source_rgb (cr, ii / 100.0, jj / 100.0, 0);
      cairo_rectangle (cr, ii, jj, ii + 10, jj + 10);
      cairo_fill (cr);
    }
  }
  
  cairo_destroy (cr);
  
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
  gtk_builder_connect_signals (builder, NULL);
  g_object_unref (G_OBJECT (builder));
  
  gtk_widget_show (window);
  gtk_main ();
  
  return 0;
}
