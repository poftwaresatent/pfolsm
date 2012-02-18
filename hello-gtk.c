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
