/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2020 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AGS_FX_NOTATION_AUDIO_PROCESSOR_H__
#define __AGS_FX_NOTATION_AUDIO_PROCESSOR_H__

#include <glib.h>
#include <glib-object.h>

#include <ags/audio/ags_audio.h>
#include <ags/audio/ags_recall_audio.h>
#include <ags/audio/ags_note.h>

G_BEGIN_DECLS

#define AGS_TYPE_FX_NOTATION_AUDIO_PROCESSOR                (ags_fx_notation_audio_processor_get_type())
#define AGS_FX_NOTATION_AUDIO_PROCESSOR(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), AGS_TYPE_FX_NOTATION_AUDIO_PROCESSOR, AgsFxNotationAudioProcessor))
#define AGS_FX_NOTATION_AUDIO_PROCESSOR_CLASS(class)        (G_TYPE_CHECK_CLASS_CAST((class), AGS_TYPE_FX_NOTATION_AUDIO_PROCESSOR, AgsFxNotationAudioProcessor))
#define AGS_IS_FX_NOTATION_AUDIO_PROCESSOR(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AGS_TYPE_FX_NOTATION_AUDIO_PROCESSOR))
#define AGS_IS_FX_NOTATION_AUDIO_PROCESSOR_CLASS(class)     (G_TYPE_CHECK_CLASS_TYPE ((class), AGS_TYPE_FX_NOTATION_AUDIO_PROCESSOR))
#define AGS_FX_NOTATION_AUDIO_PROCESSOR_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), AGS_TYPE_FX_NOTATION_AUDIO_PROCESSOR, AgsFxNotationAudioProcessorClass))

#define AGS_FX_NOTATION_AUDIO_PROCESSOR_MAX_CHANNELS (128)

typedef struct _AgsFxNotationAudioProcessor AgsFxNotationAudioProcessor;
typedef struct _AgsFxNotationAudioProcessorClass AgsFxNotationAudioProcessorClass;

struct _AgsFxNotationAudioProcessor
{
  AgsRecallAudioRun recall_audio_run;

  AgsAudioSignal* audio_signal[AGS_FX_NOTATION_AUDIO_PROCESSOR_MAX_CHANNELS];
};

struct _AgsFxNotationAudioProcessorClass
{
  AgsRecallAudioRunClass recall_audio_run;

  void (*key_on)(AgsFxNotationAudioProcessor *fx_notation_audio_processor,
		 AgsNote *note,
		 guint velocity);
  void (*key_off)(AgsFxNotationAudioProcessor *fx_notation_audio_processor,
		  AgsNote *note,
		  guint velocity);
  void (*key_pressure)(AgsFxNotationAudioProcessor *fx_notation_audio_processor,
		       AgsNote *note,
		       guint velocity);
  
  void (*play)(AgsFxNotationAudioProcessor *fx_notation_audio_processor);
  void (*record)(AgsFxNotationAudioProcessor *fx_notation_audio_processor);
  void (*feed)(AgsFxNotationAudioProcessor *fx_notation_audio_processor);
};

GType ags_fx_notation_audio_processor_get_type();

void ags_fx_notation_audio_processor_key_on(AgsFxNotationAudioProcessor *fx_notation_audio_processor,
					    AgsNote *note,
					    guint velocity);
void ags_fx_notation_audio_processor_key_off(AgsFxNotationAudioProcessor *fx_notation_audio_processor,
					     AgsNote *note,
					     guint velocity);
void ags_fx_notation_audio_processor_key_pressure(AgsFxNotationAudioProcessor *fx_notation_audio_processor,
						  AgsNote *note,
						  guint velocity);

void ags_fx_notation_audio_processor_play(AgsFxNotationAudioProcessor *fx_notation_audio_processor);
void ags_fx_notation_audio_processor_record(AgsFxNotationAudioProcessor *fx_notation_audio_processor);
void ags_fx_notation_audio_processor_feed(AgsFxNotationAudioProcessor *fx_notation_audio_processor);

AgsFxNotationAudioProcessor* ags_fx_notation_audio_processor_new(AgsAudio *audio);

G_END_DECLS

#endif /*__AGS_FX_NOTATION_AUDIO_PROCESSOR_H__*/
