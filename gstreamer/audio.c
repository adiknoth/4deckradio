#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <unistd.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include "mygstreamer.h"
#include "audio.h"

static inline GstStateChangeReturn _change_state (CustomData *data, GstState state) {
    return gst_element_set_state (data->pipeline, state);
}

GstStateChangeReturn audio_stop_player (CustomData *data) {
    return _change_state (data, GST_STATE_READY);
}

GstStateChangeReturn audio_pause_player (CustomData *data) {
    return _change_state (data, GST_STATE_PAUSED);
}

GstStateChangeReturn audio_play_player (CustomData *data) {
    return _change_state (data, GST_STATE_PLAYING);
}

static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
    GstPad *sink_pad = gst_element_get_static_pad (data->audioconvert, "sink");
    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;

    g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

    /* If our converter is already linked, we have nothing to do here */
    if (gst_pad_is_linked (sink_pad)) {
        g_print ("  We are already linked. Ignoring.\n");
        goto exit;
    }

    /* Check the new pad's type */
#if GST_VERSION_MAJOR == (0)
    new_pad_caps = gst_pad_get_caps (new_pad);
#else
    new_pad_caps = gst_pad_query_caps (new_pad, NULL);
#endif
    new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
    new_pad_type = gst_structure_get_name (new_pad_struct);
    if (!g_str_has_prefix (new_pad_type, "audio/x-raw")) {
        g_print ("  It has type '%s' which is not raw audio. Ignoring.\n", new_pad_type);
        goto exit;
    }

    /* Attempt the link */
    ret = gst_pad_link (new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED (ret)) {
        g_print ("  Type is '%s' but link failed.\n", new_pad_type);
    } else {
        g_print ("  Link succeeded (type '%s').\n", new_pad_type);
    }

exit:
    /* Unreference the new pad's caps, if we got them */
    if (new_pad_caps != NULL)
        gst_caps_unref (new_pad_caps);

    /* Unreference the sink pad */
    gst_object_unref (sink_pad);
}

static GstElement* create_gst_element (const gchar *what, const gchar *name) {
    GstElement *target = NULL;

    target = gst_element_factory_make (what, NULL);
    if (!target) {
        g_printerr ("Couldn't create %s/%s\n", what, name);
    }

    return target;
}

void audio_set_uri(CustomData *data, const gchar *uri) {
    g_object_set (data->uridecodebin, "uri", uri, NULL);
    data->duration = GST_CLOCK_TIME_NONE;
}

gboolean audio_seek(CustomData *data, gdouble value) {
    return gst_element_seek_simple (data->pipeline,
            GST_FORMAT_TIME,
            GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_SKIP,
            (gint64)(value * GST_SECOND));
}

inline gboolean audio_is_playing(CustomData *data) {
    return (data->state == GST_STATE_PLAYING);
}

void audio_pseudo_stop(CustomData *data) {
    audio_seek (data, 0.0);
    audio_pause_player (data);
}

int init_audio(CustomData *data, guint decknumber, int autoconnect) {
    GstStateChangeReturn ret;

    data->duration = GST_CLOCK_TIME_NONE;

    /* Create the elements */
    data->pipeline = gst_pipeline_new("test");

    data->uridecodebin = create_gst_element ("uridecodebin", "uri_decodebin");
    data->audioconvert = create_gst_element ("audioconvert", "audio_convert");
    data->audioresample = create_gst_element ("audioresample", "audio_resample");
    data->jackaudiosink = create_gst_element ("jackaudiosink", "jack_audiosink");

    if (!data->pipeline || !data->uridecodebin || !data->audioresample ||
            !data->jackaudiosink) {
        g_printerr ("Not all elements could be created.\n");
        return 1;
    }


    gst_bin_add_many (GST_BIN (data->pipeline), data->uridecodebin,
            data->audioconvert, data->audioresample, data->jackaudiosink, NULL);

    {
        /* settings that control interaction with jackd */
        gchar *name;
        name = g_strdup_printf("player-%u", decknumber);
        g_object_set (data->jackaudiosink, "client-name", name, NULL);
        g_free (name);

        /*
connect : Specify how the output ports will be connected
Enum "GstJackConnect" Default: 1, "auto"
(0): none             - Don't automatically connect ports to physical ports
(1): auto             - Automatically connect ports to physical ports
(2): auto-forced      - Automatically connect ports to as many physical ports as possible
*/
        g_object_set (data->jackaudiosink, "connect", autoconnect, NULL);
    }

    if (TRUE != gst_element_link_many (data->audioconvert,
                data->audioresample, data->jackaudiosink, NULL)) {
        g_printerr ("Problems linking bins\n");
        exit (1);
    }

    /* Connect to the pad-added signal */
    g_signal_connect (data->uridecodebin, "pad-added", G_CALLBACK (pad_added_handler), data);


    return 0;
}
