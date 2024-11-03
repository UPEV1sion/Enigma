/*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#include "bomb_out.h"

//
// Created by Emanuel on 07.08.2024.
//

static GtkWidget *list;

void add_output_row(const gchar *rotor, const gchar *position, const gchar *ring, const gchar *plugboard)
{
    GtkWidget *row, *grid, *label1, *label2, *label3, *label4;

    row = gtk_list_box_row_new();

    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_container_add(GTK_CONTAINER(row), grid);

    label1 = gtk_label_new(rotor);
    label2 = gtk_label_new(position);
    label3 = gtk_label_new(ring);
    label4 = gtk_label_new(plugboard);

    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 3, 0, 1, 1);

    gtk_list_box_insert(GTK_LIST_BOX(list), row, -1);
    gtk_widget_show_all(row);
}


static void add_title_row(void)
{
    GtkWidget *row, *grid, *label1, *label2, *label3, *label4;

    row = gtk_list_box_row_new();

    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_container_add(GTK_CONTAINER(row), grid);

    label1 = gtk_label_new("Rotors");
    label2 = gtk_label_new("Positions");
    label3 = gtk_label_new("Rings");
    label4 = gtk_label_new("Plugboard");

    PangoAttrList *attr_list = pango_attr_list_new();
    PangoAttribute *attr     = pango_attr_weight_new(PANGO_WEIGHT_SEMIBOLD);
    pango_attr_list_insert(attr_list, attr);
    attr = pango_attr_size_new(14 * PANGO_SCALE);
    pango_attr_list_insert(attr_list, attr);
    gtk_label_set_attributes(GTK_LABEL(label1), attr_list);
    gtk_label_set_attributes(GTK_LABEL(label2), attr_list);
    gtk_label_set_attributes(GTK_LABEL(label3), attr_list);
    gtk_label_set_attributes(GTK_LABEL(label4), attr_list);

    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 3, 0, 1, 1);

    gtk_list_box_insert(GTK_LIST_BOX(list), row, -1);
    gtk_widget_show_all(row);
    g_object_unref(attr_list);
}

static void activate(void)
{
    GtkWidget *window, *sw;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(sw, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(window), sw);

    list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
    gtk_container_add(GTK_CONTAINER(sw), list);

    add_title_row();
    gtk_widget_show_all(window);
    add_output_row("I II III", "A B C", "01 02 03", "AB CD EF");
    add_output_row("IV V VI", "D E F", "04 05 06", "GH IJ KL");

}


void run_gui_bomb_out(void)
{
    activate();
    // gtk_main();
}
