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

static GstStaticPadTemplate gst_acelpenc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/unknown")
    );

/* FIXME add/remove the formats that you want to support */
static GstStaticPadTemplate gst_acelpenc_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=[1,max],"
        "channels=[1,max],layout=interleaved")
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstAcelpenc, gst_acelpenc, GST_TYPE_AUDIO_ENCODER,
    GST_DEBUG_CATEGORY_INIT (gst_acelpenc_debug_category, "acelpenc", 0,
        "debug category for acelpenc element"));

static void
gst_acelpenc_class_init (GstAcelpencClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstAudioEncoderClass *audio_encoder_class = GST_AUDIO_ENCODER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS (klass),
      &gst_acelpenc_src_template);
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS (klass),
      &gst_acelpenc_sink_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_acelpenc_set_property;
  gobject_class->get_property = gst_acelpenc_get_property;
  gobject_class->dispose = gst_acelpenc_dispose;
  gobject_class->finalize = gst_acelpenc_finalize;
  audio_encoder_class->start = GST_DEBUG_FUNCPTR (gst_acelpenc_start);
  audio_encoder_class->stop = GST_DEBUG_FUNCPTR (gst_acelpenc_stop);
  audio_encoder_class->set_format = GST_DEBUG_FUNCPTR (gst_acelpenc_set_format);
  audio_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_acelpenc_handle_frame);
  audio_encoder_class->flush = GST_DEBUG_FUNCPTR (gst_acelpenc_flush);
  audio_encoder_class->pre_push = GST_DEBUG_FUNCPTR (gst_acelpenc_pre_push);
  audio_encoder_class->sink_event = GST_DEBUG_FUNCPTR (gst_acelpenc_sink_event);
  audio_encoder_class->src_event = GST_DEBUG_FUNCPTR (gst_acelpenc_src_event);
  audio_encoder_class->getcaps = GST_DEBUG_FUNCPTR (gst_acelpenc_getcaps);
  audio_encoder_class->open = GST_DEBUG_FUNCPTR (gst_acelpenc_open);
  audio_encoder_class->close = GST_DEBUG_FUNCPTR (gst_acelpenc_close);
  audio_encoder_class->negotiate = GST_DEBUG_FUNCPTR (gst_acelpenc_negotiate);
  audio_encoder_class->decide_allocation =
      GST_DEBUG_FUNCPTR (gst_acelpenc_decide_allocation);
  audio_encoder_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_acelpenc_propose_allocation);

}

static void
gst_acelpenc_init (GstAcelpenc *acelpenc)
{
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
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "set_format");

  return TRUE;
}

static GstFlowReturn
gst_acelpenc_handle_frame (GstAudioEncoder *encoder, GstBuffer *buffer)
{
  GstAcelpenc *acelpenc = GST_ACELPENC (encoder);

  GST_DEBUG_OBJECT (acelpenc, "handle_frame");

  return GST_FLOW_OK;
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

