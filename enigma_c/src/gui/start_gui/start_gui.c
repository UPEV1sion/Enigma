/*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#include <gtk/gtk.h>

#include "start_gui.h"
#include "gui/enigma_gui/enigma_gui.h"
#include "helper/helper.h"
#include "cyclometer/cyclometer.h"
#include "gui/bombe_gui/bombe_gui.h"

//
// Created by Emanuel on 07.08.2024.
//

static void action_listener_enigma_btn(void)
{
    run_gui();
}

static void action_listener_bomb_btn(void)
{
    run_bomb_gui();
    // run_gui_bomb_out();
}

static void activate(GtkApplication *app, char *args[])
{
    GtkBuilder *builder;
    GtkWidget *window, *enigma_btn, *cyclo_btn, *bomb_btn;

//    gchar *path = g_build_path(G_DIR_SEPARATOR_S, g_get_current_dir(),
//                                                 "..", "..", "graphics",
//                                                 "ENIGMA_FETT_WEB_EXP1.png", NULL);
//    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);

    builder = gtk_builder_new_from_file(FILE_PATH_START);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "start_window"));
    assertmsg(GTK_IS_WIDGET(window), "Error: window not found in the Glade file");

    enigma_btn = GTK_WIDGET(gtk_builder_get_object(builder, "enigma_btn"));
    assertmsg(GTK_IS_WIDGET(enigma_btn), "Error: enigma_btn not found in the Glade file");

    cyclo_btn = GTK_WIDGET(gtk_builder_get_object(builder, "cyclo_btn"));
    assertmsg(GTK_IS_WIDGET(cyclo_btn), "Error: cyclo_btn not found in the Glade file");

    bomb_btn = GTK_WIDGET(gtk_builder_get_object(builder, "bomb_btn"));
    assertmsg(GTK_IS_WIDGET(bomb_btn), "Error: bomb_btn not found in the Glade file");

    g_signal_connect(enigma_btn, "clicked", G_CALLBACK(action_listener_enigma_btn), args);

    g_signal_connect(cyclo_btn, "clicked", G_CALLBACK(create_cycles), NULL);

    g_signal_connect(bomb_btn, "clicked", G_CALLBACK(action_listener_bomb_btn), args);

//    if (pixbuf != NULL) {
//        GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 400, 400, GDK_INTERP_BILINEAR);
//
//        gtk_window_set_icon(GTK_WINDOW(window), scaled_pixbuf);
//
//        g_object_unref(scaled_pixbuf);
//        g_object_unref(pixbuf);
//    }

    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_widget_show_all(window);

    g_object_unref(builder);
}

void run_start_gui(char *argv[])
{
    GtkApplication *app = gtk_application_new("com.gui.start", 0);
    g_signal_connect(app, "activate", G_CALLBACK(activate), argv);

    g_application_run(G_APPLICATION(app), 1, argv);

    g_object_unref(app);
}
