/*
 * Planar First-Order Level Set Method.
 * 
 * Copyright (C) 2012 Roland Philippsen. All rights reserved.
 *
 * Released under the BSD 3-Clause License.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * 
 * - Neither the name of the copyright holder nor the names of
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
