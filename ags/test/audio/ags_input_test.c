/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2017 Joël Krähemann
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
#include <CUnit/Basic.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

int ags_input_test_init_suite();
int ags_input_test_clean_suite();

void ags_input_test_open_file();
void ags_input_test_is_active();
void ags_input_test_next_active();

#define AGS_INPUT_TEST_OPEN_FILE_FILENAME "/usr/share/sounds/alsa/Noise.wav"
#define AGS_INPUT_TEST_OPEN_FILE_AUDIO_CHANNEL (0)

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_input_test_init_suite()
{
  return(0);
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_input_test_clean_suite()
{
  return(0);
}

void
ags_input_test_open_file()
{
  AgsInput *input;

  gboolean success;
  
  input = g_object_new(AGS_TYPE_INPUT,
		       NULL);
  AGS_CHANNEL(input)->first_recycling =
    AGS_CHANNEL(input)->last_recycling = g_object_new(AGS_TYPE_RECYCLING,
						      NULL);
  
  success = ags_input_open_file(input,
				AGS_INPUT_TEST_OPEN_FILE_FILENAME,
				NULL,
				NULL,
				NULL,
				AGS_INPUT_TEST_OPEN_FILE_AUDIO_CHANNEL);
  
  CU_ASSERT(success == TRUE &&
	    input->file_link != NULL &&
	    AGS_CHANNEL(input)->first_recycling->audio_signal != NULL);
}

void
ags_input_test_is_active()
{
  AgsInput *input;
  AgsAudioSignal *audio_signal;
  AgsRecallID *recall_id;
  AgsRecyclingContext *recycling_context, *parent_recycling_context;

  gboolean is_active;
  
  input = g_object_new(AGS_TYPE_INPUT,
		       NULL);
  AGS_CHANNEL(input)->first_recycling =
    AGS_CHANNEL(input)->last_recycling = g_object_new(AGS_TYPE_RECYCLING,
						      NULL);

  recycling_context = g_object_new(AGS_TYPE_RECYCLING_CONTEXT,
				   NULL);
  
  parent_recycling_context = g_object_new(AGS_TYPE_RECYCLING_CONTEXT,
					  "parent", recycling_context,
					  NULL);
  
  /* assert - not active */
  is_active = ags_input_is_active(input,
				  recycling_context);

  CU_ASSERT(is_active == FALSE);
  
  /* assert - active */
  recall_id = g_object_new(AGS_TYPE_RECALL_ID,
			   "recycling-context", recycling_context,
			   NULL);
  
  audio_signal = g_object_new(AGS_TYPE_AUDIO_SIGNAL,
			      "recall-id", recall_id,
			      NULL);
  ags_recycling_add_audio_signal(AGS_CHANNEL(input)->first_recycling,
				 audio_signal);
  
  is_active = ags_input_is_active(input,
				  recycling_context);

  CU_ASSERT(is_active == TRUE);
}

void
ags_input_test_next_active()
{
  AgsInput *input[3], *current;
  AgsAudioSignal *audio_signal;
  AgsRecallID *recall_id;
  AgsRecyclingContext *recycling_context, *parent_recycling_context;

  input[0] = g_object_new(AGS_TYPE_INPUT,
			  NULL);
  AGS_CHANNEL(input[0])->first_recycling =
    AGS_CHANNEL(input[0])->last_recycling = g_object_new(AGS_TYPE_RECYCLING,
							 NULL);

  input[1] = g_object_new(AGS_TYPE_INPUT,
			  NULL);
  AGS_CHANNEL(input[1])->first_recycling =
    AGS_CHANNEL(input[1])->last_recycling = g_object_new(AGS_TYPE_RECYCLING,
							 NULL);
  AGS_CHANNEL(input[1])->prev = input[0];
  AGS_CHANNEL(input[1])->prev_pad = input[0];
  AGS_CHANNEL(input[0])->next = input[1];
  AGS_CHANNEL(input[0])->next_pad = input[1];
  
  input[2] = g_object_new(AGS_TYPE_INPUT,
			  NULL);
  AGS_CHANNEL(input[2])->first_recycling =
    AGS_CHANNEL(input[2])->last_recycling = g_object_new(AGS_TYPE_RECYCLING,
							 NULL);
  AGS_CHANNEL(input[2])->prev = input[1];
  AGS_CHANNEL(input[2])->prev_pad = input[1];
  AGS_CHANNEL(input[1])->next = input[2];
  AGS_CHANNEL(input[1])->next_pad = input[2];

  recycling_context = g_object_new(AGS_TYPE_RECYCLING_CONTEXT,
				   NULL);
  
  parent_recycling_context = g_object_new(AGS_TYPE_RECYCLING_CONTEXT,
					  "parent", recycling_context,
					  NULL);

  /* assert - no active */
  current = ags_input_next_active(input[0],
				  recycling_context);

  CU_ASSERT(current == NULL);
  
  /* assert - input[2] */
  recall_id = g_object_new(AGS_TYPE_RECALL_ID,
			   "recycling-context", recycling_context,
			   NULL);
  
  audio_signal = g_object_new(AGS_TYPE_AUDIO_SIGNAL,
			      "recall-id", recall_id,
			      NULL);
  ags_recycling_add_audio_signal(AGS_CHANNEL(input[2])->first_recycling,
				 audio_signal);
  
  current = ags_input_next_active(input[0],
				  recycling_context);

  CU_ASSERT(current == input[2]);

  /* assert - input[1] */
  recall_id = g_object_new(AGS_TYPE_RECALL_ID,
			   "recycling-context", recycling_context,
			   NULL);
  
  audio_signal = g_object_new(AGS_TYPE_AUDIO_SIGNAL,
			      "recall-id", recall_id,
			      NULL);
  ags_recycling_add_audio_signal(AGS_CHANNEL(input[1])->first_recycling,
				 audio_signal);
  
  current = ags_input_next_active(input[0],
				  recycling_context);

  CU_ASSERT(current == input[1]);

  /* assert - input[0] */
  recall_id = g_object_new(AGS_TYPE_RECALL_ID,
			   "recycling-context", recycling_context,
			   NULL);
  
  audio_signal = g_object_new(AGS_TYPE_AUDIO_SIGNAL,
			      "recall-id", recall_id,
			      NULL);
  ags_recycling_add_audio_signal(AGS_CHANNEL(input[0])->first_recycling,
				 audio_signal);
  
  current = ags_input_next_active(input[0],
				  recycling_context);

  CU_ASSERT(current == input[0]);
}

int
main(int argc, char **argv)
{
  CU_pSuite pSuite = NULL;

  /* initialize the CUnit test registry */
  if(CUE_SUCCESS != CU_initialize_registry()){
    return CU_get_error();
  }

  /* add a suite to the registry */
  pSuite = CU_add_suite("AgsInputTest", ags_input_test_init_suite, ags_input_test_clean_suite);
  
  if(pSuite == NULL){
    CU_cleanup_registry();
    
    return CU_get_error();
  }

#if 0
  g_log_set_fatal_mask("GLib-GObject",
  		       G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL);

  g_log_set_fatal_mask("Gtk",
  		       G_LOG_LEVEL_CRITICAL);

  g_log_set_fatal_mask(NULL,
  		       G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL);
#endif
  
  /* add the tests to the suite */
  if((CU_add_test(pSuite, "test of AgsInput open file", ags_input_test_open_file) == NULL) ||
     (CU_add_test(pSuite, "test of AgsInput is active", ags_input_test_is_active) == NULL) ||
     (CU_add_test(pSuite, "test of AgsInput next active", ags_input_test_next_active) == NULL)){
    CU_cleanup_registry();
    
    return CU_get_error();
  }
  
  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  
  CU_cleanup_registry();
  
  return(CU_get_error());
}

