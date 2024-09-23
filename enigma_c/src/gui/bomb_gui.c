/*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#include "bomb_gui.h"
#include "turing_bomb/brute_force.h"
#include "helper/helper.h"

#include <gtk/gtk.h>

//
// Created by Emanuel on 07.08.2024.
//


static GtkWidget *known_plaintext;
static GtkTextBuffer *enc_buffer;

static gchar* get_encrypted_text(void)
{
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(enc_buffer, &start_iter);
    gtk_text_buffer_get_end_iter(enc_buffer, &end_iter);
    gchar *encrypted_text =
            gtk_text_buffer_get_text(enc_buffer, &start_iter, &end_iter, FALSE);
    to_uppercase(encrypted_text);
    remove_non_alpha(encrypted_text);
    return encrypted_text;
}

static void action_listener_start(void)
{
    gchar *encrypted_text   = get_encrypted_text();
    const gchar *known_text = gtk_entry_get_text(GTK_ENTRY(known_plaintext));
    crack_enigma(known_text, encrypted_text);
}

static void activate(void)
{
    GtkBuilder *builder;
    GtkWidget *start_btn, *window, *enc_text;


    builder = gtk_builder_new_from_file(FILE_PATH_BOMB);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "bomb_window"));
    assertmsg(GTK_IS_WIDGET(window), "Error: window not found in the Glade file");

    start_btn = GTK_WIDGET(gtk_builder_get_object(builder, "start_btn"));
    assertmsg(GTK_IS_WIDGET(start_btn), "Error: start_btn not found in the Glade file");

    known_plaintext = GTK_WIDGET(gtk_builder_get_object(builder, "known_text"));
    assertmsg(GTK_IS_WIDGET(known_plaintext), "Error: known_text not found in the Glade file");

    enc_text = GTK_WIDGET(gtk_builder_get_object(builder, "enc_text"));
    assertmsg(GTK_IS_WIDGET(enc_text), "Error: enc_text not found in the Glade file");

    enc_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(enc_text));

    g_signal_connect(start_btn, "clicked", G_CALLBACK(action_listener_start), NULL);

    gtk_widget_show_all(window);

    g_object_unref(builder);
}

void run_bomb_gui(void)
{
    activate();
}
