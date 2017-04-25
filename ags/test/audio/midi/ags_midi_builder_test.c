/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2017 Joël Krähemann
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

#include <libags.h>
#include <libags-audio.h>

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>

int ags_midi_builder_test_init_suite();
int ags_midi_builder_test_clean_suite();

void ags_midi_builder_test_append_header();
void ags_midi_builder_test_append_track();
void ags_midi_builder_test_append_key_on();
void ags_midi_builder_test_append_key_off();
void ags_midi_builder_test_append_key_pressure();
void ags_midi_builder_test_append_change_parameter();
void ags_midi_builder_test_append_change_pitch_bend();
void ags_midi_builder_test_append_change_program();
void ags_midi_builder_test_append_change_pressure();
void ags_midi_builder_test_append_sysex();
void ags_midi_builder_test_append_quarter_frame();
void ags_midi_builder_test_append_song_position();
void ags_midi_builder_test_append_song_select();
void ags_midi_builder_test_append_tune_request();
void ags_midi_builder_test_append_sequence_number();
void ags_midi_builder_test_append_smtpe();
void ags_midi_builder_test_append_time_signature();
void ags_midi_builder_test_append_key_signature();
void ags_midi_builder_test_append_sequencer_meta_event();
void ags_midi_builder_test_append_text_event();

#define AGS_MIDI_BUILDER_TEST_APPEND_HEADER_FORMAT (1)
#define AGS_MIDI_BUILDER_TEST_APPEND_HEADER_TRACK_COUNT (0)
#define AGS_MIDI_BUILDER_TEST_APPEND_HEADER_DIVISION (0xe250)
#define AGS_MIDI_BUILDER_TEST_APPEND_HEADER_TIMES (30)
#define AGS_MIDI_BUILDER_TEST_APPEND_HEADER_BPM (120)
#define AGS_MIDI_BUILDER_TEST_APPEND_HEADER_CLICKS (4)

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_midi_builder_test_init_suite()
{
  return(0);
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_midi_builder_test_clean_suite()
{
  return(0);
}

void
ags_midi_builder_test_append_header()
{
  AgsMidiBuilder *midi_builder;

  glong offset, format;
  glong track_count, division;
    
  midi_builder = ags_midi_builder_new(NULL);

  ags_midi_builder_append_header(midi_builder,
				 6, AGS_MIDI_BUILDER_TEST_APPEND_HEADER_FORMAT,
				 AGS_MIDI_BUILDER_TEST_APPEND_HEADER_TRACK_COUNT, AGS_MIDI_BUILDER_TEST_APPEND_HEADER_DIVISION,
				 AGS_MIDI_BUILDER_TEST_APPEND_HEADER_TIMES, AGS_MIDI_BUILDER_TEST_APPEND_HEADER_BPM,
				 AGS_MIDI_BUILDER_TEST_APPEND_HEADER_CLICKS);

  CU_ASSERT(midi_builder->midi_header != NULL);

  CU_ASSERT(midi_builder->midi_header->offset == 6);
  CU_ASSERT(midi_builder->midi_header->format == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_FORMAT);
  CU_ASSERT(midi_builder->midi_header->count == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_TRACK_COUNT);
  CU_ASSERT(midi_builder->midi_header->division == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_DIVISION);
  CU_ASSERT(midi_builder->midi_header->times == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_TIMES);
  CU_ASSERT(midi_builder->midi_header->beat == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_BPM);
  CU_ASSERT(midi_builder->midi_header->clicks == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_CLICKS);

  ags_midi_builder_build(midi_builder);

  CU_ASSERT(midi_builder->data != NULL);
  CU_ASSERT(midi_builder->length == 14);

  ags_midi_buffer_util_get_header(midi_builder->data,
				  &offset, &format,
				  &track_count, &division);

  
  CU_ASSERT(offset == 6);
  CU_ASSERT(format == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_FORMAT);
  CU_ASSERT(track_count == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_TRACK_COUNT);
  CU_ASSERT(division == AGS_MIDI_BUILDER_TEST_APPEND_HEADER_DIVISION);
}

void
ags_midi_builder_test_append_track()
{
  AgsMidiBuilder *midi_builder;

  midi_builder = ags_midi_builder_new(NULL);

  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_key_on()
{
  AgsMidiBuilder *midi_builder;

  midi_builder = ags_midi_builder_new(NULL);

  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_key_off()
{
  AgsMidiBuilder *midi_builder;

  midi_builder = ags_midi_builder_new(NULL);

  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_key_pressure()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_change_parameter()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_change_pitch_bend()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_change_program()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_change_pressure()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_sysex()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_quarter_frame()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_song_position()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_song_select()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_tune_request()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_sequence_number()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_smtpe()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_time_signature()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_key_signature()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_sequencer_meta_event()
{
  //TODO:JK: implement me
}

void
ags_midi_builder_test_append_text_event()
{
  //TODO:JK: implement me
}

int
main(int argc, char **argv)
{
  CU_pSuite pSuite = NULL;

  putenv("LC_ALL=C\0");
  putenv("LANG=C\0");
  
  /* initialize the CUnit test registry */
  if(CUE_SUCCESS != CU_initialize_registry()){
    return CU_get_error();
  }

  /* add a suite to the registry */
  pSuite = CU_add_suite("AgsMidiBuilderTest\0", ags_midi_builder_test_init_suite, ags_midi_builder_test_clean_suite);
  
  if(pSuite == NULL){
    CU_cleanup_registry();
    
    return CU_get_error();
  }

  /* add the tests to the suite */
  if((CU_add_test(pSuite, "test of AgsMidiBuilder to append header\0", ags_midi_builder_test_append_header) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append track\0", ags_midi_builder_test_append_track) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append key on\0", ags_midi_builder_test_append_key_on) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append key off\0", ags_midi_builder_test_append_key_off) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append key pressure\0", ags_midi_builder_test_append_key_pressure) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append change parameter\0", ags_midi_builder_test_append_change_parameter) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append change pitch bend\0", ags_midi_builder_test_append_change_pitch_bend) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append change program\0", ags_midi_builder_test_append_change_program) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append change pressure\0", ags_midi_builder_test_append_change_pressure) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append sysex\0", ags_midi_builder_test_append_sysex) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append quarter frame\0", ags_midi_builder_test_append_quarter_frame) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append song position\0", ags_midi_builder_test_append_song_position) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append song select\0", ags_midi_builder_test_append_song_select) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append tune request\0", ags_midi_builder_test_append_tune_request) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append sequence number\0", ags_midi_builder_test_append_sequence_number) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append smtpe\0", ags_midi_builder_test_append_smtpe) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append time signature\0", ags_midi_builder_test_append_time_signature) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append key signature\0", ags_midi_builder_test_append_key_signature) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append sequencer meta event\0", ags_midi_builder_test_append_sequencer_meta_event) == NULL) || 
     (CU_add_test(pSuite, "test of AgsMidiBuilder to append text event\0", ags_midi_builder_test_append_text_event) == NULL)){
    CU_cleanup_registry();
    
    return CU_get_error();
  }
  
  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  
  CU_cleanup_registry();
  
  return(CU_get_error());
}


