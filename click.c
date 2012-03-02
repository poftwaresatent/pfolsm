#include <gtk/gtk.h>
#include <err.h>


static gint cb_click (GtkWidget * ww,
		      GdkEventButton * bb,
		      gpointer data)
{
  fprintf (stderr, "click: %s\n", (char*) data);
  return TRUE;
}


static void cb_quit (GtkWidget * ww,
		     gpointer data)
{
  gtk_main_quit();
}


int main (int argc, char ** argv)
{
  GtkWidget *window, *vbox, *da, *btn;
  
  gtk_init (&argc, &argv);
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  
  da = gtk_drawing_area_new ();
  gtk_widget_set_size_request (da, 500, 500);
  gtk_box_pack_start (GTK_BOX (vbox), da, TRUE, TRUE, 0);
  g_signal_connect (da, "button_press_event", G_CALLBACK (cb_click), "da press");
  g_signal_connect (da, "button_release_event", G_CALLBACK (cb_click), "da release");
  gtk_widget_set_events (da, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  
  btn = gtk_button_new_with_label ("quit");
  g_signal_connect (btn, "clicked", G_CALLBACK (cb_quit), NULL);
  gtk_box_pack_start (GTK_BOX (vbox), btn, FALSE, TRUE, 0);
  
  gtk_widget_show (btn);
  gtk_widget_show (da);
  gtk_widget_show (vbox);
  gtk_widget_show (window);
  gtk_main ();
  
  return 0;
}
