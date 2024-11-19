/*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#include <gtk/gtk.h>
#include <ctype.h>

#include "enigma_gui.h"
#include "json/json.h"

//
// Created by Emanuel on 25.07.2024.
//

#define BUFFER_SIZE 1024

// TODO wrap in struct
static GtkWidget *window, *model, *reflector, *plugboard, *start, *input, *output;
static GtkWidget *rotors[4], *positions[4], *rings[4];
static GtkTextBuffer *in_buffer;

enum ENIGMA_TYPE get_enigma_type_from_gui(void)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(model)) == 0 ? ENIGMA_M3 : ENIGMA_M4;
}

enum ROTOR_TYPE *get_rotors_from_gui(void)
{
    const uint8_t num_rotors = get_enigma_type_from_gui();
    enum ROTOR_TYPE *rotor_arr = malloc(sizeof(enum ROTOR_TYPE) * num_rotors);
    assertmsg(rotor_arr != NULL, "rotors == NULL");
    for (uint8_t i = 0; i < 3; ++i)
    {
        rotor_arr[i] = gtk_combo_box_get_active(GTK_COMBO_BOX(rotors[i])) + 1;
    }
    if (num_rotors == 4)
    {
        rotor_arr[3] = gtk_combo_box_get_active(GTK_COMBO_BOX(rotors[3])) == 0 ? ROTOR_BETA : ROTOR_GAMMA;
    }

    return rotor_arr;
}

enum REFLECTOR_TYPE get_reflector_type_from_gui(void)
{
    const enum ENIGMA_TYPE enigma_type = get_enigma_type_from_gui();
    if(enigma_type== ENIGMA_M3)
    {
        switch (gtk_combo_box_get_active(GTK_COMBO_BOX(reflector)))
        {
            case 0:
                return UKW_A;
            case 1:
                return UKW_B;
            case 2:
                return UKW_C;
            default:
                fprintf(stderr, "Unknown reflector type");
                exit(1);
        }
    }
    else
    {
        switch (gtk_combo_box_get_active(GTK_COMBO_BOX(reflector)))
        {
            case 0:
                return UKW_B_THIN;
            case 1:
                return UKW_C_THIN;
            default:
                fprintf(stderr, "Unknown reflector type");
                exit(1);
        }
    }
}

uint8_t *get_rotor_positions_from_gui(void)
{
    const uint8_t num_rotors = get_enigma_type_from_gui();
    uint8_t *rotor_position_arr = malloc(sizeof(uint8_t) * num_rotors);
    assertmsg(rotor_position_arr != NULL, "rotors == NULL");
    for (uint8_t i = 0; i < num_rotors; ++i)
    {
        rotor_position_arr[i] =
                gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(positions[i])) - 1;
    }

    return rotor_position_arr;
}

uint8_t *get_rotor_ring_positions_from_gui(void)
{
    const uint8_t num_rotors = get_enigma_type_from_gui();
    uint8_t *rotor_position_arr = malloc(sizeof(uint8_t) * num_rotors);
    assertmsg(rotor_position_arr != NULL, "rotors == NULL");
    for (uint8_t i = 0; i < num_rotors; ++i)
    {
        rotor_position_arr[i] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rings[i])) - 1;
    }

    return rotor_position_arr;
}

char *get_plugboard_from_gui(void)
{
    const gchar *plugboard_text = gtk_entry_get_text(GTK_ENTRY(plugboard));
    if (plugboard_text == NULL) return NULL;

    char *text = strdup(plugboard_text);
    assertmsg(text != NULL, "strdup failed");

    // Error checking has been omitted,
    // since the only two errors ERR_EMPTY_STRING and ERR_NULL_POINTER are "valid inputs"
    remove_non_ascii(text);
    to_uppercase(text);
    remove_non_alpha(text);

    return text;
}

char *get_input_text_from_gui(void)
{
    GtkTextIter start_iter, end_iter;
    GtkTextBuffer *buffer = in_buffer;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);

    gchar *input_text =
            gtk_text_buffer_get_text(buffer, &start_iter, &end_iter, FALSE);
    if (input_text == NULL || *input_text == 0) return NULL;

    char *text = strdup(input_text);
    assertmsg(text != NULL, "strdup failed");

    //TODO replace umlauts with paraphrases
    remove_non_ascii(text);


    g_free(input_text);

    return text;
}

static char *format_output(const char *output_text)
{
    const size_t len = strlen(output_text);
    const size_t num_spaces_needed = (len - 1) / 5; // Typically blocks of five

    char *new_output = malloc(len + num_spaces_needed + 1);
    size_t new_output_index = 0;

    for (size_t pos = 0; pos < len; ++pos)
    {
        new_output[new_output_index++] = output_text[pos];
        if ((pos + 1) % 5 == 0 && (pos + 1) < len)
            new_output[new_output_index++] = ' ';

    }
    new_output[new_output_index] = 0;

    return new_output;
}

static void update_output(const char *output_text)
{

    char *formatted_output = format_output(output_text);

    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));

    gtk_text_buffer_set_text(buffer, formatted_output, -1);
    free(formatted_output);
}

static void action_listener_enigma_model(GtkComboBox *combo_box)
{
    const gint i = gtk_combo_box_get_active(combo_box);

    gtk_widget_set_sensitive(rotors[3], i);
    gtk_widget_set_sensitive(positions[3], i);
    gtk_widget_set_sensitive(rings[3], i);

    GtkListStore *list_store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(reflector)));
    gtk_list_store_clear(list_store);

    GtkTreeIter iter;

    if (i == 0)
    {
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, "UKW A", -1);

        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, "UKW B", -1);

        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, "UKW C", -1);

        gtk_combo_box_set_active(GTK_COMBO_BOX(reflector), 1);
    } else
    {
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, "UKW B Thin", -1);

        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, 0, "UKW C Thin", -1);

        gtk_combo_box_set_active(GTK_COMBO_BOX(reflector), 0);

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

static void action_listener_rot_comb(void)
{
    // g_print("%d\n", gtk_combo_box_get_active(combo_box));
}

static void action_listener_rot_pos_ring(void)
{
    // gint value = gtk_spin_button_get_value_as_int(button);
    // g_print("%d", value);
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

    for (uint16_t i = 0; i < (uint16_t) length; i++)
    {
        if (!isalpha(text[i]) && !isspace(text[i]))
        {
            g_signal_stop_emission_by_name(entry, "insert-text");
            return;
        }
    }

    gchar new_text[BUFFER_SIZE];

    strncpy(new_text, current_text, BUFFER_SIZE - 1);

    strncat(new_text, text, BUFFER_SIZE - g_utf8_strlen(new_text, -1) - 1);

    if (has_duplicates(new_text))
    {
        g_signal_stop_emission_by_name(entry, "insert-text");
    }
}

static char* create_output_with_enigma(char *restrict input_text)
{
    enum ROTOR_TYPE *rotor_arr               = get_rotors_from_gui();
    const enum ENIGMA_TYPE enigma_type       = get_enigma_type_from_gui();
    const enum REFLECTOR_TYPE reflector_type = get_reflector_type_from_gui();
    uint8_t *rotor_position_arr              = get_rotor_positions_from_gui();
    uint8_t *rotor_ring_position_arr         = get_rotor_ring_positions_from_gui();
    char *plugboard_text                     = get_plugboard_from_gui();
    EnigmaConfiguration configuration = {
            .rotors = rotor_arr, .rotor_positions = rotor_position_arr, .ring_settings = rotor_ring_position_arr,
            .type = enigma_type, .reflector = reflector_type, .message = input_text
    };
    memcpy(configuration.plugboard, plugboard_text, strlen(plugboard_text));

    Enigma *enigma = create_enigma_from_configuration(&configuration);
    uint8_t *enigma_output = traverse_enigma(enigma);
    char *plaintext = get_string_from_int_array(enigma_output, strlen(input_text));
    assertmsg(plaintext != NULL, "int[] to string conversion failed");

    enigma_config_to_json(&configuration, plaintext);

    free(rotor_arr);
    free(rotor_position_arr);
    free(rotor_ring_position_arr);
    free(plugboard_text);
    free(enigma_output);

    return plaintext;
}

static void generate_output_with_enigma(char *restrict input_text)
{
    char *plaintext = create_output_with_enigma(input_text);

    update_output(plaintext);

    free(plaintext);
}

static void show_input_text_dialog(void)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "Please enter a input text!"
    );
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void action_listener_start_btn(void)
{
    char *input_text = get_input_text_from_gui();
    if(input_text == NULL)
    {
        show_input_text_dialog();
        return;
    }
    const size_t len = strlen(input_text);
    const ssize_t space_count = count_c(input_text, ' ');
    to_uppercase(input_text);
    remove_non_alpha(input_text);
    if (len != strlen(input_text) + space_count)
    {
        fprintf(stderr, "Warning: Only use alphabetic chars and spaces!\n");
        fprintf(stderr, "The invalid chars have been truncated\n");
    }

    const gchar *plugboard_text = gtk_entry_get_text(GTK_ENTRY(plugboard));
    const ssize_t alpha_count_plugboard = count_alphas(plugboard_text);
//    assertmsg(alpha_count_plugboard >= 0, "count alphas failed");
    if (alpha_count_plugboard % 2 != 0)
    {
        free(input_text);
        show_plugboard_dialog();
        return;
    }

    //More efficient than a bool array - probably doesn't matter here, but I leave it in
    // BETA and GAMMA reflector must be unique!
    uint8_t rotor_mask = 0;
    for (uint16_t i = 0; i < 3; ++i)
    {
        const gint rot = gtk_combo_box_get_active(GTK_COMBO_BOX(rotors[i]));
        const uint8_t active_rotor = 1 << rot;
        if (rotor_mask & active_rotor)
        {
            free(input_text);
            show_rotor_dialog();
            return;
        }
        rotor_mask |= active_rotor;
    }
    generate_output_with_enigma(input_text);
    free(input_text);
}

//TODO refactor
static void activate(void)
{
    GtkBuilder *builder;

    builder = gtk_builder_new_from_file(FILE_PATH_ENIGMA);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    assertmsg(GTK_IS_WIDGET(window), "Error: window not found in the Glade file");

    model = GTK_WIDGET(gtk_builder_get_object(builder, "model_comb"));
    assertmsg(GTK_IS_WIDGET(model), "Error: model not found in the Glade file");

    reflector = GTK_WIDGET(gtk_builder_get_object(builder, "refl_comb"));
    assertmsg(GTK_IS_WIDGET(reflector), "Error: refl not found in the Glade file");

    for (uint16_t i = 0; i < 4; ++i)
    {
        char name[] = "rot1_comb";
        name[3] = (char) (i + '1');
        rotors[i] = GTK_WIDGET(gtk_builder_get_object(builder, name));
        assertmsg(GTK_IS_WIDGET(rotors[i]), "Error: rotor not found in the Glade file");
    }

    for (uint16_t i = 0; i < 4; ++i)
    {
        char name[] = "rot1_pos";
        name[3] = (char) (i + '1');
        positions[i] = GTK_WIDGET(gtk_builder_get_object(builder, name));
        assertmsg(GTK_IS_WIDGET(positions[i]), "Error: position not found in the Glade file");
    }

    for (uint16_t i = 0; i < 4; ++i)
    {
        char name[] = "rot1_ring";
        name[3] = (char) (i + '1');
        rings[i] = GTK_WIDGET(gtk_builder_get_object(builder, name));
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
    // g_signal_connect(in_buffer, "insert-text",
    // G_CALLBACK(action_listener_input_in), NULL);

    g_signal_connect(plugboard, "insert-text", G_CALLBACK(action_listener_plugboard), NULL);

    g_signal_connect(start, "clicked", G_CALLBACK(action_listener_start_btn),
                     NULL);

    gtk_widget_show_all(window);
    gtk_combo_box_set_active(GTK_COMBO_BOX(model), 0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(reflector), 1);

    g_object_unref(builder);
}

void run_gui(void)
{
    activate();
}
