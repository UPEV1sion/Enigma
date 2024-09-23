/*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#include <ctype.h>
#include <stdbool.h>

#include "enigma_gui.h"
#include "json/json.h"
#include "helper/helper.h"

//
// Created by Emanuel on 25.07.2024.
//


#define BUFFER_SIZE 1024

static GtkWidget *window, *model, *reflector, *plugboard, *start, *input, *output;
static GtkWidget *rotors[4], *positions[4], *rings[4];
static GtkTextBuffer *in_buffer;

uint8_t get_enigma_type_from_gui(void)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(model)) + 3;
}

uint8_t* get_rotors_from_gui(void)
{
    const uint8_t num_rotors = get_enigma_type_from_gui();
    uint8_t *rotor_arr       = malloc(sizeof(uint8_t) * num_rotors);
    assertmsg(rotor_arr != NULL, "rotors == NULL");
    for (uint8_t i = 0; i < num_rotors; ++i)
    {
        rotor_arr[i] = gtk_combo_box_get_active(GTK_COMBO_BOX(rotors[i])) + 1;
    }

    return rotor_arr;
}

uint8_t* get_rotor_positions_from_gui(void)
{
    const uint8_t num_rotors    = get_enigma_type_from_gui();
    uint8_t *rotor_position_arr = malloc(sizeof(uint8_t) * num_rotors);
    assertmsg(rotor_position_arr != NULL, "rotors == NULL");
    for (uint8_t i = 0; i < num_rotors; ++i)
    {
        rotor_position_arr[i] =
                gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(positions[i])) - 1;
    }

    return rotor_position_arr;
}

uint8_t* get_rotor_ring_positions_from_gui(void)
{
    const uint8_t num_rotors    = get_enigma_type_from_gui();
    uint8_t *rotor_position_arr = malloc(sizeof(uint8_t) * num_rotors);
    assertmsg(rotor_position_arr != NULL, "rotors == NULL");
    for (uint8_t i = 0; i < num_rotors; ++i)
    {
        rotor_position_arr[i] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rings[i])) - 1;
    }

    return rotor_position_arr;
}

char get_reflector_type_from_gui(void)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(model));
}

char* get_plugboard_from_gui(void)
{
    const gchar *plugboard_text = gtk_entry_get_text(GTK_ENTRY(plugboard));
    if (plugboard_text == NULL) return NULL;
    const size_t len = strlen(plugboard_text);
    char *text       = malloc(len + 1);
    assertmsg(text != NULL, "malloc failed");
    text[len] = 0;

    strcpy(text, plugboard_text);
    int32_t err_code = 0;
    err_code |= to_uppercase(text);
    err_code |= remove_non_alpha(text);
    assertmsg(err_code == 0, "normalization failed");

    return text;
}

char* get_input_text_from_gui(void)
{
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = in_buffer;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gchar *input_text =
            gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, FALSE);
    const size_t len = strlen(input_text);
    char *text       = malloc(len + 1);
    assertmsg(text != NULL, "malloc failed");
    text[len] = 0;
    strcpy(text, input_text);
    int32_t err_code = 0;
    err_code |= to_uppercase(text);
    err_code |= remove_non_alpha(text);
    assertmsg(err_code == 0, "normalization failed");
    g_free(input_text);

    return text;
}

static void update_output(const char *plaintext)
{
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));

    gtk_text_buffer_set_text(buffer, plaintext, -1);
}

static void action_listener_enigma_model(GtkComboBox *combo_box)
{
    const gint i = gtk_combo_box_get_active(combo_box);
    gtk_widget_set_sensitive(rotors[3], i);
    gtk_widget_set_sensitive(positions[3], i);
    gtk_widget_set_sensitive(rings[3], i);
}

static void action_listener_refl(GtkComboBox *combo_box)
{
    GtkTreeModel *modelT;
    GtkTreeIter iter;
    gchar *selected_text;

    modelT = gtk_combo_box_get_model(combo_box);
    if (gtk_combo_box_get_active_iter(combo_box, &iter))
    {
        gtk_tree_model_get(modelT, &iter, 0, &selected_text, -1);
        g_free(selected_text);
    }
}

static void show_rotor_dialog(void)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_OK,
        "Rotors must differ!"
    );
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void action_listener_rot_comb(GtkComboBox *combo_box)
{
    // g_print("%d\n", gtk_combo_box_get_active(combo_box));
}

static void action_listener_rot_pos_ring(GtkSpinButton *button)
{
    // gint value = gtk_spin_button_get_value_as_int(button);
    // g_print("%d", value);
}

static void action_listener_input_in(GtkTextBuffer *buffer,
                                     __attribute__((unused)) GtkTextIter *location,
                                     const gchar *text, const gint len,
                                     __attribute__((unused)) gpointer user_data)
{
    for (uint16_t i = 0; i < len; i++)
    {
        if (!isalpha(text[i]) && !isspace(text[i]))
        {
            g_signal_stop_emission_by_name(buffer, "insert-text");
            return;
        }
    }
}

static void show_plugboard_dialog(void)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_OK,
        "Please enter a valid plugboard configuration!"
    );
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void action_listener_plugboard(GtkEntry *entry,
                                      const gchar *text, const gint length)
{
    const gchar *current_text = gtk_entry_get_text(entry);

    for (uint16_t i = 0; i < length; i++)
    {
        if (!isalpha(text[i]) && !isspace(text[i]))
        {
            g_signal_stop_emission_by_name(entry, "insert-text");
            return;
        }
    }

    gchar new_text[BUFFER_SIZE];

    strncpy(new_text, current_text, BUFFER_SIZE - 1);

    strncat(new_text, text, BUFFER_SIZE - 1);

    if (has_duplicates(new_text))
    {
        g_signal_stop_emission_by_name(entry, "insert-text");
    }
}

static Enigma* create_enigma_from_input(void)
{
    uint8_t *rotor_arr                 = get_rotors_from_gui();
    uint8_t *rotor_position_arr        = get_rotor_positions_from_gui();
    uint8_t *rotor_ring_position_arr   = get_rotor_ring_positions_from_gui();
    const enum ENIGMA_TYPE enigma_type = get_enigma_type_from_gui();
    const char reflector               = get_reflector_type_from_gui();
    char *plugboard                    = get_plugboard_from_gui();
    char *input_text                   = get_input_text_from_gui();

    EnigmaConfiguration configuration = {
        .rotors = rotor_arr, .rotor_positions = rotor_position_arr, .ring_settings = rotor_ring_position_arr,
        .type = enigma_type, .reflector = reflector, .message = input_text
    };
    memcpy(configuration.plugboard, plugboard, strlen(plugboard));
    Enigma *enigma = create_enigma_from_configuration(&configuration);

    free(rotor_arr);
    free(rotor_position_arr);
    free(rotor_ring_position_arr);
    free(plugboard);
    free(input_text);

    return enigma;
}


//TODO refactor
static void action_listener_start_btn(void)
{
    const gchar *plugboard_text = gtk_entry_get_text(GTK_ENTRY(plugboard));
    const size_t alpha_count = count_alphas(plugboard_text);
    assertmsg(alpha_count != SIZE_MAX, "count alphas failed");
    if (alpha_count % 2 != 0)
    {
        show_plugboard_dialog();
        return;
    }

    //More efficient than a bool array
    uint8_t rotor_mask = 0;
    for (uint16_t i = 0; i < gtk_combo_box_get_active(GTK_COMBO_BOX(model)) + 3; ++i)
    {
        const gint rot = gtk_combo_box_get_active(GTK_COMBO_BOX(rotors[i]));
        //More efficient than a bool array
        const uint8_t active_rotor = 1 << rot;
        if (rotor_mask & active_rotor)
        {
            show_rotor_dialog();
            return;
        }
        rotor_mask |= active_rotor;
    }

    enigma_to_json("");

    Enigma *enigma  = create_enigma_from_input();
    uint8_t *text   = traverse_enigma(enigma);
    char *plaintext = get_string_from_int_array(text, strlen(enigma->plaintext));
    assertmsg(plaintext != NULL, "int[] to string conversion failed");
    enigma_to_json(plaintext);
    update_output(plaintext);
    char *json_text = read_json();
    puts(json_text);

    free(json_text);
    free_enigma(enigma);
    free(text);
    free(plaintext);
}

//TODO refactor
static void activate(void)
{
    GtkBuilder *builder;

    builder = gtk_builder_new_from_file(FILE_PATH_ENIGMA);

    //    gchar *path = g_build_path(G_DIR_SEPARATOR_S, g_get_current_dir(),
    //                                     "..", "src", "gui", "icon",
    //                                     "Enigma-logo.svg", NULL);
    //    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    assertmsg(GTK_IS_WIDGET(window), "Error: window not found in the Glade file");

    model = GTK_WIDGET(gtk_builder_get_object(builder, "model_comb"));
    assertmsg(GTK_IS_WIDGET(model), "Error: model not found in the Glade file");

    reflector = GTK_WIDGET(gtk_builder_get_object(builder, "refl_comb"));
    assertmsg(GTK_IS_WIDGET(reflector), "Error: refl not found in the Glade file");

    for (uint16_t i = 0; i < 4; ++i)
    {
        char name[] = "rot1_comb";
        name[3]     = (char) (i + '1');
        rotors[i]   = GTK_WIDGET(gtk_builder_get_object(builder, name));
        assertmsg(GTK_IS_WIDGET(rotors[i]), "Error: rotor not found in the Glade file");
    }

    for (uint16_t i = 0; i < 4; ++i)
    {
        char name[]  = "rot1_pos";
        name[3]      = (char) (i + '1');
        positions[i] = GTK_WIDGET(gtk_builder_get_object(builder, name));
        assertmsg(GTK_IS_WIDGET(positions[i]), "Error: position not found in the Glade file");
    }

    for (uint16_t i = 0; i < 4; ++i)
    {
        char name[] = "rot1_ring";
        name[3]     = (char) (i + '1');
        rings[i]    = GTK_WIDGET(gtk_builder_get_object(builder, name));
        assertmsg(GTK_IS_WIDGET(rings[i]), "Error: ring not found in the Glade file");
    }

    plugboard = GTK_WIDGET(gtk_builder_get_object(builder, "plug_in"));
    assertmsg(GTK_IS_WIDGET(plugboard), "Error: plug not found in the Glade file");

    start = GTK_WIDGET(gtk_builder_get_object(builder, "start_btn"));
    assertmsg(GTK_IS_WIDGET(start), "Error: start not found in the Glade file");

    input = GTK_WIDGET(gtk_builder_get_object(builder, "input_in"));
    assertmsg(GTK_IS_WIDGET(input), "Error: input not found in the Glade file");

    output = GTK_WIDGET(gtk_builder_get_object(builder, "output_in"));
    assertmsg(GTK_IS_WIDGET(output), "Error: output not found in the Glade file");

    g_signal_connect(model, "changed", G_CALLBACK(action_listener_enigma_model),
                     NULL);

    g_signal_connect(reflector, "changed", G_CALLBACK(action_listener_refl),
                     NULL);

    for (uint16_t i = 0; i < 4; ++i)
    {
        g_signal_connect(rotors[i], "changed",
                         G_CALLBACK(action_listener_rot_comb), NULL);
    }

    for (uint16_t i = 0; i < 4; ++i)
    {
        g_signal_connect(positions[i], "value-changed",
                         G_CALLBACK(action_listener_rot_pos_ring), NULL);
    }

    for (uint16_t i = 0; i < 4; ++i)
    {
        g_signal_connect(rings[i], "value-changed",
                         G_CALLBACK(action_listener_rot_pos_ring), NULL);
    }
    in_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));
    g_signal_connect(in_buffer, "insert-text",
                     G_CALLBACK(action_listener_input_in), NULL);

    g_signal_connect(plugboard, "insert-text", G_CALLBACK(action_listener_plugboard), NULL);

    g_signal_connect(start, "clicked", G_CALLBACK(action_listener_start_btn),
                     NULL);


    //    gtk_window_set_icon(GTK_WINDOW(window), pixbuf);

    gtk_widget_show_all(window);
    gtk_combo_box_set_active(GTK_COMBO_BOX(model), 0);

    g_object_unref(builder);
}

void run_gui(void)
{
    activate();
}
