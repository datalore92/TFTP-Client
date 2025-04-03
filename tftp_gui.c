#include "tftp_gui.h"
#include "tftp_client.h"
#include <gtk/gtk.h>
#include <string.h>

static GtkWidget *window;
static GtkWidget *server_entry;
static GtkWidget *file_label;
static GtkWidget *status_label;
static char selected_file[1024] = "";

// Add CSS styling
static void apply_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    
    const gchar *css =
        "window { padding: 15px; }"
        "entry { min-width: 250px; }"
        "button { margin: 5px; padding: 5px 10px; }"
        "label { margin-top: 5px; margin-bottom: 5px; }"
        ".filename { font-size: 10pt; font-style: italic; color: #555555; }"
        ".status { font-weight: bold; }"
        ".success { color: #008800; }"
        ".error { color: #CC0000; }";
    
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void update_status(const gchar *message, gboolean is_error) {
    gtk_label_set_text(GTK_LABEL(status_label), message);
    
    // Remove existing style classes
    GtkStyleContext *context = gtk_widget_get_style_context(status_label);
    gtk_style_context_remove_class(context, "success");
    gtk_style_context_remove_class(context, "error");
    
    // Add appropriate style class
    if (is_error) {
        gtk_style_context_add_class(context, "error");
    } else {
        gtk_style_context_add_class(context, "success");
    }
}

static void send_file(GtkWidget *widget, gpointer data) {
    const char *server = gtk_entry_get_text(GTK_ENTRY(server_entry));
    
    if (strlen(selected_file) == 0) {
        update_status("Please select a file first!", TRUE);
        return;
    }
    
    if (strlen(server) == 0) {
        update_status("Please enter a server address!", TRUE);
        return;
    }
    
    update_status("Sending file...", FALSE);
    
    // Update UI while processing
    while (gtk_events_pending())
        gtk_main_iteration();
    
    if (tftp_send_file(server, 69, selected_file) == 0) {
        update_status("File sent successfully!", FALSE);
    } else {
        update_status("Error sending file!", TRUE);
    }
}

static void choose_file(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choose File",
        GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Open", GTK_RESPONSE_ACCEPT,
        NULL);
    
    // Add file filters
    GtkFileFilter *all_files = gtk_file_filter_new();
    gtk_file_filter_set_name(all_files, "All Files");
    gtk_file_filter_add_pattern(all_files, "*");
    
    GtkFileFilter *images = gtk_file_filter_new();
    gtk_file_filter_set_name(images, "Images");
    gtk_file_filter_add_mime_type(images, "image/*");
    
    GtkFileFilter *docs = gtk_file_filter_new();
    gtk_file_filter_set_name(docs, "Documents");
    gtk_file_filter_add_mime_type(docs, "application/pdf");
    gtk_file_filter_add_mime_type(docs, "text/plain");
    gtk_file_filter_add_mime_type(docs, "application/msword");
    
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_files);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), images);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), docs);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        strncpy(selected_file, filename, sizeof(selected_file) - 1);
        
        // Show just the filename, not the full path in the UI
        const char *base_filename = filename;
        const char *last_slash = strrchr(filename, '\\');
        if (last_slash) {
            base_filename = last_slash + 1;
        } else {
            last_slash = strrchr(filename, '/');
            if (last_slash) {
                base_filename = last_slash + 1;
            }
        }
        
        gtk_label_set_text(GTK_LABEL(file_label), base_filename);
        g_free(filename);
        update_status("File selected", FALSE);
    }

    gtk_widget_destroy(dialog);
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Apply CSS styling
    apply_css();
    
    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "TFTP File Transfer Client");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 250);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
    // Main container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    // Header label
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='large' weight='bold'>TFTP File Upload Client</span>");
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 5);
    
    // Server address section
    GtkWidget *server_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *server_label = gtk_label_new("Server Address:");
    server_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(server_entry), "Enter server IP address");
    
    gtk_box_pack_start(GTK_BOX(server_box), server_label, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(server_box), server_entry, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), server_box, FALSE, FALSE, 5);
    
    // File selection section
    GtkWidget *file_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *browse_button = gtk_button_new_with_label("Select File");
    gtk_box_pack_start(GTK_BOX(file_box), browse_button, FALSE, FALSE, 5);
    
    // File label with style
    GtkWidget *file_frame = gtk_frame_new("Selected File");
    file_label = gtk_label_new("No file selected");
    gtk_label_set_xalign(GTK_LABEL(file_label), 0.0);
    gtk_label_set_ellipsize(GTK_LABEL(file_label), PANGO_ELLIPSIZE_MIDDLE);
    gtk_style_context_add_class(gtk_widget_get_style_context(file_label), "filename");
    gtk_container_add(GTK_CONTAINER(file_frame), file_label);
    
    gtk_box_pack_start(GTK_BOX(main_box), file_box, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(main_box), file_frame, FALSE, TRUE, 5);
    
    // Send button (centered)
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *send_button = gtk_button_new_with_label("Send File");
    gtk_style_context_add_class(gtk_widget_get_style_context(send_button), "suggested-action");
    GtkWidget *send_icon = gtk_image_new_from_icon_name("document-send", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(send_button), send_icon);
    gtk_button_set_always_show_image(GTK_BUTTON(send_button), TRUE);
    
    gtk_box_pack_start(GTK_BOX(button_box), gtk_label_new(""), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), send_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), gtk_label_new(""), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 10);
    
    // Status section
    GtkWidget *status_frame = gtk_frame_new("Status");
    status_label = gtk_label_new("Ready");
    gtk_style_context_add_class(gtk_widget_get_style_context(status_label), "status");
    gtk_container_add(GTK_CONTAINER(status_frame), status_label);
    gtk_box_pack_start(GTK_BOX(main_box), status_frame, FALSE, FALSE, 5);
    
    // Add a separator before the footer
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), separator, FALSE, FALSE, 5);
    
    // Footer with version info
    GtkWidget *footer = gtk_label_new("TFTP Client v1.0");
    gtk_label_set_xalign(GTK_LABEL(footer), 1.0);
    gtk_box_pack_start(GTK_BOX(main_box), footer, FALSE, FALSE, 5);
    
    // Connect signals
    g_signal_connect(browse_button, "clicked", G_CALLBACK(choose_file), NULL);
    g_signal_connect(send_button, "clicked", G_CALLBACK(send_file), NULL);
    
    gtk_widget_show_all(window);
}

int run_tftp_gui(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.gtk.tftp", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}