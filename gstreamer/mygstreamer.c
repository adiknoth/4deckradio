#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>


#include <gtk/gtk.h>
#include <gst/gst.h>

#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif

#include "mygstreamer.h"
#include "audio.h"

#define NUM_PLAYERS 4


static void print_usage(char *name) {
    fprintf (stderr, "usage: %s [options]\n"
            "\n"
            "\t\t-f\tfullscreen\n"
            "\t\t-a\tautconnect\n"
            "\t\t-h\thelp (this message)\n"
            "\n", name
            );

}

static void get_tag(GstTagList *tags, const gchar *tag, gchar **target) {
        if (TRUE != gst_tag_list_get_string (tags, tag, target)) {
            *target = g_strdup_printf ("Unknown");
        }
}

static void _update_timelabel(CustomData *data, const gchar *str, const gchar *fmt) {
    char *markup;
    gchar *format = g_strdup_printf("<span size=\"x-large\" %s>%%s</span>", fmt);
    markup = g_markup_printf_escaped (format, str);
    gtk_label_set_markup (GTK_LABEL (data->timelabel), markup);
    g_free (markup);
    g_free (format);
}

static void update_timelabel_bg(CustomData *data, const gchar *str, const gchar *color) {
    gchar *format = g_strdup_printf("bgcolor=\"%s\"", color);
    _update_timelabel(data, str, format);
    g_free (format);
}

static void update_timelabel(CustomData *data, const gchar *str) {
    _update_timelabel(data, str, "");
}

/* currently unused, but might come in handy later */
#if 0
static void unset_background(CustomData *data) {
    gtk_label_set_attributes(GTK_LABEL (data->timelabel), NULL);
}

static void set_background(CustomData *data) {
    PangoAttrList *attrs;

    attrs = pango_attr_list_new();
    pango_attr_list_insert (attrs, pango_attr_background_new (65535, 0, 0));
    gtk_label_set_attributes(GTK_LABEL (data->timelabel), attrs);
    pango_attr_list_unref (attrs);
}
#endif

static inline void update_taglabel(CustomData *data, const gchar *str) {
    gtk_label_set_text (GTK_LABEL (data->taglabel), str);
}


static void playpause_cb(GtkButton *button, CustomData *data) {
    if (data->state == GST_STATE_PLAYING) {
        gst_element_set_state (data->pipeline, GST_STATE_PAUSED);
    } else {
        gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
    }
}

static void file_selection_cb (GtkFileChooser *chooser, CustomData *data) {
    gchar *fileURI = gtk_file_chooser_get_uri (chooser);
    gchar *fileName = gtk_file_chooser_get_filename (chooser);

    if (NULL == fileURI) {
        return;
    }

    if (g_file_test(fileName, G_FILE_TEST_IS_DIR)) {
        return;
    }

    audio_stop_player (data);
    g_print ("File URI: %s\n", fileURI);
    update_taglabel(data, g_filename_display_basename(fileName));

    audio_set_uri (data, fileURI);
    data->duration = GST_CLOCK_TIME_NONE;

    /* load new file by putting the player into pause state */
    audio_pause_player (data);
}

/* This function is called when the STOP button is clicked */
static void stop_cb (GtkButton *button, CustomData *data) {
    update_timelabel(data, "Stopped");
    audio_stop_player (data);
}

static void quit_all (CustomData *data) {
    GtkWidget *dialog;

    gboolean all_in_readystate = TRUE;

    for (int i = 0; i < NUM_PLAYERS; i++) {
            all_in_readystate = all_in_readystate && (GST_STATE_READY == data[i].state);
    }

    if (all_in_readystate) {
            dialog = gtk_message_dialog_new (GTK_WINDOW (data[0].mainwindow),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_QUESTION,
                            GTK_BUTTONS_YES_NO,
                            "Really quit?");
    } else {
            dialog = gtk_message_dialog_new (GTK_WINDOW (data[0].mainwindow),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_INFO,
                            GTK_BUTTONS_OK,
                            "Won't quit while playing. Stop all players first.");
    }
    gint result = gtk_dialog_run (GTK_DIALOG (dialog));
    switch (result) {
    case GTK_RESPONSE_YES:
            gtk_main_quit();
            break;
    default:
            break;
    }
    gtk_widget_destroy (dialog);
}

/* This function is called when the main window is closed */
static void delete_event_cb (GtkWidget *widget, GdkEvent *event, void *argp) {
    gtk_main_quit ();
}

/* This function is called when the slider changes its position. We perform a seek to the
 * new position here. */
static void slider_cb (GtkRange *range, CustomData *data) {
    gchar *time;

    gdouble value = gtk_range_get_value (GTK_RANGE (data->slider));
    time = g_strdup_printf("%" HMS_TIME_FORMAT, HMS_TIME_ARGS((gint64)(value * GST_SECOND)));
    update_timelabel(data, time);
    g_free (time);

    gst_element_seek_simple (data->pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
            (gint64)(value * GST_SECOND));
}

/* Creates a button with an icon but no text */
static GtkWidget* _create_media_button (const gchar *stockid) {
        GtkWidget *button = gtk_button_new();

        gtk_button_set_image (GTK_BUTTON (button),
                        gtk_image_new_from_stock (stockid,
                                GTK_ICON_SIZE_BUTTON));

        return button;
}

/* This creates all the GTK+ widgets that compose our application, and registers the callbacks */
static GtkWidget* create_player_ui (CustomData *data) {
    GtkWidget *stop_button; /* Buttons */
    GtkWidget *myGrid;


    myGrid = gtk_grid_new();

    data->playPauseButton = _create_media_button (GTK_STOCK_MEDIA_PAUSE);
    g_signal_connect (G_OBJECT (data->playPauseButton), "clicked", G_CALLBACK (playpause_cb), data);

    stop_button = _create_media_button (GTK_STOCK_MEDIA_STOP);
    g_signal_connect (G_OBJECT (stop_button), "clicked", G_CALLBACK (stop_cb), data);

    data->slider = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_scale_set_draw_value (GTK_SCALE (data->slider), 0);
    data->slider_update_signal_id = g_signal_connect (G_OBJECT (data->slider), "value-changed", G_CALLBACK (slider_cb), data);
    g_signal_connect (G_OBJECT (data->slider), "move-slider", G_CALLBACK (slider_cb), data);

    data->timelabel = gtk_label_new ("");
    update_timelabel(data, "Time remaining");
    data->taglabel = gtk_label_new ("Selected filename");
    {
        data->filechooser = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_OPEN);
        gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (data->filechooser), TRUE);
        gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (data->filechooser), FALSE);
        gtk_file_chooser_set_use_preview_label (GTK_FILE_CHOOSER (data->filechooser), FALSE);
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (data->filechooser), getenv("HOME"));

        GtkFileFilter *filter = gtk_file_filter_new();

        gtk_file_filter_add_mime_type (filter, "audio/*");
        gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (data->filechooser), filter);

        data->file_selection_signal_id = 
            g_signal_connect (G_OBJECT (data->filechooser), "selection-changed", G_CALLBACK (file_selection_cb), data);

        /* block signal to prevent false selection on startup*/
        g_signal_handler_block (data->filechooser, data->file_selection_signal_id);
    }


    /* arrange all elements into myGrid */
    gtk_grid_attach (GTK_GRID (myGrid), data->filechooser, 0, 0, 1, 3);
    gtk_grid_attach_next_to (GTK_GRID (myGrid), data->playPauseButton, data->filechooser, GTK_POS_RIGHT, 1, 1);
    gtk_grid_attach_next_to (GTK_GRID (myGrid), stop_button, data->playPauseButton, GTK_POS_BOTTOM, 1, 1);

    gtk_grid_attach_next_to (GTK_GRID (myGrid), data->taglabel, data->filechooser, GTK_POS_BOTTOM, 2, 1);
    gtk_grid_attach_next_to (GTK_GRID (myGrid), data->timelabel, data->taglabel, GTK_POS_BOTTOM, 2, 1);

    gtk_grid_attach_next_to (GTK_GRID (myGrid), data->slider, data->timelabel, GTK_POS_BOTTOM, 2, 1);


    /* allow at least one expanding child, so all uppper widgets will resize
     * when maximising the window
     */
    gtk_widget_set_hexpand (data->filechooser, TRUE);
    gtk_widget_set_vexpand (data->filechooser, TRUE);

    gtk_label_set_line_wrap(GTK_LABEL (data->taglabel), TRUE);

    return myGrid;
}



/* This function is called periodically to refresh the GUI */
static gboolean refresh_ui (CustomData *data) {
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 current = -1;

    /* We do not want to update anything unless we are in the PAUSED or PLAYING states */
    if (data->state < GST_STATE_PAUSED)
        return TRUE;

    /* If we didn't know it yet, query the stream duration */
    if (!GST_CLOCK_TIME_IS_VALID (data->duration)) {
#if GST_VERSION_MAJOR == (0)
        if (!gst_element_query_duration (data->pipeline, &fmt, &data->duration)) {
#else
        if (!gst_element_query_duration (data->pipeline, fmt, &data->duration)) {
#endif
            g_printerr ("Could not query current duration.\n");
        } else {
            /* Set the range of the slider to the clip duration, in SECONDS */
            gtk_range_set_range (GTK_RANGE (data->slider), 0, (gdouble)data->duration / GST_SECOND);
        }
    }

#if GST_VERSION_MAJOR == (0)
    if (gst_element_query_position (data->pipeline, &fmt, &current)) {
#else
    if (gst_element_query_position (data->pipeline, fmt, &current)) {
#endif
        /* Block the "value-changed" signal, so the slider_cb function is not called
         * (which would trigger a seek the user has not requested) */
        g_signal_handler_block (data->slider, data->slider_update_signal_id);
        /* Set the position of the slider to the current pipeline positoin, in SECONDS */
        gtk_range_set_value (GTK_RANGE (data->slider), (gdouble)current / GST_SECOND);
        /* Re-enable the signal */
        g_signal_handler_unblock (data->slider, data->slider_update_signal_id);
        {
            gchar *time;
            GstClockTimeDiff remaining = data->duration-current;

            time = g_strdup_printf("%" HMS_TIME_FORMAT " / -%" HMS_TIME_FORMAT " / %" HMS_TIME_FORMAT,
                    HMS_TIME_ARGS(current),
                    HMS_TIME_ARGS(remaining),
                    HMS_TIME_ARGS(data->duration));

            if (remaining < 15 * GST_SECOND) {
                if (remaining < 5 * GST_SECOND) {
                    update_timelabel_bg (data, time, "red");
                } else {
                    update_timelabel_bg (data, time, "yellow");
                }
            } else {
                update_timelabel_bg (data, time, "green");
            }

            g_free (time);
        }
    }
    return TRUE;
}

/* This function is called when new metadata is discovered in the stream */
static void tags_cb (GstElement *pipeline, gint stream, CustomData *data) {
    /* We are possibly in a GStreamer working thread, so we notify the main
     * thread of this event through a message in the bus */
    gst_element_post_message (pipeline,
            gst_message_new_application (GST_OBJECT (pipeline),
                gst_structure_new ("tags-changed", NULL)));
}

/* This function is called when an error message is posted on the bus */
static void error_cb (GstBus *bus, GstMessage *msg, CustomData *data) {
    GError *err;
    gchar *debug_info;

    /* Print error details on the screen */
    gst_message_parse_error (msg, &err, &debug_info);
    g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
    g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error (&err);
    g_free (debug_info);

    /* Set the pipeline to READY (which stops playback) */
    audio_stop_player (data);
}

/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
static void eos_cb (GstBus *bus, GstMessage *msg, CustomData *data) {
    g_print ("End-Of-Stream reached.\n");
    update_timelabel(data, "Finished");
    audio_stop_player (data);
}

static void _set_playPauseImage (CustomData *data, const gchar *stockid) {
        gtk_image_set_from_stock(
                        GTK_IMAGE (gtk_button_get_image (GTK_BUTTON (data->playPauseButton))),
                        stockid, GTK_ICON_SIZE_BUTTON);
}

/* This function is called when the pipeline changes states. We use it to
 * keep track of the current state. */
static void state_changed_cb (GstBus *bus, GstMessage *msg, CustomData *data) {
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
    if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->pipeline)) {
        data->state = new_state;
        g_print ("State set to %s\n", gst_element_state_get_name (new_state));

        /* update playPauseImage */
        if (data->state == GST_STATE_PAUSED || data->state == GST_STATE_READY) {
                _set_playPauseImage(data, GTK_STOCK_MEDIA_PLAY);
        } else {
                _set_playPauseImage(data, GTK_STOCK_MEDIA_PAUSE);
        }

        if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED) {
            /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
            refresh_ui (data);
        }
    }
}

/* This function is called when a "tag" message is posted on the bus. */
static void tag_cb (GstBus *bus, GstMessage *msg, CustomData *data) {
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_TAG) {
        GstTagList *tags = NULL;
        gchar *title, *artist, *tagstring;

        gst_message_parse_tag (msg, &tags);

        get_tag (tags, GST_TAG_TITLE, &title);
        get_tag (tags, GST_TAG_ARTIST, &artist);

        g_print ("Title: %s Artist %s\n", title, artist);
        tagstring = g_strdup_printf("%s - %s", title, artist);

        /* only update when we got a meaningful tag */
        if (!(0 == g_strcmp0 (title, "Unknown") &&
                0 == g_strcmp0 (artist, "Unknown"))) {
            update_taglabel(data, tagstring);
        }

        g_free(title);
        g_free(artist);
        g_free(tagstring);

#if GST_VERSION_MAJOR == (0)
        gst_tag_list_free (tags);
#else
        gst_tag_list_unref (tags);
#endif
    }
}

static GtkWidget* create_mainwindow(void) {
    GtkWidget *main_window;

    main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (main_window), "delete-event", G_CALLBACK (delete_event_cb), NULL);

    gtk_window_set_title (GTK_WINDOW (main_window), "The player's player");

    return main_window;
}




/* Initialise a single instance */
static GtkWidget* init_player(CustomData *data, guint decknumber, int autoconnect) {
    GtkWidget *playerUI;
    GstBus *bus;


    init_audio(data, decknumber, autoconnect);

    /* Create the GUI */
    playerUI = create_player_ui (data);

    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    bus = gst_element_get_bus (data->pipeline);
    gst_bus_add_signal_watch (bus);
    g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, data);
    g_signal_connect (G_OBJECT (bus), "message::eos", (GCallback)eos_cb, data);
    g_signal_connect (G_OBJECT (bus), "message::state-changed", (GCallback)state_changed_cb, data);
    g_signal_connect (G_OBJECT (bus), "message::tag", (GCallback)tag_cb, data);
    gst_object_unref (bus);

    /* Register a function that GLib will call every second */
    g_timeout_add_seconds (1, (GSourceFunc)refresh_ui, data);

    return playerUI;
}

static gboolean socket_handler (GIOChannel *source, GIOCondition cond, CustomData *data) {
#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

    int joyfd;

    struct js_event {
        uint32_t time;    /* event timestamp in milliseconds */
        int16_t value;    /* value */
        uint8_t type;     /* event type */
        uint8_t number;   /* axis/button number */
    } e;

    joyfd = g_io_channel_unix_get_fd(source);

    read(joyfd, &e, sizeof(struct js_event));

    if ((e.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON) {
        if (e.number < NUM_PLAYERS) {
            if (e.value) {
                audio_play_player (&data[e.number]);
            } else {
                audio_pause_player (&data[e.number]);
            }
        }
        g_print ("joystick button %d = %d\n", e.number, e.value);
    }

    return TRUE;
}

static void keyboard_handler(CustomData *data) {
    g_print ("Keyboard interaction, calling handler\n");
    playpause_cb(NULL, data);
}

static void _add_hotkey (const gchar *hotkey, GtkAccelGroup *accelgroup,
                GCallback callback, gpointer user_data) {
        GClosure *keycallback;
        guint accelkey;
        GdkModifierType accelmod;

        gtk_accelerator_parse (hotkey, &accelkey, &accelmod);

        keycallback = g_cclosure_new_swap (callback, user_data, NULL);

        gtk_accel_group_connect(accelgroup, accelkey, accelmod, 0, keycallback);
        g_closure_unref(keycallback);
}


static void create_hotkeys(GtkWidget *main_window, CustomData *data) {
    GtkAccelGroup *accelgroup;

    accelgroup = gtk_accel_group_new();

    for (int i = 0; i < NUM_PLAYERS; i++) {
        gchar *hotkey = g_strdup_printf("F%u", 9 + i);

        _add_hotkey (hotkey, accelgroup, G_CALLBACK (keyboard_handler), &data[i]);
        g_free (hotkey);
    }

    /* Bind the quit_all callback to ctrl-q */
    _add_hotkey ("<Control>q", accelgroup, G_CALLBACK (quit_all), data);


    gtk_window_add_accel_group (GTK_WINDOW (main_window), accelgroup);
}

static GIOChannel* create_joystick(CustomData *data) {
    GIOChannel *io_channel;
    int joyfd = open ("/dev/input/js0", O_RDONLY);

    if (-1 == joyfd) {
        perror ("Couldn't open /dev/input/js0");
        return NULL;
    }

    io_channel = g_io_channel_unix_new (joyfd);

    g_io_channel_set_encoding (io_channel, NULL, NULL);

    g_io_add_watch (io_channel, G_IO_IN | G_IO_PRI, (GIOFunc)socket_handler, data);

    return io_channel;
}

static gchar *dummyuri(void) {
        return g_strdup_printf ("file:///");
}

static gchar* make_silence(void) {
        const gchar silentwave[] = {0x52, 0x49, 0x46, 0x46, 0x24, 0x00, 0x00, 0x00, 0x57,
                0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x02, 0x00, 0x80, 0xBB, 0x00, 0x00, 0x00, 0xEE, 0x02,
                0x00, 0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0x00, 0x00,
                0x00, 0x00};
        GError *error;
        gchar *tmpfilename;
        gchar *ret;
        GIOChannel *outfile;
        gsize bytes_written;

        error = NULL;
        gint fd = g_file_open_tmp (NULL, &tmpfilename, &error);

        if (-1 == fd) {
                g_print ("Unable to create tmp file: %s\n", error->message);
                g_error_free (error);
                return dummyuri();
        }

        outfile = g_io_channel_unix_new (fd);

        /* make it a binary channel */
        if (G_IO_STATUS_NORMAL != g_io_channel_set_encoding (outfile, NULL, NULL)) {
                g_print ("Unable to create tmpfile in binary mode\n");
                return dummyuri();
        }

        g_print ("File is %d\n", sizeof(*silentwave));

        error = NULL;
        if (G_IO_STATUS_NORMAL != g_io_channel_write_chars (outfile, &silentwave[0], sizeof(silentwave), &bytes_written, &error)) {
                g_print ("Unable to write to tmpfile: %s\n", error->message);
                g_error_free (error);
                return dummyuri();
        }

        g_io_channel_shutdown (outfile, TRUE, NULL);
        g_io_channel_unref (outfile);

        close(fd);

        return g_filename_to_uri (tmpfilename, NULL, NULL);
}


int main(int argc, char *argv[]) {
    CustomData data[NUM_PLAYERS];
    GtkWidget *main_window;
    GtkWidget *main_grid;
    GIOChannel *io_joystick;

    extern char *optarg;
    extern int optind, optopt;
    gboolean show_usage = FALSE;
    gboolean fullscreen = FALSE;
    int autoconnect = 0;
    int c;

    while ((c = getopt(argc, argv, "hfa")) != -1) {
        switch(c) {
            case 'h':
                show_usage = TRUE;
                break;
            case 'f':
                fullscreen = TRUE;
                break;
            case 'a':
                autoconnect = 1;
                break;
            case '?':
                fprintf(stderr,
                        "Unrecognized option: -%c\n", optopt);
                show_usage = TRUE;
                break;
        }
    }

    if (TRUE == show_usage) {
        print_usage(argv[0]);
        exit(2);
    }



    /* Initialize GTK */
    gtk_init (&argc, &argv);

    /* Initialize GStreamer */
    gst_init (&argc, &argv);

    /* Initialize our data structure */
    memset (&data, 0, sizeof (data));

    main_window = create_mainwindow();

    main_grid = gtk_grid_new();

    for (int i=0; i < NUM_PLAYERS; i++) {
        GtkWidget *playerUI = init_player (&data[i], i, autoconnect);
        data[i].mainwindow = main_window;

        /* two columns (i%2) in (i/2) rows */
        gtk_grid_attach (GTK_GRID (main_grid), playerUI, i % 2, i / 2, 1, 1);
    }

    /* force main_grid to be homogeneous, so all player UIs remain stable
     * and don't resize unintentionally (e.g., when selecting a long
     * filename in the filechooser)
     */
    gtk_grid_set_row_homogeneous(GTK_GRID (main_grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID (main_grid), TRUE);

    gtk_container_add (GTK_CONTAINER (main_window), main_grid);

    if (TRUE == fullscreen) {
        gtk_window_fullscreen (GTK_WINDOW (main_window));
    } else {
        gtk_window_maximize (GTK_WINDOW (main_window));
    }

    gtk_widget_show_all (main_window);

    {
            /* ugly hack to expose the jack ports until jackaudiosink is fixed */
            gchar *tmpfileuri = make_silence();

            /* enable file selection signals */
            for (int i=0; i < NUM_PLAYERS; i++) {
                    g_signal_handler_unblock (data[i].filechooser,
                                    data[i].file_selection_signal_id);
                    audio_set_uri(&data[i], tmpfileuri);
                    audio_pause_player(&data[i]);
            }

            g_free (tmpfileuri);
    }


    create_hotkeys(main_window, data);

    io_joystick = create_joystick(data);

    /* Start the GTK main loop. We will not regain control until gtk_main_quit is called. */
    gtk_main ();

    if (NULL != io_joystick) {
        g_io_channel_shutdown (io_joystick, FALSE, NULL);
        g_io_channel_unref (io_joystick);
    }

    /* Free resources */
    for (int i=0; i < NUM_PLAYERS; i++) {
        gst_element_set_state (data[i].pipeline, GST_STATE_NULL);
        gst_object_unref (data[i].pipeline);
    }
    return 0;
}
