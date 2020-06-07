/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2020 Joël Krähemann
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

int ags_sf2_synth_util_test_init_suite();
int ags_sf2_synth_util_test_clean_suite();

void ags_sf2_synth_util_test_copy_s8();
void ags_sf2_synth_util_test_copy_s16();
void ags_sf2_synth_util_test_copy_s24();
void ags_sf2_synth_util_test_copy_s32();
void ags_sf2_synth_util_test_copy_s64();
void ags_sf2_synth_util_test_copy_float();
void ags_sf2_synth_util_test_copy_double();
void ags_sf2_synth_util_test_copy_complex();

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_sf2_synth_util_test_init_suite()
{ 
  return(0);
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int
ags_sf2_synth_util_test_clean_suite()
{
  return(0);
}

void
ags_sf2_synth_util_test_copy_s8()
{
  CU_ASSERT(TRUE);
  
  //TODO:JK: implement me  
}

void
ags_sf2_synth_util_test_copy_s16()
{
  //TODO:JK: implement me  
}

void
ags_sf2_synth_util_test_copy_s24()
{
  //TODO:JK: implement me  
}

void
ags_sf2_synth_util_test_copy_s32()
{
  //TODO:JK: implement me  
}

void
ags_sf2_synth_util_test_copy_s64()
{
  //TODO:JK: implement me  
}

void
ags_sf2_synth_util_test_copy_float()
{
  //TODO:JK: implement me  
}

void
ags_sf2_synth_util_test_copy_double()
{
  //TODO:JK: implement me  
}

void
ags_sf2_synth_util_test_copy_complex()
{
  //TODO:JK: implement me  
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
  pSuite = CU_add_suite("AgsSF2SynthUtilTest", ags_sf2_synth_util_test_init_suite, ags_sf2_synth_util_test_clean_suite);
  
  if(pSuite == NULL){
    CU_cleanup_registry();
    
    return CU_get_error();
  }

  /* add the tests to the suite */
  if((CU_add_test(pSuite, "test of ags_sf2_synth_util.c copy s8", ags_sf2_synth_util_test_copy_s8) == NULL) ||
     (CU_add_test(pSuite, "test of ags_sf2_synth_util.c copy s16", ags_sf2_synth_util_test_copy_s16) == NULL) ||
     (CU_add_test(pSuite, "test of ags_sf2_synth_util.c copy s16", ags_sf2_synth_util_test_copy_s24) == NULL) ||
     (CU_add_test(pSuite, "test of ags_sf2_synth_util.c copy s16", ags_sf2_synth_util_test_copy_s32) == NULL) ||
     (CU_add_test(pSuite, "test of ags_sf2_synth_util.c copy s16", ags_sf2_synth_util_test_copy_s64) == NULL) ||
     (CU_add_test(pSuite, "test of ags_sf2_synth_util.c copy s16", ags_sf2_synth_util_test_copy_float) == NULL) ||
     (CU_add_test(pSuite, "test of ags_sf2_synth_util.c copy s16", ags_sf2_synth_util_test_copy_double) == NULL) ||
     (CU_add_test(pSuite, "test of ags_sf2_synth_util.c copy complex", ags_sf2_synth_util_test_copy_complex) == NULL)){
    CU_cleanup_registry();
      
    return CU_get_error();
  }
  
  /* Run all tests using the CUnit Basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  
  CU_cleanup_registry();
  
  return(CU_get_error());
}

