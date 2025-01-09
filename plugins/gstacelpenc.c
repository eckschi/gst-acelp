/* GStreamer
 * Copyright (C) 2025 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstacelpenc
 *
 * The acelpenc element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! acelpenc ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/audio/gstaudioencoder.h>
#include "gstacelpenc.h"

#include "c_source.h"
#include "c_structs.h"

#define L_frame     240
#define serial_size 138
#define ana_size     23

GST_DEBUG_CATEGORY_STATIC (gst_acelpenc_debug_category);
#define GST_CAT_DEFAULT gst_acelpenc_debug_category

/* prototypes */


static void gst_acelpenc_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_acelpenc_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_acelpenc_dispose (GObject * object);
static void gst_acelpenc_finalize (GObject * object);

static gboolean gst_acelpenc_start (GstAudioEncoder * encoder);
static gboolean gst_acelpenc_stop (GstAudioEncoder * encoder);
static gboolean gst_acelpenc_set_format (GstAudioEncoder * encoder,
    GstAudioInfo * info);
static GstFlowReturn gst_acelpenc_handle_frame (GstAudioEncoder * encoder,
    GstBuffer * buffer);
static void gst_acelpenc_flush (GstAudioEncoder * encoder);
static GstFlowReturn gst_acelpenc_pre_push (GstAudioEncoder * encoder,
    GstBuffer ** buffer);
static gboolean gst_acelpenc_sink_event (GstAudioEncoder * encoder,
    GstEvent * event);
static gboolean gst_acelpenc_src_event (GstAudioEncoder * encoder,
    GstEvent * event);
static GstCaps *gst_acelpenc_getcaps (GstAudioEncoder * encoder,
    GstCaps * filter);
static gboolean gst_acelpenc_open (GstAudioEncoder * encoder);
static gboolean gst_acelpenc_close (GstAudioEncoder * encoder);
static gboolean gst_acelpenc_negotiate (GstAudioEncoder * encoder);
static gboolean gst_acelpenc_decide_allocation (GstAudioEncoder * encoder,
    GstQuery * query);
static gboolean gst_acelpenc_propose_allocation (GstAudioEncoder * encoder,
    GstQuery * query);

enum
{
  PROP_0
};

/* pad templates */

// the output format
static GstStaticPadTemplate gst_acelpenc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-tetra-acelp, "
        "rate = 8000, " 
        "channels = 1")

    );


/* the input formats that we support */
static GstStaticPadTemplate gst_acelpenc_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-raw, "
                    "format = (string) " GST_AUDIO_NE (S16) ", "
                    "layout = (string) interleaved, "
                    "rate = (int) 8000, channels = (int) 1")

    );


/* class initialization */
G_DEFINE_TYPE_WITH_CODE (GstAcelpenc, gst_acelpenc, GST_TYPE_AUDIO_ENCODER,
    GST_DEBUG_CATEGORY_INIT (gst_acelpenc_debug_category, "acelpenc", 0,
        "debug category for acelpenc element"));

static void
gst_acelpenc_class_init (GstAcelpencClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstAudioEncoderClass *audio_encoder_class = GST_AUDIO_ENCODER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (element_class, &gst_acelpenc_src_template);
  gst_element_class_add_static_pad_template (element_class, &gst_acelpenc_sink_template);

  gst_element_class_set_static_metadata (element_class,
      "TETRA ACELP encoder", "Codec/Encoder/Audio", "Encode TETRA ACELP audio",
      "Manfred Eckschlager<me@eckschi.net>");

  gobject_class->set_property = gst_acelpenc_set_property;
  gobject_class->get_property = gst_acelpenc_get_property;
  //gobject_class->dispose = gst_acelpenc_dispose;
  //gobject_class->finalize = gst_acelpenc_finalize;
  audio_encoder_class->start = GST_DEBUG_FUNCPTR (gst_acelpenc_start);
  audio_encoder_class->stop = GST_DEBUG_FUNCPTR (gst_acelpenc_stop);
  audio_encoder_class->set_format = GST_DEBUG_FUNCPTR (gst_acelpenc_set_format);
  audio_encoder_class->handle_frame = GST_DEBUG_FUNCPTR (gst_acelpenc_handle_frame);
  //audio_encoder_class->flush = GST_DEBUG_FUNCPTR (gst_acelpenc_flush);
  //audio_encoder_class->pre_push = GST_DEBUG_FUNCPTR (gst_acelpenc_pre_push);
  //audio_encoder_class->sink_event = GST_DEBUG_FUNCPTR (gst_acelpenc_sink_event);
  //audio_encoder_class->src_event = GST_DEBUG_FUNCPTR (gst_acelpenc_src_event);
  //audio_encoder_class->getcaps = GST_DEBUG_FUNCPTR (gst_acelpenc_getcaps);
  //audio_encoder_class->open = GST_DEBUG_FUNCPTR (gst_acelpenc_open);
  //audio_encoder_class->close = GST_DEBUG_FUNCPTR (gst_acelpenc_close);
  //audio_encoder_class->negotiate = GST_DEBUG_FUNCPTR (gst_acelpenc_negotiate);
  //audio_encoder_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_acelpenc_decide_allocation);
  //audio_encoder_class->propose_allocation = GST_DEBUG_FUNCPTR (gst_acelpenc_propose_allocation);

}

static void
gst_acelpenc_init (GstAcelpenc *acelpenc)
{
  GST_PAD_SET_ACCEPT_TEMPLATE (GST_AUDIO_ENCODER_SINK_PAD (acelpenc));

  /* Set defaults. */
  acelpenc->samples_per_block = L_frame;
}

void
gst_acelpenc_set_property (GObject *object, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (object);

  GST_DEBUG_OBJECT (acelpenc, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_acelpenc_get_property (GObject *object, guint property_id,
    GValue *value, GParamSpec *pspec)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (object);

  GST_DEBUG_OBJECT (acelpenc, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_acelpenc_dispose (GObject *object)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (object);

  GST_DEBUG_OBJECT (acelpenc, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_acelpenc_parent_class)->dispose (object);
}

void
gst_acelpenc_finalize (GObject *object)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (object);

  GST_DEBUG_OBJECT (acelpenc, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_acelpenc_parent_class)->finalize (object);
}

static gboolean
gst_acelpenc_start (GstAudioEncoder *encoder)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "start");

  Init_Pre_Process(&acelpenc->coderData);
  Init_Coder_Tetra(&acelpenc->coderData);

  return TRUE;
}

static gboolean
gst_acelpenc_stop (GstAudioEncoder *encoder)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "stop");

  return TRUE;
}

static gboolean
gst_acelpenc_set_format (GstAudioEncoder *encoder, GstAudioInfo *info)
{
  GstCaps *caps;
  gboolean ret;
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "set_format");

  // acelpenc->rate = GST_AUDIO_INFO_RATE (info);
  // acelpenc->channels = GST_AUDIO_INFO_CHANNELS (info);

  caps = gst_caps_new_simple ("audio/x-tetra-acelp",
      "rate", G_TYPE_INT, 8000,
      "channels", G_TYPE_INT, 1,
      "layout", G_TYPE_STRING, "interleaved",
      NULL);

  ret = gst_audio_encoder_set_output_format (GST_AUDIO_ENCODER (encoder), caps);
  gst_caps_unref (caps);
  
  acelpenc->samples_per_block = L_frame;

  /* report needs to base class */
  gst_audio_encoder_set_frame_samples_min (encoder, acelpenc->samples_per_block);
  gst_audio_encoder_set_frame_samples_max (encoder, acelpenc->samples_per_block);
  gst_audio_encoder_set_frame_max (encoder, 1);
  
  return ret;
}

static GstBuffer *
acelpenc_encode_block (GstAcelpenc * enc, const gint16 * samples, int blocksize)
{
  GstBuffer *outbuf = NULL;
  GstMapInfo omap;
  Word16 syn[L_frame];			// Local synthesis.
  Word16 ana[ana_size];			/* Analysis parameters.   */

  outbuf = gst_buffer_new_and_alloc (blocksize);
  gst_buffer_map (outbuf, &omap, GST_MAP_WRITE);

  memcpy(enc->coderData.new_speech, samples, L_frame);        // copy input audio into codec

  Pre_Process(enc->coderData.new_speech, (Word16)L_frame, &enc->coderData);	    // Pre processing of input speech 

  Coder_Tetra(ana, syn, &enc->coderData);       // Find speech parameters         

  Post_Process(syn, (Word16)L_frame, &enc->coderData);           // Post processing of synthesis   

  Prm2bits_Tetra_8(ana, omap.data, &enc->coderData);                  // Parameters to serial bits      

  gst_buffer_unmap (outbuf, &omap);

  return outbuf;
}

static GstFlowReturn
gst_acelpenc_handle_frame(GstAudioEncoder *encoder, GstBuffer *buffer)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);
  GstFlowReturn ret = GST_FLOW_OK;
  gint16 *samples;
  GstBuffer *outbuf;
  int input_bytes_per_block;
  const int BYTES_PER_SAMPLE = 2;
  GstMapInfo map;

  GST_DEBUG_OBJECT (acelpenc, "handle_frame");

  /* we don't deal with squeezing remnants, so simply discard those */
  if (G_UNLIKELY (buffer == NULL)) 
  {
    GST_DEBUG_OBJECT (encoder, "no data");
    goto done;
  }

  input_bytes_per_block =
       acelpenc->samples_per_block * BYTES_PER_SAMPLE; // should be 480

  gst_buffer_map (buffer, &map, GST_MAP_READ);
  if (G_UNLIKELY (map.size < input_bytes_per_block)) 
  {
    GST_DEBUG_OBJECT (acelpenc, "discarding trailing data %d", (gint) map.size);
    gst_buffer_unmap (buffer, &map);
    ret = gst_audio_encoder_finish_frame (encoder, NULL, -1);
    goto done;
  }

  samples = (gint16 *) map.data;
  GST_DEBUG_OBJECT (acelpenc, "zipfi");
  // do tha shizzle
  outbuf = acelpenc_encode_block (acelpenc, samples, 137);//acelpenc->blocksize);
    
  gst_buffer_unmap (buffer, &map);

  GST_DEBUG_OBJECT (acelpenc, "foo");
  ret = gst_audio_encoder_finish_frame (encoder, outbuf, acelpenc->samples_per_block);
  GST_DEBUG_OBJECT (acelpenc, "foodle %d", ret);

done:
  return ret;
}

static void
gst_acelpenc_flush (GstAudioEncoder *encoder)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "flush");

}

static GstFlowReturn
gst_acelpenc_pre_push (GstAudioEncoder *encoder, GstBuffer **buffer)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "pre_push");

  return GST_FLOW_OK;
}

static gboolean
gst_acelpenc_sink_event (GstAudioEncoder *encoder, GstEvent *event)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "sink_event");

  return TRUE;
}

static gboolean
gst_acelpenc_src_event (GstAudioEncoder *encoder, GstEvent *event)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "src_event");

  return TRUE;
}

static GstCaps *
gst_acelpenc_getcaps (GstAudioEncoder *encoder, GstCaps *filter)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "getcaps");

  return NULL;
}

static gboolean
gst_acelpenc_open (GstAudioEncoder *encoder)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "open");

  return TRUE;
}

static gboolean
gst_acelpenc_close (GstAudioEncoder *encoder)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "close");

  return TRUE;
}

static gboolean
gst_acelpenc_negotiate (GstAudioEncoder *encoder)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "negotiate");

  return TRUE;
}

static gboolean
gst_acelpenc_decide_allocation (GstAudioEncoder *encoder, GstQuery *query)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "decide_allocation");

  return TRUE;
}

static gboolean
gst_acelpenc_propose_allocation (GstAudioEncoder *encoder, GstQuery *query)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "propose_allocation");

  return TRUE;
}

static gboolean
plugin_init (GstPlugin *plugin)
{
  gst_element_register (plugin, "acelpenc", GST_RANK_PRIMARY, GST_TYPE_ACELPENC);

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, acelpenc, "TETRA ACELP Encoder plugin", plugin_init, VERSION, "LGPL",
    "GStreamer THOMSON-CSF", "https://github.com/eckschi/gst-acelp")
