/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2018 Joël Krähemann
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

#include <glib.h>
#include <glib-object.h>

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

#include <math.h>

int ags_record_midi_audio_test_init_suite();
int ags_record_midi_audio_test_clean_suite();

void ags_record_midi_audio_test_port();
 
AgsDevout *devout;
AgsAudio *audio;

extern AgsApplicationContext *ags_application_context;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_record_midi_audio_test_init_suite()
{ 
  ags_application_context = ags_audio_application_context_new();
  
  /* create soundcard */
  devout = g_object_new(AGS_TYPE_DEVOUT,
			NULL);
  g_object_ref(devout);

  /* create audio */
  audio = ags_audio_new(devout);
  g_object_ref(audio);

  ags_audio_set_flags(audio,
		      (AGS_AUDIO_OUTPUT_HAS_RECYCLING |
		       AGS_AUDIO_INPUT_HAS_RECYCLING));
  ags_audio_set_ability_flags(audio,
			      AGS_SOUND_ABILITY_PLAYBACK);

  /* create input/output */
  ags_audio_set_audio_channels(audio,
			       1, 0);

  ags_audio_set_pads(audio,
		     AGS_TYPE_OUTPUT,
		     1, 0);
  ags_audio_set_pads(audio,
		     AGS_TYPE_INPUT,
 		     1, 0);

  ags_channel_set_ability_flags(audio->output,
				AGS_SOUND_ABILITY_PLAYBACK);

  ags_channel_set_ability_flags(audio->input,
				AGS_SOUND_ABILITY_PLAYBACK);

  ags_connectable_connect(AGS_CONNECTABLE(audio));
  
  ags_connectable_connect(AGS_CONNECTABLE(audio->output));
  ags_connectable_connect(AGS_CONNECTABLE(audio->input));

  return(0);
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_record_midi_audio_test_clean_suite()
{  
  g_object_run_dispose(devout);
  g_object_unref(devout);

  g_object_run_dispose(audio);
  g_object_unref(audio);

  return(0);
}

void
ags_record_midi_audio_test_port()
{
  AgsRecordMidiAudio *record_midi_audio;
  AgsPort *port;

  record_midi_audio = ags_record_midi_audio_new(audio);

  CU_ASSERT(record_midi_audio != NULL);
  CU_ASSERT(AGS_IS_RECORD_MIDI_AUDIO(record_midi_audio));

  /* test ports */
  port = NULL;
  g_object_get(record_midi_audio,
	       "playback", &port,
	       NULL);

  CU_ASSERT(port != NULL);
  CU_ASSERT(AGS_IS_PORT(port));

  port = NULL;
  g_object_get(record_midi_audio,
	       "record", &port,
	       NULL);

  CU_ASSERT(port != NULL);
  CU_ASSERT(AGS_IS_PORT(port));

  port = NULL;
  g_object_get(record_midi_audio,
	       "filename", &port,
	       NULL);

  CU_ASSERT(port != NULL);
  CU_ASSERT(AGS_IS_PORT(port));

  port = NULL;
  g_object_get(record_midi_audio,
	       "division", &port,
	       NULL);

  CU_ASSERT(port != NULL);
  CU_ASSERT(AGS_IS_PORT(port));

  port = NULL;
  g_object_get(record_midi_audio,
	       "tempo", &port,
	       NULL);

  CU_ASSERT(port != NULL);
  CU_ASSERT(AGS_IS_PORT(port));

  port = NULL;
  g_object_get(record_midi_audio,
	       "bpm", &port,
	       NULL);

  CU_ASSERT(port != NULL);
  CU_ASSERT(AGS_IS_PORT(port));
}

int
main(int argc, char **argv)
{
  CU_pSuite pSuite = NULL;

  putenv("LC_ALL=C");
  putenv("LANG=C");
  
  /* initialize the CUnit test registry */
  if(CUE_SUCCESS != CU_initialize_registry()){
    return CU_get_error();
  }

  /* add a suite to the registry */
  pSuite = CU_add_suite("AgsRecordMidiAudioTest", ags_record_midi_audio_test_init_suite, ags_record_midi_audio_test_clean_suite);
  
  if(pSuite == NULL){
    CU_cleanup_registry();
    
    return CU_get_error();
  }

  /* add the tests to the suite */
  if((CU_add_test(pSuite, "test of AgsRecordMidiAudio port", ags_record_midi_audio_test_port) == NULL)){
    CU_cleanup_registry();
    
    return CU_get_error();
  }
  
  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  
  CU_cleanup_registry();
  
  return(CU_get_error());
}
