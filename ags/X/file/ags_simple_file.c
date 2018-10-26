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

#include <ags/X/file/ags_simple_file.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>
#include <ags/libags-gui.h>

#include <ags/X/ags_ui_provider.h>
#include <ags/X/ags_xorg_application_context.h>
#include <ags/X/ags_window.h>
#include <ags/X/ags_notation_editor.h>
#include <ags/X/ags_automation_window.h>
#include <ags/X/ags_automation_editor.h>
#include <ags/X/ags_machine.h>
#include <ags/X/ags_pad.h>
#include <ags/X/ags_line.h>
#include <ags/X/ags_line_member.h>
#include <ags/X/ags_effect_bridge.h>
#include <ags/X/ags_effect_bulk.h>
#include <ags/X/ags_bulk_member.h>
#include <ags/X/ags_effect_pad.h>
#include <ags/X/ags_effect_line.h>

#include <ags/X/editor/ags_machine_selector.h>
#include <ags/X/editor/ags_machine_radio_button.h>

#include <ags/X/machine/ags_panel.h>
#include <ags/X/machine/ags_mixer.h>
#include <ags/X/machine/ags_equalizer10.h>
#include <ags/X/machine/ags_spectrometer.h>
#include <ags/X/machine/ags_drum.h>
#include <ags/X/machine/ags_matrix.h>
#include <ags/X/machine/ags_synth.h>
#include <ags/X/machine/ags_synth_input_line.h>
#include <ags/X/machine/ags_syncsynth.h>
#include <ags/X/machine/ags_oscillator.h>

#ifdef AGS_WITH_LIBINSTPATCH
#include <ags/X/machine/ags_ffplayer.h>
#endif

#include <ags/X/machine/ags_audiorec.h>

#include <ags/X/machine/ags_ladspa_bridge.h>
#include <ags/X/machine/ags_dssi_bridge.h>
#include <ags/X/machine/ags_lv2_bridge.h>
#include <ags/X/machine/ags_live_dssi_bridge.h>
#include <ags/X/machine/ags_live_lv2_bridge.h>

#include <ags/config.h>

#ifdef AGS_WITH_LIBINSTPATCH
#include <libinstpatch/libinstpatch.h>
#endif

#include <libxml/parser.h>
#include <libxml/xlink.h>
#include <libxml/xpath.h>
#include <libxml/valid.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>

void ags_simple_file_class_init(AgsSimpleFileClass *simple_file);
void ags_simple_file_init(AgsSimpleFile *simple_file);
void ags_simple_file_set_property(GObject *gobject,
				  guint prop_id,
				  const GValue *value,
				  GParamSpec *param_spec);
void ags_simple_file_get_property(GObject *gobject,
				  guint prop_id,
				  GValue *value,
				  GParamSpec *param_spec);
void ags_simple_file_finalize(GObject *gobject);

void ags_simple_file_real_open(AgsSimpleFile *simple_file,
			       GError **error);
void ags_simple_file_real_open_from_data(AgsSimpleFile *simple_file,
					 gchar *data, guint length,
					 GError **error);
void ags_simple_file_real_rw_open(AgsSimpleFile *simple_file,
				  gboolean create,
				  GError **error);

void ags_simple_file_real_write(AgsSimpleFile *simple_file);
void ags_simple_file_real_write_resolve(AgsSimpleFile *simple_file);

void ags_simple_file_real_read(AgsSimpleFile *simple_file);
void ags_simple_file_real_read_resolve(AgsSimpleFile *simple_file);
void ags_simple_file_real_read_start(AgsSimpleFile *simple_file);

void ags_simple_file_read_config(AgsSimpleFile *simple_file, xmlNode *node, AgsConfig **config);
void ags_simple_file_read_property_list(AgsSimpleFile *simple_file, xmlNode *node, GList **property);
void ags_simple_file_read_property(AgsSimpleFile *simple_file, xmlNode *node, GParameter **property);
void ags_simple_file_read_strv(AgsSimpleFile *simple_file, xmlNode *node, gchar ***strv);
void ags_simple_file_read_value(AgsSimpleFile *simple_file, xmlNode *node, GValue **value);
void ags_simple_file_read_window(AgsSimpleFile *simple_file, xmlNode *node, AgsWindow **window);
void ags_simple_file_read_window_launch(AgsFileLaunch *file_launch,
					AgsWindow *window);
void ags_simple_file_read_machine_list(AgsSimpleFile *simple_file, xmlNode *node, GList **machine);
void ags_simple_file_read_machine(AgsSimpleFile *simple_file, xmlNode *node, AgsMachine **machine);
void ags_simple_file_read_machine_resize_pads(AgsMachine *machine,
					      GType channel_type,
					      guint new_size, guint old_size,
					      gpointer data);
void ags_simple_file_read_machine_launch(AgsFileLaunch *file_launch,
					 AgsMachine *machine);
void ags_simple_file_read_pad_list(AgsSimpleFile *simple_file, xmlNode *node, GList **pad);
void ags_simple_file_read_pad(AgsSimpleFile *simple_file, xmlNode *node, AgsPad **pad);
void ags_simple_file_read_pad_launch(AgsFileLaunch *file_launch,
				     AgsPad *pad);
void ags_simple_file_read_line_list(AgsSimpleFile *simple_file, xmlNode *node, GList **line);
void ags_simple_file_read_line(AgsSimpleFile *simple_file, xmlNode *node, AgsLine **line);
void ags_simple_file_read_line_launch(AgsFileLaunch *file_launch,
				      AgsLine *line);
void ags_simple_file_read_channel_line_launch(AgsFileLaunch *file_launch,
					      AgsChannel *channel);
void ags_simple_file_read_effect_pad_list(AgsSimpleFile *simple_file, xmlNode *node, GList **effect_pad);
void ags_simple_file_read_effect_pad(AgsSimpleFile *simple_file, xmlNode *node, AgsEffectPad **effect_pad);
void ags_simple_file_read_effect_pad_launch(AgsFileLaunch *file_launch,
					    AgsEffectPad *effect_pad);
void ags_simple_file_read_effect_line_list(AgsSimpleFile *simple_file, xmlNode *node, GList **effect_line);
void ags_simple_file_read_effect_line(AgsSimpleFile *simple_file, xmlNode *node, AgsEffectLine **effect_line);
void ags_simple_file_read_effect_line_launch(AgsFileLaunch *file_launch,
					     AgsEffectLine *effect_line);
void ags_simple_file_read_oscillator_list(AgsSimpleFile *simple_file, xmlNode *node, GList **oscillator);
void ags_simple_file_read_oscillator(AgsSimpleFile *simple_file, xmlNode *node, AgsOscillator **oscillator);

void ags_simple_file_read_notation_editor(AgsSimpleFile *simple_file, xmlNode *node, AgsNotationEditor **notation_editor);
void ags_simple_file_read_notation_editor_launch(AgsFileLaunch *file_launch,
						 AgsNotationEditor *notation_editor);
void ags_simple_file_read_automation_editor(AgsSimpleFile *simple_file, xmlNode *node, AgsAutomationEditor **automation_editor);
void ags_simple_file_read_automation_editor_launch(AgsFileLaunch *file_launch,
						   AgsAutomationEditor *automation_editor);
void ags_simple_file_read_wave_editor(AgsSimpleFile *simple_file, xmlNode *node, AgsWaveEditor **wave_editor);
void ags_simple_file_read_wave_editor_launch(AgsFileLaunch *file_launch,
					     AgsWaveEditor *wave_editor);

void ags_simple_file_read_notation_list(AgsSimpleFile *simple_file, xmlNode *node, GList **notation);
void ags_simple_file_read_notation(AgsSimpleFile *simple_file, xmlNode *node, AgsNotation **notation);
void ags_simple_file_read_notation_list_fixup_1_0_to_1_2(AgsSimpleFile *simple_file, xmlNode *node, GList **notation);

void ags_simple_file_read_automation_list(AgsSimpleFile *simple_file, xmlNode *node, GList **automation);
void ags_simple_file_read_automation(AgsSimpleFile *simple_file, xmlNode *node, AgsAutomation **automation);
void ags_simple_file_read_automation_list_fixup_1_0_to_1_3(AgsSimpleFile *simple_file, xmlNode *node, GList **automation);
void ags_simple_file_read_preset_list(AgsSimpleFile *simple_file, xmlNode *node, GList **preset);
void ags_simple_file_read_preset(AgsSimpleFile *simple_file, xmlNode *node, AgsPreset **preset);

xmlNode* ags_simple_file_write_config(AgsSimpleFile *simple_file, xmlNode *parent, AgsConfig *config);
xmlNode* ags_simple_file_write_window(AgsSimpleFile *simple_file, xmlNode *parent, AgsWindow *window);
xmlNode* ags_simple_file_write_property_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *property);
xmlNode* ags_simple_file_write_property(AgsSimpleFile *simple_file, xmlNode *parent, GParameter *property);
xmlNode* ags_simple_file_write_strv(AgsSimpleFile *simple_file, xmlNode *parent, gchar **strv);
xmlNode* ags_simple_file_write_value(AgsSimpleFile *simple_file, xmlNode *parent, GValue *value);
xmlNode* ags_simple_file_write_window(AgsSimpleFile *simple_file, xmlNode *parent, AgsWindow *window);
xmlNode* ags_simple_file_write_machine_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *machine);
xmlNode* ags_simple_file_write_machine(AgsSimpleFile *simple_file, xmlNode *parent, AgsMachine *machine);
xmlNode* ags_simple_file_write_pad_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *pad);
xmlNode* ags_simple_file_write_pad(AgsSimpleFile *simple_file, xmlNode *parent, AgsPad *pad);
xmlNode* ags_simple_file_write_line_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *line);
xmlNode* ags_simple_file_write_line(AgsSimpleFile *simple_file, xmlNode *parent, AgsLine *line);
void ags_simple_file_write_line_resolve_link(AgsFileLookup *file_lookup,
					     AgsChannel *channel);
xmlNode* ags_simple_file_write_effect_pad_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *effect_pad);
xmlNode* ags_simple_file_write_effect_pad(AgsSimpleFile *simple_file, xmlNode *parent, AgsEffectPad *effect_pad);
xmlNode* ags_simple_file_write_effect_line_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *effect_line);
xmlNode* ags_simple_file_write_effect_line(AgsSimpleFile *simple_file, xmlNode *parent, AgsEffectLine *effect_line);
xmlNode* ags_simple_file_write_oscillator_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *oscillator);
xmlNode* ags_simple_file_write_oscillator(AgsSimpleFile *simple_file, xmlNode *parent, AgsOscillator *oscillator);

xmlNode* ags_simple_file_write_notation_editor(AgsSimpleFile *simple_file, xmlNode *parent, AgsNotationEditor *notation_editor);
void ags_simple_file_write_notation_editor_resolve_machine(AgsFileLookup *file_lookup,
						  AgsNotationEditor *notation_editor);
xmlNode* ags_simple_file_write_automation_editor(AgsSimpleFile *simple_file, xmlNode *parent, AgsAutomationEditor *automation_editor);
void ags_simple_file_write_automation_editor_resolve_machine(AgsFileLookup *file_lookup,
							     AgsAutomationEditor *automation_editor);
xmlNode* ags_simple_file_write_wave_editor(AgsSimpleFile *simple_file, xmlNode *parent, AgsWaveEditor *wave_editor);
void ags_simple_file_write_wave_editor_resolve_machine(AgsFileLookup *file_lookup,
						       AgsWaveEditor *wave_editor);
xmlNode* ags_simple_file_write_notation_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *notation);
xmlNode* ags_simple_file_write_notation(AgsSimpleFile *simple_file, xmlNode *parent, AgsNotation *notation);
xmlNode* ags_simple_file_write_automation_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *automation);
xmlNode* ags_simple_file_write_automation(AgsSimpleFile *simple_file, xmlNode *parent, AgsAutomation *automation);
xmlNode* ags_simple_file_write_preset_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *preset);
xmlNode* ags_simple_file_write_preset(AgsSimpleFile *simple_file, xmlNode *parent, AgsPreset *preset);

/**
 * SECTION:ags_file
 * @short_description: read/write XML file
 * @title: AgsSimpleFile
 * @section_id:
 * @include: ags/file/ags_file.h
 *
 * The #AgsSimpleFile is an object to read or write files using XML. It
 * is the persisting layer of Advanced Gtk+ Sequencer.
 */

enum{
  PROP_0,
  PROP_FILENAME,
  PROP_ENCODING,
  PROP_AUDIO_FORMAT,
  PROP_AUDIO_ENCODING,
  PROP_XML_DOC,
  PROP_APPLICATION_CONTEXT,
};

enum{
  OPEN,
  OPEN_FROM_DATA,
  RW_OPEN,
  WRITE,
  WRITE_RESOLVE,
  READ,
  READ_RESOLVE,
  READ_START,
  LAST_SIGNAL,
};

static gpointer ags_simple_file_parent_class = NULL;
static guint simple_file_signals[LAST_SIGNAL] = { 0 };

GType
ags_simple_file_get_type(void)
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_simple_file = 0;

    static const GTypeInfo ags_simple_file_info = {
      sizeof (AgsSimpleFileClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_simple_file_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsSimpleFile),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_simple_file_init,
    };

    ags_type_simple_file = g_type_register_static(G_TYPE_OBJECT,
						  "AgsSimpleFile",
						  &ags_simple_file_info,
						  0);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_simple_file);
  }

  return g_define_type_id__volatile;
}

void
ags_simple_file_class_init(AgsSimpleFileClass *simple_file)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;

  ags_simple_file_parent_class = g_type_class_peek_parent(simple_file);

  /* GObjectClass */
  gobject = (GObjectClass *) simple_file;

  gobject->get_property = ags_simple_file_get_property;
  gobject->set_property = ags_simple_file_set_property;

  gobject->finalize = ags_simple_file_finalize;

  /* properties */
  /**
   * AgsSimpleFile:filename:
   *
   * The assigned filename to open and read from.
   *
   * Since: 2.0.0
   */
  param_spec = g_param_spec_string("filename",
				   "filename to read or write",
				   "The filename to read or write to.",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FILENAME,
				  param_spec);

  /**
   * AgsSimpleFile:encoding:
   *
   * The charset encoding to use.
   *
   * Since: 2.0.0
   */
  param_spec = g_param_spec_string("encoding",
				   "encoding to use",
				   "The encoding of the XML document.",
				   NULL,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_ENCODING,
				  param_spec);

  /**
   * AgsSimpleFile:audio-format:
   *
   * The format of embedded audio data.
   *
   * Since: 2.0.0
   */
  param_spec = g_param_spec_string("audio-format",
				   "audio format to use",
				   "The audio format used to embedded audio.",
				   AGS_SIMPLE_FILE_DEFAULT_AUDIO_FORMAT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_FORMAT,
				  param_spec);

  /**
   * AgsSimpleFile:audio-encoding:
   *
   * The encoding to use for embedding audio data.
   *
   * Since: 2.0.0
   */
  param_spec = g_param_spec_string("audio-encoding",
				   "audio encoding to use",
				   "The audio encoding used to embedded audio.",
				   AGS_SIMPLE_FILE_DEFAULT_AUDIO_ENCODING,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_AUDIO_ENCODING,
				  param_spec);

  /**
   * AgsSimpleFile:xml-doc:
   *
   * The assigned xml-doc.
   * 
   * Since: 2.0.0
   */
  param_spec = g_param_spec_pointer("xml-doc",
				    "xml document of file",
				    "The xml document assigned with file",
				    G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_XML_DOC,
				  param_spec);

  /**
   * AgsSimpleFile:application-context:
   *
   * The application context assigned with.
   *
   * Since: 2.0.0
   */
  param_spec = g_param_spec_object("application-context",
				   "application context of file",
				   "The application context to write to file.",
				   AGS_TYPE_APPLICATION_CONTEXT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_APPLICATION_CONTEXT,
				  param_spec);

  /* AgsSimpleFileClass */
  simple_file->open = ags_simple_file_real_open;
  simple_file->rw_open = ags_simple_file_real_rw_open;
  simple_file->open_from_data = ags_simple_file_real_open_from_data;

  simple_file->write = ags_simple_file_real_write;
  simple_file->write_resolve = ags_simple_file_real_write_resolve;
  simple_file->read = ags_simple_file_real_read;
  simple_file->read_resolve = ags_simple_file_real_read_resolve;
  simple_file->read_start = ags_simple_file_real_read_start;

  /* signals */
  /**
   * AgsSimpleFile::open:
   * @simple_file: the #AgsSimpleFile
   * @error: a #GError-struct pointer to return error
   * 
   * Open @simple_file with appropriate filename.
   *
   * Since: 2.0.0
   */
  simple_file_signals[OPEN] =
    g_signal_new("open",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, open),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__POINTER,
		 G_TYPE_NONE, 1,
		 G_TYPE_POINTER);

  /**
   * AgsSimpleFile::open-from-data:
   * @simple_file: the #AgsSimpleFile
   * @buffer: the buffer containing the file
   * @length: the buffer length
   * @error: a #GError-struct pointer to return error
   * 
   * Open @simple_file from a buffer containing the file.
   *
   * Since: 2.0.0
   */
  simple_file_signals[OPEN_FROM_DATA] =
    g_signal_new("open-from-data",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, open_from_data),
		 NULL, NULL,
		 ags_cclosure_marshal_VOID__STRING_UINT_POINTER,
		 G_TYPE_NONE, 3,
		 G_TYPE_STRING,
		 G_TYPE_UINT,
		 G_TYPE_POINTER);

  /**
   * AgsSimpleFile::open-from-data:
   * @simple_file: the #AgsSimpleFile
   * @create: if %TRUE the file will be created if not exists
   * @error: a #GError-struct pointer to return error
   * 
   * Open @simple_file in read-write mode.
   *
   * Since: 2.0.0
   */
  simple_file_signals[RW_OPEN] =
    g_signal_new("rw-open",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, rw_open),
		 NULL, NULL,
		 ags_cclosure_marshal_VOID__BOOLEAN_POINTER,
		 G_TYPE_NONE, 2,
		 G_TYPE_BOOLEAN,
		 G_TYPE_POINTER);

  /**
   * AgsSimpleFile::write:
   * @simple_file: the #AgsSimpleFile
   * 
   * Write XML Document to disk.
   *
   * Since: 2.0.0
   */
  simple_file_signals[WRITE] =
    g_signal_new("write",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, write),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsSimpleFile::write-resolve:
   * @simple_file: the #AgsSimpleFile
   *
   * Resolve references and generate thus XPath expressions just
   * before writing to disk.
   *
   * Since: 2.0.0
   */
  simple_file_signals[WRITE_RESOLVE] =
    g_signal_new("write_resolve",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, write_resolve),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsSimpleFile::read:
   * @simple_file: the #AgsSimpleFile
   *
   * Read a XML document from disk with specified simple_filename.
   * 
   * Since: 2.0.0
   */
  simple_file_signals[READ] =
    g_signal_new("read",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, read),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsSimpleFile::read-resolve:
   * @simple_file: the #AgsSimpleFile
   *
   * Resolve XPath expressions to their counterpart the newly created
   * instances refering to.
   * 
   * Since: 2.0.0
   */
  simple_file_signals[READ_RESOLVE] =
    g_signal_new("read_resolve",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, read_resolve),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);

  /**
   * AgsSimpleFile::read-start:
   * @simple_file: the #AgsSimpleFile
   *
   * Hook after reading XML document to update or start the application.
   * 
   * Since: 2.0.0
   */
  simple_file_signals[READ_START] =
    g_signal_new("read_start",
		 G_TYPE_FROM_CLASS(simple_file),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET(AgsSimpleFileClass, read_start),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__VOID,
		 G_TYPE_NONE, 0);
}

GQuark
ags_simple_file_error_quark()
{
  return(g_quark_from_static_string("ags-simple-file-error-quark"));
}

void
ags_simple_file_init(AgsSimpleFile *simple_file)
{
  simple_file->flags = 0;

  simple_file->out = NULL;
  simple_file->buffer = NULL;

  simple_file->filename = NULL;
  simple_file->encoding = AGS_SIMPLE_FILE_DEFAULT_ENCODING;
  simple_file->dtd = AGS_SIMPLE_FILE_DEFAULT_DTD;

  simple_file->audio_format = AGS_SIMPLE_FILE_DEFAULT_AUDIO_FORMAT;
  simple_file->audio_encoding = AGS_SIMPLE_FILE_DEFAULT_AUDIO_ENCODING;

  simple_file->doc = NULL;

  simple_file->id_ref = NULL;
  simple_file->lookup = NULL;
  simple_file->launch = NULL;

  simple_file->application_context = NULL;
}

void
ags_simple_file_set_property(GObject *gobject,
			     guint prop_id,
			     const GValue *value,
			     GParamSpec *param_spec)
{
  AgsSimpleFile *simple_file;

  simple_file = AGS_SIMPLE_FILE(gobject);

  switch(prop_id){
  case PROP_FILENAME:
    {
      gchar *filename;

      filename = g_value_get_string(value);

      if(simple_file->filename == filename){
	return;
      }

      if(simple_file->filename != NULL){
	g_free(simple_file->filename);
      }
      
      simple_file->filename = g_strdup(filename);
    }
    break;
  case PROP_ENCODING:
    {
      gchar *encoding;

      encoding = g_value_get_string(value);

      simple_file->encoding = encoding;
    }
    break;
  case PROP_AUDIO_FORMAT:
    {
      gchar *audio_format;

      audio_format = g_value_get_string(value);

      simple_file->audio_format = audio_format;
    }
    break;
  case PROP_AUDIO_ENCODING:
    {
      gchar *audio_encoding;

      audio_encoding = g_value_get_string(value);

      simple_file->audio_encoding = audio_encoding;
    }
    break;
  case PROP_XML_DOC:
    {
      xmlDoc *doc;

      doc = (xmlDoc *) g_value_get_pointer(value);
      
      simple_file->doc = doc;
    }
    break;
  case PROP_APPLICATION_CONTEXT:
    {
      GObject *application_context;

      application_context = g_value_get_object(value);

      if(simple_file->application_context == application_context){
	return;
      }

      if(simple_file->application_context != NULL){
	g_object_unref(simple_file->application_context);
      }

      if(application_context != NULL){
	g_object_ref(application_context);
      }

      simple_file->application_context = application_context;
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_simple_file_get_property(GObject *gobject,
			     guint prop_id,
			     GValue *value,
			     GParamSpec *param_spec)
{
  AgsSimpleFile *simple_file;

  simple_file = AGS_SIMPLE_FILE(gobject);

  switch(prop_id){
  case PROP_FILENAME:
    {
      g_value_set_string(value, simple_file->filename);
    }
    break;
  case PROP_ENCODING:
    {
      g_value_set_string(value, simple_file->encoding);
    }
    break;
  case PROP_AUDIO_FORMAT:
    {
      g_value_set_string(value, simple_file->audio_format);
    }
    break;
  case PROP_AUDIO_ENCODING:
    {
      g_value_set_string(value, simple_file->audio_encoding);
    }
    break;
  case PROP_XML_DOC:
    {
      g_value_set_pointer(value, simple_file->doc);
    }
    break;
  case PROP_APPLICATION_CONTEXT:
    {
      g_value_set_object(value, simple_file->application_context);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_simple_file_finalize(GObject *gobject)
{
  //TODO:JK: implement me
}

gchar*
ags_simple_file_str2md5(gchar *content, guint content_length)
{
  GChecksum *checksum;
  gchar *str;

  str = g_compute_checksum_for_string(G_CHECKSUM_MD5,
				      content,
				      content_length);

  return(str);
}

void
ags_simple_file_add_id_ref(AgsSimpleFile *simple_file, GObject *id_ref)
{
  if(simple_file == NULL ||
     id_ref == NULL){
    return;
  }
  
  g_object_ref(id_ref);
  simple_file->id_ref = g_list_prepend(simple_file->id_ref,
				       id_ref);
}

GObject*
ags_simple_file_find_id_ref_by_node(AgsSimpleFile *simple_file, xmlNode *node)
{
  AgsFileIdRef *file_id_ref;
  GList *list;

  if(simple_file == NULL ||
     node == NULL){
    return(NULL);
  }
  
  list = simple_file->id_ref;

  while(list != NULL){
    file_id_ref = AGS_FILE_ID_REF(list->data);

    if(file_id_ref->node == node){
      return((GObject *) file_id_ref);
    }

    list = list->next;
  }

  return(NULL);
}

GList*
ags_simple_file_find_id_ref_by_xpath(AgsSimpleFile *simple_file, gchar *xpath)
{
  AgsFileIdRef *file_id_ref;

  xmlXPathContext *xpath_context; 
  xmlXPathObject *xpath_object;
  xmlNode **node;

  GList *list;
  
  guint i;

  if(simple_file == NULL || xpath == NULL || !g_str_has_prefix(xpath, "xpath=")){
    g_message("invalid xpath: %s", xpath);

    return(NULL);
  }

  xpath = &(xpath[6]);

  /* Create xpath evaluation context */
  xpath_context = xmlXPathNewContext(simple_file->doc);

  if(xpath_context == NULL) {
    g_warning("Error: unable to create new XPath context");

    return(NULL);
  }

  /* Evaluate xpath expression */
  xpath_object = xmlXPathEval(xpath, xpath_context);

  if(xpath_object == NULL) {
    g_warning("Error: unable to evaluate xpath expression \"%s\"", xpath);
    xmlXPathFreeContext(xpath_context); 

    return(NULL);
  }

  node = xpath_object->nodesetval->nodeTab;

  list = NULL;
  
  for(i = 0; i < xpath_object->nodesetval->nodeNr; i++){
    if(node[i]->type == XML_ELEMENT_NODE){
      GObject *gobject;
      
      gobject = ags_simple_file_find_id_ref_by_node(simple_file,
						    node[i]);

      if(gobject != NULL){
	list = g_list_prepend(list,
			      gobject);
      }
    }
  }

  if(list == NULL){
    g_message("no xpath match: %s", xpath);
  }
  
  return(list);
}

GList*
ags_simple_file_find_id_ref_by_reference(AgsSimpleFile *simple_file, gpointer ref)
{
  AgsFileIdRef *file_id_ref;
  
  GList *list;
  GList *ref_list;
  
  if(simple_file == NULL || ref == NULL){
    return(NULL);
  }

  list = simple_file->id_ref;
  ref_list = NULL;
  
  while(list != NULL){
    file_id_ref = AGS_FILE_ID_REF(list->data);

    if(file_id_ref->ref == ref){
      ref_list = g_list_prepend(ref_list,
				file_id_ref);
    }

    list = list->next;
  }

  return(ref_list);
}

void
ags_simple_file_add_lookup(AgsSimpleFile *simple_file, GObject *file_lookup)
{
  if(simple_file == NULL || file_lookup == NULL){
    return;
  }

  g_object_ref(G_OBJECT(file_lookup));

  simple_file->lookup = g_list_prepend(simple_file->lookup,
				       file_lookup);
}

void
ags_simple_file_add_launch(AgsSimpleFile *simple_file, GObject *file_launch)
{
  if(simple_file == NULL || file_launch == NULL){
    return;
  }

  g_object_ref(G_OBJECT(file_launch));

  simple_file->launch = g_list_prepend(simple_file->launch,
				       file_launch);
}

void
ags_simple_file_real_open(AgsSimpleFile *simple_file,
			  GError **error)
{
  /* parse the file and get the DOM */
  simple_file->doc = xmlReadFile(simple_file->filename, NULL, 0);

  if(simple_file->doc == NULL){
    g_warning("ags_simple_file.c - failed to read XML document %s", simple_file->filename);

    if(error != NULL){
      g_set_error(error,
		  AGS_SIMPLE_FILE_ERROR,
		  AGS_SIMPLE_FILE_ERROR_PARSER_FAILURE,
		  "unable to parse document: %s\n",
		  simple_file->filename);
    }
  }else{
    /* Get the root element node */
    simple_file->root_node = xmlDocGetRootElement(simple_file->doc);
  }
}

void
ags_simple_file_open(AgsSimpleFile *simple_file,
		     GError **error)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[OPEN], 0,
		error);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_open_from_data(AgsSimpleFile *simple_file,
				    gchar *data, guint length,
				    GError **error)
{
  simple_file->doc = xmlReadMemory(data, length, simple_file->filename, NULL, 0);

  if(simple_file->doc == NULL){
    g_warning("ags_simple_file.c - failed to read XML document %s", simple_file->filename);

    if(error != NULL){
      g_set_error(error,
		  AGS_SIMPLE_FILE_ERROR,
		  AGS_SIMPLE_FILE_ERROR_PARSER_FAILURE,
		  "unable to parse document from data: %s\n",
		  simple_file->filename);
    }
  }else{
    /* Get the root element node */
    simple_file->root_node = xmlDocGetRootElement(simple_file->doc);
  }
}

void
ags_simple_file_open_from_data(AgsSimpleFile *simple_file,
			       gchar *data, guint length,
			       GError **error)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[OPEN_FROM_DATA], 0,
		data, length,
		error);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_rw_open(AgsSimpleFile *simple_file,
			     gboolean create,
			     GError **error)
{
  simple_file->out = fopen(simple_file->filename, "w+");

  simple_file->doc = xmlNewDoc("1.0");
  simple_file->root_node = xmlNewNode(NULL, "ags-simple-file");
  xmlDocSetRootElement(simple_file->doc, simple_file->root_node);
}

void
ags_simple_file_rw_open(AgsSimpleFile *simple_file,
			gboolean create,
			GError **error)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[RW_OPEN], 0,
		create,
		error);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_open_filename(AgsSimpleFile *simple_file,
			      gchar *filename)
{
  GError *error;
  
  if(simple_file == NULL){
    return;
  }

  if(simple_file->filename != NULL){
    ags_simple_file_close(simple_file);
  }

  error = NULL;
  g_object_set(simple_file,
	       "filename", filename,
	       NULL);
  ags_simple_file_open(simple_file,
		       &error);

  if(error != NULL){
    g_warning("%s", error->message);
  }
}

void
ags_simple_file_close(AgsSimpleFile *simple_file)
{
  if(simple_file == NULL){
    return;
  }

  if(simple_file->out != NULL){
    fclose(simple_file->out);
  }
  
  /* free the document */
  xmlFreeDoc(simple_file->doc);

  /*
   * Free the global variables that may
   * have been allocated by the parser.
   */
  xmlCleanupParser();

  /*
   * this is to debug memory for regression tests
   */
  xmlMemoryDump();

  simple_file->filename = NULL;
}

void
ags_simple_file_real_write(AgsSimpleFile *simple_file)
{
  AgsApplicationContext *application_context;
  AgsConfig *config;
  
  xmlNode *node, *child;
  
  gchar *id;

  guint size;
  
  if(simple_file == NULL ||
     simple_file->root_node == NULL){
    return;
  }

  application_context = (AgsApplicationContext *) simple_file->application_context;
  config = ags_config_get_instance();
  
  id = ags_id_generator_create_uuid();

  node = simple_file->root_node;
  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     AGS_APPLICATION_CONTEXT(application_context)->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     AGS_APPLICATION_CONTEXT(application_context)->build_id);

  /* add to parent */
  ags_simple_file_write_config(simple_file,
			       node,
			       config);  
  
  ags_simple_file_write_window(simple_file,
			       node,
			       AGS_XORG_APPLICATION_CONTEXT(application_context)->window);

  /* resolve */
  ags_simple_file_write_resolve(simple_file);

  /* 
   * Dumping document to file
   */
  //  xmlSaveFormatFileEnc(simple_file->filename, simple_file->doc, "UTF-8", 1);
  xmlDocDumpFormatMemoryEnc(simple_file->doc, &(simple_file->buffer), &size, simple_file->encoding, TRUE);

  fwrite(simple_file->buffer, size, sizeof(xmlChar), simple_file->out);
  fflush(simple_file->out);
}


void
ags_simple_file_write(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[WRITE], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_write_resolve(AgsSimpleFile *simple_file)
{
  GList *list;
  
  list = simple_file->lookup;

  while(list != NULL){
    ags_file_lookup_resolve(AGS_FILE_LOOKUP(list->data));

    list = list->next;
  }
}

void
ags_simple_file_write_resolve(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[WRITE_RESOLVE], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_read(AgsSimpleFile *simple_file)
{
  AgsApplicationContext *application_context;

  AgsGuiThread *gui_thread;

  xmlNode *root_node, *child;
  
  application_context = (AgsApplicationContext *) simple_file->application_context;

  gui_thread = ags_thread_find_type(application_context->main_loop,
				    AGS_TYPE_GUI_THREAD);

  root_node = simple_file->root_node;

  /* child elements */
  child = root_node->children;

  /*  */
  ags_application_context_register_types(application_context);
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp("ags-sf-config",
		     child->name,
		     13)){
	//NOTE:JK: no redundant code here
      }else if(!xmlStrncmp("ags-sf-window",
			   child->name,
			   14)){
	ags_simple_file_read_window(simple_file,
				    child,
				    (AgsWindow **) &(AGS_XORG_APPLICATION_CONTEXT(application_context)->window));
      }
    }

    child = child->next;
  }

  /* resolve */
  ags_simple_file_read_resolve(simple_file);
  
  g_message("XML simple file resolved");

  /* connect */  
  ags_connectable_connect(AGS_CONNECTABLE(application_context));

  gtk_widget_show_all((GtkWidget *) AGS_XORG_APPLICATION_CONTEXT(application_context)->window);

  g_message("XML simple file connected");

  /* start */
  ags_simple_file_read_start(simple_file);

  /* set file ready */
  g_atomic_int_set(&(AGS_XORG_APPLICATION_CONTEXT(application_context)->file_ready),
		   TRUE);
    
}

void
ags_simple_file_read(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[READ], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_read_resolve(AgsSimpleFile *simple_file)
{
  GList *list;

  list = g_list_reverse(simple_file->lookup);
  
  while(list != NULL){
    ags_file_lookup_resolve(AGS_FILE_LOOKUP(list->data));

    list = list->next;
  }
}

void
ags_simple_file_read_resolve(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[READ_RESOLVE], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_real_read_start(AgsSimpleFile *simple_file)
{
  GList *list;

  list = g_list_reverse(simple_file->launch);

  while(list != NULL){
    ags_file_launch_start(AGS_FILE_LAUNCH(list->data));

    list = list->next;
  }
}

void
ags_simple_file_read_start(AgsSimpleFile *simple_file)
{
  g_return_if_fail(AGS_IS_SIMPLE_FILE(simple_file));

  g_object_ref(G_OBJECT(simple_file));
  g_signal_emit(G_OBJECT(simple_file),
		simple_file_signals[READ_START], 0);
  g_object_unref(G_OBJECT(simple_file));
}

void
ags_simple_file_read_config(AgsSimpleFile *simple_file, xmlNode *node, AgsConfig **ags_config)
{
  AgsConfig *gobject;

  gchar *id;

  char *buffer;
  gsize buffer_length;

  gobject = *ags_config;
  gobject->version = xmlGetProp(node,
				AGS_FILE_VERSION_PROP);

  gobject->build_id = xmlGetProp(node,
				 AGS_FILE_BUILD_ID_PROP);

  buffer = xmlNodeGetContent(node);
  buffer_length = xmlStrlen(buffer);

  g_message("%s", buffer);
  
  ags_config_load_from_data(gobject,
			    buffer, buffer_length);
}

void
ags_simple_file_read_property_list(AgsSimpleFile *simple_file, xmlNode *node, GList **property)
{
  GParameter *current;
  GList *list;
  
  xmlNode *child;

  guint i;
  
  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-property",
		     11)){
	current = NULL;

	if(*property != NULL){
	  GList *iter;

	  iter = g_list_nth(*property,
			    i);

	  if(iter != NULL){
	    current = iter->data;
	  }
	}
	
	ags_simple_file_read_property(simple_file, child, &current);
	list = g_list_prepend(list, current);

	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *property = list;
}

void
ags_simple_file_read_property(AgsSimpleFile *simple_file, xmlNode *node, GParameter **property)
{
  GParameter *pointer;

  xmlChar *str;
  xmlChar *type;
  
  if(*property != NULL){
    pointer = *property;
  }else{
    pointer = (GParameter *) g_new0(GParameter,
				    1);
    pointer->name = NULL;
    
    *property = pointer;
  }

  str = xmlGetProp(node,
		   "name");

  if(str != NULL){
    pointer->name = g_strdup(str);
  }
  
  str = xmlGetProp(node,
		   "value");

  type = xmlGetProp(node,
		    "type");

  if(str != NULL){
    if(!g_strcmp0(type,
		  "gboolean")){
      g_value_init(&(pointer->value),
		   G_TYPE_BOOLEAN);
      
      if(!g_strcmp0(str,
		    "false")){
	g_value_set_boolean(&(pointer->value),
			    FALSE);
      }else{
	g_value_set_boolean(&(pointer->value),
			    TRUE);
      }
    }else if(!g_strcmp0(type,
			"guint")){
      guint val;

      g_value_init(&(pointer->value),
		   G_TYPE_UINT);
      
      val = g_ascii_strtoull(str,
			     NULL,
			     10);

      g_value_set_uint(&(pointer->value),
		       val);
    }else if(!g_strcmp0(type,
			"gint")){
      gint val;

      g_value_init(&(pointer->value),
		   G_TYPE_UINT);
      
      val = g_ascii_strtoll(str,
			    NULL,
			    10);

      g_value_set_int(&(pointer->value),
		      val);
    }else if(!g_strcmp0(type,
			"gdouble")){
      gdouble val;
      
      g_value_init(&(pointer->value),
		   G_TYPE_DOUBLE);

      val = g_ascii_strtod(str,
			   NULL);

      g_value_set_double(&(pointer->value),
			 val);
    }else if(!g_strcmp0(type,
			"AgsComplex")){
      AgsComplex z;
      
      g_value_init(&(pointer->value),
		   AGS_TYPE_COMPLEX);

      sscanf(str, "%f %f", &(z[0]), &(z[1]));
      g_value_set_boxed(&(pointer->value),
			&z);
    }else{
      g_value_init(&(pointer->value),
		   G_TYPE_STRING);

      g_value_set_string(&(pointer->value),
			 str);
    }
  }
}

void
ags_simple_file_read_strv(AgsSimpleFile *simple_file, xmlNode *node, gchar ***strv)
{
  xmlNode *child;
  
  gchar **current;

  guint i;
  
  current = NULL;
  
  child = node->children;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-str",
		     11)){
	if(current == NULL){
	  current = malloc(2 * sizeof(gchar *));
	}else{
	  current = realloc(current,
			    (i + 2) * sizeof(gchar *));
	}

	current[i] = xmlNodeGetContent(child);
	
	i++;
      }
    }

    child = child->next;
  }

  /* set return value */
  strv[0] = current;
}

void
ags_simple_file_read_value(AgsSimpleFile *simple_file, xmlNode *node, GValue **value)
{
  GValue *current;

  gchar *str, *type;
  
  current = value[0];
  
  str = xmlGetProp(node,
		   "value");

  type = xmlGetProp(node,
		    "type");

  if(str != NULL){
    if(!g_strcmp0(type,
		  "gboolean")){
      g_value_init(current,
		   G_TYPE_BOOLEAN);
      
      if(!g_strcmp0(str,
		    "false")){
	g_value_set_boolean(current,
			    FALSE);
      }else{
	g_value_set_boolean(current,
			    TRUE);
      }
    }else if(!g_strcmp0(type,
			"guint")){
      guint val;

      g_value_init(current,
		   G_TYPE_UINT);
      
      val = g_ascii_strtoull(str,
			     NULL,
			     10);

      g_value_set_uint(current,
		       val);
    }else if(!g_strcmp0(type,
			"gint")){
      gint val;

      g_value_init(current,
		   G_TYPE_UINT);
      
      val = g_ascii_strtoll(str,
			    NULL,
			    10);

      g_value_set_int(current,
		      val);
    }else if(!g_strcmp0(type,
			"gdouble")){
      gdouble val;
      
      g_value_init(current,
		   G_TYPE_DOUBLE);

      val = g_ascii_strtod(str,
			   NULL);

      g_value_set_double(current,
			 val);
    }else if(!g_strcmp0(type,
			"AgsComplex")){
      AgsComplex z;
      
      g_value_init(current,
		   AGS_TYPE_COMPLEX);

      sscanf(str, "%f %f", &(z[0]), &(z[1]));
      g_value_set_boxed(current,
			&z);
    }else{
      g_value_init(current,
		   G_TYPE_STRING);

      g_value_set_string(current,
			 str);
    }
  }
}

void
ags_simple_file_read_window(AgsSimpleFile *simple_file, xmlNode *node, AgsWindow **window)
{
  AgsWindow *gobject;

  AgsFileLaunch *file_launch;

  AgsApplicationContext *application_context;
  AgsConfig *config;
  
  xmlNode *child;

  GList *list;
  
  xmlChar *str;

  guint pcm_channels;
  guint samplerate;
  guint buffer_size;
  guint format;
  
  if(*window != NULL){
    gobject = *window;
  }else{
    gobject = ags_window_new(simple_file->application_context);
    AGS_XORG_APPLICATION_CONTEXT(simple_file->application_context)->window = gobject;
    
    *window = gobject;
  }

  str = xmlGetProp(node,
		   "filename");

  if(str != NULL){
    gobject->name = g_strdup(str);

    gtk_window_set_title((GtkWindow *) gobject, g_strconcat("GSequencer - ", gobject->name, NULL));
  }
  
  /* children */  
  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-machine-list",
		     20)){
	GList *machine_start;

	machine_start = NULL;
	ags_simple_file_read_machine_list(simple_file,
					  child,
					  &machine_start);
	g_list_free(machine_start);
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-notation-editor",
			   23)){
	ags_simple_file_read_notation_editor(simple_file,
					     child,
					     &(gobject->notation_editor));
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-automation-editor",
			   25)){
	ags_simple_file_read_automation_editor(simple_file,
					       child,
					       &(AGS_AUTOMATION_WINDOW(gobject->automation_window)->automation_editor));
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-wave-editor",
			   19)){
	ags_simple_file_read_wave_editor(simple_file,
					 child,
					 &(AGS_WAVE_WINDOW(gobject->wave_window)->wave_editor));
      }
    }

    child = child->next;
  }

  config = ags_config_get_instance();

  /* presets */
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "pcm-channels");

  if(str != NULL){
    pcm_channels = g_ascii_strtoull(str,
				    NULL,
				    10);
    g_free(str);
  }
  
  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "samplerate");
  
  if(str != NULL){
    samplerate = g_ascii_strtoull(str,
				  NULL,
				  10);
    free(str);
  }

  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "buffer-size");
  if(str != NULL){
    buffer_size = g_ascii_strtoull(str,
				   NULL,
				   10);
    free(str);
  }

  str = ags_config_get_value(config,
			     AGS_CONFIG_SOUNDCARD,
			     "format");
  if(str != NULL){
    format = g_ascii_strtoull(str,
			      NULL,
			      10);
    free(str);
  }
  
  /* launch settings */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_simple_file_read_window_launch), gobject);
  ags_simple_file_add_launch(simple_file,
			     (GObject *) file_launch);
}

void
ags_simple_file_read_window_launch(AgsFileLaunch *file_launch,
				   AgsWindow *window)
{
  xmlChar *str;
  
  gdouble bpm;
  gdouble loop_start, loop_end;
  gboolean loop;

  /* bpm */
  str = xmlGetProp(file_launch->node,
		   "bpm");

  if(str != NULL){
    bpm = g_ascii_strtod(str,
			 NULL);
    gtk_adjustment_set_value(window->navigation->bpm->adjustment,
			     bpm);
  }

  /* loop */
  str = xmlGetProp(file_launch->node,
		   "loop");

  if(str != NULL){
    if(!g_strcmp0(str,
		  "false")){
      gtk_toggle_button_set_active((GtkToggleButton *) window->navigation->loop,
				   FALSE);
    }else{
      gtk_toggle_button_set_active((GtkToggleButton *) window->navigation->loop,
				   TRUE);
    }
  }
  
  /* loop start */
  str = xmlGetProp(file_launch->node,
		   "loop-start");
  
  if(str != NULL){
    loop_start = g_ascii_strtod(str,
				NULL);
    gtk_adjustment_set_value(window->navigation->loop_left_tact->adjustment,
			     loop_start);
  }

  /* loop end */
  str = xmlGetProp(file_launch->node,
		   "loop-end");

  if(str != NULL){
    loop_end = g_ascii_strtod(str,
			      NULL);
    gtk_adjustment_set_value(window->navigation->loop_right_tact->adjustment,
			     loop_end);
  }
}

void
ags_simple_file_read_machine_list(AgsSimpleFile *simple_file, xmlNode *node, GList **machine)
{
  AgsMachine *current;
  
  xmlNode *child;

  GList *list;

  guint i;
  
  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-machine",
		     11)){
	current = NULL;

	if(*machine != NULL){
	  GList *iter;

	  iter = g_list_nth(*machine,
			    i);

	  if(iter != NULL){
	    current = iter->data;
	  }
	}
	
	ags_simple_file_read_machine(simple_file, child, &current);
	list = g_list_prepend(list, current);

	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *machine = list;
}

void
ags_simple_file_read_machine_resize_pads(AgsMachine *machine,
					 GType channel_type,
					 guint new_size, guint old_size,
					 gpointer data)
{
  gboolean *resized;

  resized = data;

  if(channel_type == AGS_TYPE_OUTPUT){
    resized[0] = TRUE;
  }else{
    resized[1] = TRUE;    
  }
}

void
ags_simple_file_read_machine(AgsSimpleFile *simple_file, xmlNode *node, AgsMachine **machine)
{
  AgsWindow *window;
  AgsMachine *gobject;

  AgsConfig *config;
  GObject *soundcard;

  AgsFileLaunch *file_launch;

  xmlNode *child;
  
  GList *list;
  GList *output_pad;
  GList *input_pad;

  xmlChar *device;
  xmlChar *type_name;
  xmlChar *str;
  
  guint audio_channels;
  guint output_pads, input_pads;
  gboolean wait_data[2];
  gboolean wait_output, wait_input;
  guint i;
  
  type_name = xmlGetProp(node,
			 AGS_FILE_TYPE_PROP);

  if(*machine != NULL){
    gobject = *machine;
  }else{
    gobject = g_object_new(g_type_from_name(type_name),
			   NULL);
    
    *machine = gobject;
  }

  if(gobject == NULL ||
     !AGS_IS_MACHINE(gobject)){
    return;
  }
  
  ags_simple_file_add_id_ref(simple_file,
			     g_object_new(AGS_TYPE_FILE_ID_REF,
					  "application-context", simple_file->application_context,
					  "file", simple_file,
					  "node", node,
					  "reference", gobject,
					  NULL));
  
  /* retrieve window */  
  window = AGS_XORG_APPLICATION_CONTEXT(simple_file->application_context)->window;

  config = ags_config_get_instance();
  
  /* find soundcard */
  soundcard = NULL;
  device = xmlGetProp(node,
		      "soundcard-device");

  if(device != NULL){
    list = AGS_XORG_APPLICATION_CONTEXT(simple_file->application_context)->soundcard;

    for(i = 0; list != NULL; i++){
      str = ags_soundcard_get_device(AGS_SOUNDCARD(list->data));
      
      if(str != NULL){
	if(!g_ascii_strcasecmp(str,
			       device)){
	  soundcard = list->data;
	  break;
	}
      }

      /* iterate soundcard */
      list = list->next;
    }
  }
  
  if(soundcard == NULL &&
     AGS_XORG_APPLICATION_CONTEXT(simple_file->application_context)->soundcard != NULL){
    soundcard = AGS_XORG_APPLICATION_CONTEXT(simple_file->application_context)->soundcard->data;
  }
  
  g_object_set(gobject->audio,
	       "output-soundcard", soundcard,
	       NULL);

  /* machine specific */
  if(AGS_IS_LADSPA_BRIDGE(gobject)){
    xmlChar *filename, *effect;

    filename = xmlGetProp(node,
			  "plugin-file");

    effect = xmlGetProp(node,
			"effect");

    g_object_set(gobject,
		 "filename", filename,
		 "effect", effect,
		 NULL);
  }else if(AGS_IS_DSSI_BRIDGE(gobject)){
    xmlChar *filename, *effect;

    filename = xmlGetProp(node,
			  "plugin-file");

    effect = xmlGetProp(node,
			"effect");

    g_object_set(gobject,
		 "filename", filename,
		 "effect", effect,
		 NULL);
  }else if(AGS_IS_LIVE_DSSI_BRIDGE(gobject)){
    xmlChar *filename, *effect;

    filename = xmlGetProp(node,
			  "plugin-file");

    effect = xmlGetProp(node,
			"effect");

    g_object_set(gobject,
		 "filename", filename,
		 "effect", effect,
		 NULL);
  }else if(AGS_IS_LV2_BRIDGE(gobject)){
    AgsLv2Plugin *lv2_plugin;

    xmlChar *filename, *effect;

    filename = xmlGetProp(node,
			  "plugin-file");

    effect = xmlGetProp(node,
			"effect");
    
    lv2_plugin = ags_lv2_manager_find_lv2_plugin(ags_lv2_manager_get_instance(),
						 filename, effect);
  
    if(lv2_plugin != NULL &&
       (AGS_LV2_PLUGIN_IS_SYNTHESIZER & (lv2_plugin->flags)) != 0){
      
      ags_audio_set_flags(gobject->audio, (AGS_AUDIO_OUTPUT_HAS_RECYCLING |
					   AGS_AUDIO_INPUT_HAS_RECYCLING |
					   AGS_AUDIO_SYNC |
					   AGS_AUDIO_ASYNC));
      ags_audio_set_ability_flags(gobject->audio, (AGS_SOUND_ABILITY_NOTATION));
      ags_audio_set_behaviour_flags(gobject->audio, (AGS_SOUND_BEHAVIOUR_DEFAULTS_TO_INPUT));
      
      gobject->flags |= (AGS_MACHINE_IS_SYNTHESIZER |
			 AGS_MACHINE_REVERSE_NOTATION);

      ags_machine_popup_add_connection_options((AgsMachine *) gobject,
					       (AGS_MACHINE_POPUP_MIDI_DIALOG));

      ags_machine_popup_add_edit_options((AgsMachine *) gobject,
					 (AGS_MACHINE_POPUP_ENVELOPE));
    }

    g_object_set(gobject,
		 "filename", filename,
		 "effect", effect,
		 NULL);
  }else if(AGS_IS_LIVE_LV2_BRIDGE(gobject)){
    AgsLv2Plugin *lv2_plugin;

    xmlChar *filename, *effect;

    filename = xmlGetProp(node,
			  "plugin-file");

    effect = xmlGetProp(node,
			"effect");
    
    lv2_plugin = ags_lv2_manager_find_lv2_plugin(ags_lv2_manager_get_instance(),
						 filename, effect);
  
    if(lv2_plugin != NULL &&
       (AGS_LV2_PLUGIN_IS_SYNTHESIZER & (lv2_plugin->flags)) != 0){
      
      ags_audio_set_flags(gobject->audio, (AGS_AUDIO_OUTPUT_HAS_RECYCLING |
					   AGS_AUDIO_INPUT_HAS_RECYCLING |
					   AGS_AUDIO_SYNC |
					   AGS_AUDIO_ASYNC));
      ags_audio_set_ability_flags(gobject->audio, (AGS_SOUND_ABILITY_NOTATION));
      ags_audio_set_behaviour_flags(gobject->audio, (AGS_SOUND_BEHAVIOUR_DEFAULTS_TO_INPUT));
      gobject->flags |= (AGS_MACHINE_IS_SYNTHESIZER |
			 AGS_MACHINE_REVERSE_NOTATION);

      ags_machine_popup_add_connection_options((AgsMachine *) gobject,
					       (AGS_MACHINE_POPUP_MIDI_DIALOG));

      ags_machine_popup_add_edit_options((AgsMachine *) gobject,
					 (AGS_MACHINE_POPUP_ENVELOPE));
    }

    g_object_set(gobject,
		 "filename", filename,
		 "effect", effect,
		 NULL);
  }
  
  gtk_box_pack_start((GtkBox *) window->machines,
		     GTK_WIDGET(gobject),
		     FALSE, FALSE,
		     0);

  /* set name if available */
  str = xmlGetProp(node,
		   "name");

  if(str != NULL){
    g_object_set(gobject,
		 "machine-name", str,
		 NULL);
  }

  /* reverse mapping */
  str = xmlGetProp(node,
		   "reverse-mapping");
  if(!g_strcmp0(str,
		"true")){
    ags_audio_set_behaviour_flags(gobject->audio, (AGS_SOUND_BEHAVIOUR_REVERSE_MAPPING));
  }
  
  /* connect AgsMachine */
  ags_connectable_connect(AGS_CONNECTABLE(gobject));
  
  /* retrieve channel allocation */
  output_pads = gobject->audio->output_pads;
  input_pads = gobject->audio->input_pads;

  audio_channels = gobject->audio->audio_channels;
  
  wait_output = FALSE;
  wait_input = FALSE;
  
  wait_data[0] = FALSE;
  wait_data[1] = FALSE;

  g_signal_connect_after(gobject, "resize-pads", 
			 G_CALLBACK(ags_simple_file_read_machine_resize_pads), wait_data);
  
  str = xmlGetProp(node,
		   "channels");

  if(str != NULL){
    audio_channels = g_ascii_strtoull(str,
				      NULL,
				      10);
    gobject->audio->audio_channels = audio_channels;
  }

  str = xmlGetProp(node,
		   "input-pads");

  if(str != NULL){
    input_pads = g_ascii_strtoull(str,
				  NULL,
				  10);

    ags_audio_set_pads(gobject->audio,
		       AGS_TYPE_INPUT,
		       input_pads, 0);
    wait_input = TRUE;
  }

  str = xmlGetProp(node,
		   "output-pads");

  if(str != NULL){
    output_pads = g_ascii_strtoull(str,
				   NULL,
				   10);
    
    ags_audio_set_pads(gobject->audio,
		       AGS_TYPE_OUTPUT,
		       output_pads, 0);
    wait_output = TRUE;
  }

  /* dispatch */
  while((wait_output && !wait_data[0]) ||
	(wait_input && !wait_data[1])){
    usleep(USEC_PER_SEC / 30);
    g_main_context_iteration(NULL,
			     FALSE);
  }

  g_object_disconnect(gobject,
		      "any_signal::resize-pads", 
		      G_CALLBACK(ags_simple_file_read_machine_resize_pads),
		      wait_data,
		      NULL);

  /* children */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-pad-list",
		     16)){
	GList *pad;

	pad = NULL;
	ags_simple_file_read_pad_list(simple_file,
				      child,
				      &pad);

	g_list_free(pad);
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-effect-pad-list",
			   23)){
	GList *effect_pad;

	effect_pad = NULL;
	ags_simple_file_read_effect_pad_list(simple_file,
					     child,
					     &effect_pad);

	g_list_free(effect_pad);
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-effect-list",
			   19)){
	AgsEffectBulk *effect_bulk;
	
	xmlNode *effect_list_child;

	xmlChar *filename, *effect;
	
	gboolean is_output;

	if(AGS_IS_LADSPA_BRIDGE(gobject) ||
	   AGS_IS_DSSI_BRIDGE(gobject) ||
	   AGS_IS_LIVE_DSSI_BRIDGE(gobject) ||
	   AGS_IS_LV2_BRIDGE(gobject) ||
	   AGS_IS_LIVE_LV2_BRIDGE(gobject)){
	  child = child->next;
	  
	  continue;
	}
	
	is_output = TRUE;
	str = xmlGetProp(child,
			 "is-output");

	if(str != NULL){
	  if(!g_strcmp0(str,
			"false")){
	    is_output = FALSE;
	  }
	}

	if(is_output){
	  effect_bulk = (AgsEffectBulk *) AGS_EFFECT_BRIDGE(gobject->bridge)->bulk_output;
	}else{
	  effect_bulk = (AgsEffectBulk *) AGS_EFFECT_BRIDGE(gobject->bridge)->bulk_input;
	}

	filename = xmlGetProp(child,
			      "filename");
	effect = xmlGetProp(child,
			    "effect");

	ags_effect_bulk_add_effect(effect_bulk,
				   NULL,
				   filename,
				   effect);
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-oscillator-list",
			   19)){
	if(AGS_IS_SYNCSYNTH(gobject)){
	  GList *oscillator, *list;

	  guint count;
	  guint i;
	  
	  list = gtk_container_get_children(AGS_SYNCSYNTH(gobject)->oscillator);
	  count = g_list_length(list);

	  for(i = 0; i < count; i++){
	    ags_syncsynth_remove_oscillator(gobject,
					    0);
	  }

	  g_list_free(list);
	  
	  oscillator = NULL;
	  
	  ags_simple_file_read_oscillator_list(simple_file,
					       child,
					       &oscillator);

	  list = g_list_reverse(oscillator);
	  
	  while(list != NULL){
	    ags_syncsynth_add_oscillator(gobject,
					 list->data);

	    list = list->next;
	  }
	  
	  g_list_free(oscillator);
	}
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-pattern-list",
			   20)){
	xmlNode *pattern_list_child;
	
	
  	/* pattern list children */
	pattern_list_child = child->children;

	while(pattern_list_child != NULL){
	  if(pattern_list_child->type == XML_ELEMENT_NODE){	    
	    if(!xmlStrncmp(pattern_list_child->name,
			   (xmlChar *) "ags-sf-pattern",
			   15)){
	      AgsChannel *channel;
	      AgsPattern *pattern;

	      GList *list;

	      xmlChar *content;
	      
	      guint line;
	      guint bank_0, bank_1;
	      guint i;
	      
	      /* retrieve channel */
	      line = 0;
	      str = xmlGetProp(pattern_list_child,
			       "nth-line");

	      if(str != NULL){
		line = g_ascii_strtoull(str,
				       NULL,
				       10);
	      }
	      
	      channel = ags_channel_nth(gobject->audio->input,
					line);

	      /* retrieve bank */
	      bank_0 =
		bank_1 = 0;

	      str = xmlGetProp(pattern_list_child,
			       "bank-0");

	      if(str != NULL){
		bank_0 = g_ascii_strtoull(str,
					  NULL,
					  10);
	      }

	      str = xmlGetProp(pattern_list_child,
			       "bank-1");

	      if(str != NULL){
		bank_1 = g_ascii_strtoull(str,
					  NULL,
					  10);
	      }
	      
	      /* toggle pattern */
	      pattern = channel->pattern->data;

	      content = xmlNodeGetContent(pattern_list_child);
	      i = 0;

	      if(content != NULL){
		for(i = 0; i < pattern->dim[2]; i++){
		  if(content[i] == '\0'){
		    break;
		  }
		  
		  if(content[i] == '1'){
		    ags_pattern_toggle_bit(pattern,
					   bank_0,
					   bank_1,
					   i);
		  }
		}
	      }
	    }
	  }
	  
	  pattern_list_child = pattern_list_child->next;
	}
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-notation-list",
			   21)){
	gchar *version;

	guint major, minor;
	
	version = xmlGetProp(simple_file->root_node,
			     "version");
	major = 0;
	minor = 0;

	if(version != NULL){
	  sscanf(version, "%d.%d",
		 &major,
		 &minor);
	}
	
	if(major == 0 ||
	   (major == 1 && minor < 2)){
	  ags_simple_file_read_notation_list_fixup_1_0_to_1_2(simple_file,
							      child,
							      &(gobject->audio->notation));
	}else{
	  ags_simple_file_read_notation_list(simple_file,
					     child,
					     &(gobject->audio->notation));
	}
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-preset-list",
			   21)){
	GList *preset;

	preset = NULL;
	ags_simple_file_read_preset_list(simple_file,
					 child,
					 &preset);

	g_list_free_full(gobject->audio->preset,
			 g_object_unref);

	gobject->audio->preset = preset;
      }
    }

    child = child->next;
  }

  if(AGS_IS_LADSPA_BRIDGE(gobject)){
    ags_ladspa_bridge_load((AgsLadspaBridge *) gobject);
  }else if(AGS_IS_DSSI_BRIDGE(gobject)){
    ags_dssi_bridge_load((AgsDssiBridge *) gobject);
  }else if(AGS_IS_LIVE_DSSI_BRIDGE(gobject)){
    ags_live_dssi_bridge_load((AgsLiveDssiBridge *) gobject);
  }else if(AGS_IS_LV2_BRIDGE(gobject)){
    ags_lv2_bridge_load((AgsLv2Bridge *) gobject);
  }else if(AGS_IS_LIVE_LV2_BRIDGE(gobject)){
    ags_live_lv2_bridge_load((AgsLiveLv2Bridge *) gobject);
  }

  /* retrieve midi mapping */
  str = xmlGetProp(node,
		   "audio-start-mapping");

  if(str != NULL){
    g_object_set(gobject->audio,
		 "audio-start-mapping", g_ascii_strtoull(str,
							   NULL,
							   10),
		 NULL);
  }

  str = xmlGetProp(node,
		   "audio-end-mapping");

  if(str != NULL){
    g_object_set(gobject->audio,
		 "audio-end-mapping", g_ascii_strtoull(str,
							 NULL,
							 10),
		 NULL);
  }

  str = xmlGetProp(node,
		   "midi-start-mapping");

  if(str != NULL){
    g_object_set(gobject->audio,
		 "midi-start-mapping", g_ascii_strtoull(str,
							  NULL,
							  10),
		 NULL);
  }

  str = xmlGetProp(node,
		   "midi-end-mapping");

  if(str != NULL){
    g_object_set(gobject->audio,
		 "midi-end-mapping", g_ascii_strtoull(str,
							NULL,
							10),
		 NULL);
  }

  gtk_widget_show_all((GtkWidget *) gobject);

  /* add audio to soundcard */
  list = ags_sound_provider_get_audio(AGS_SOUND_PROVIDER(simple_file->application_context));
  g_object_ref(G_OBJECT(gobject->audio));
  
  list = g_list_prepend(list,
			gobject->audio);
  ags_sound_provider_set_audio(AGS_SOUND_PROVIDER(simple_file->application_context),
			       list);
  
  /* children */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-automation-list",
			   23)){
	GList *automation;

	gchar *version;

	guint major, minor;
	
	version = xmlGetProp(simple_file->root_node,
			     "version");
	major = 0;
	minor = 0;

	if(version != NULL){
	  sscanf(version, "%d.%d",
		 &major,
		 &minor);
	}
	
	if(major == 0 ||
	   (major == 1 && minor < 3)){
	  ags_simple_file_read_automation_list_fixup_1_0_to_1_3(simple_file,
								child,
								&(gobject->audio->automation));
	}else{
	  ags_simple_file_read_automation_list(simple_file,
					       child,
					       &(gobject->audio->automation));
	}
      }
    }

    child = child->next;
  }
  
  /* launch AgsMachine */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "file", simple_file,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_simple_file_read_machine_launch), gobject);
  ags_simple_file_add_launch(simple_file,
			     (GObject *) file_launch);
}

void
ags_simple_file_read_machine_launch(AgsFileLaunch *file_launch,
				    AgsMachine *machine)
{
  xmlNode *child;

  xmlChar *str;

  auto void ags_simple_file_read_equalizer10_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsEqualizer10 *equalizer10);
  auto void ags_simple_file_read_drum_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsDrum *drum);
  auto void ags_simple_file_read_matrix_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsMatrix *matrix);
  auto void ags_simple_file_read_syncsynth_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsSyncsynth *syncsynth);

#ifdef AGS_WITH_LIBINSTPATCH
  auto void ags_simple_file_read_ffplayer_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsFFPlayer *ffplayer);
#endif
  
  auto void ags_simple_file_read_audiorec_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsAudiorec *audiorec);

 auto void ags_simple_file_read_dssi_bridge_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsDssiBridge *dssi_bridge);
  auto void ags_simple_file_read_live_dssi_bridge_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsLiveDssiBridge *live_dssi_bridge);
  auto void ags_simple_file_read_lv2_bridge_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsLv2Bridge *lv2_bridge);
  auto void ags_simple_file_read_live_lv2_bridge_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsLiveLv2Bridge *live_lv2_bridge);

  auto void ags_simple_file_read_effect_bridge_launch(AgsSimpleFile *simple_file, xmlNode *node, AgsEffectBridge *effect_bridge);
  auto void ags_simple_file_read_effect_bulk_launch(AgsSimpleFile *simple_file, xmlNode *node, AgsEffectBulk *effect_bulk);
  auto void ags_simple_file_read_bulk_member_launch(AgsSimpleFile *simple_file, xmlNode *node, AgsBulkMember *bulk_member);

  void ags_simple_file_read_equalizer10_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsEqualizer10 *equalizer10)
  {
    xmlNode *child;

    child = node->children;

    while(child != NULL){
      if(child->type == XML_ELEMENT_NODE){
	if(!xmlStrncmp(child->name,
		       "ags-sf-equalizer10",
		       19)){
	  xmlNode *control_node;

	  control_node = child->children;

	  while(control_node != NULL){
	    if(control_node->type == XML_ELEMENT_NODE){
	      if(!xmlStrncmp(control_node->name,
			     "ags-sf-control",
			     15)){
		xmlChar *specifier;
		xmlChar *value;
		
		gdouble val;
		
		specifier = xmlGetProp(control_node,
				       "specifier");

		value = xmlGetProp(control_node,
				   "value");

		val = 0.0;
		
		if(value != NULL){
		  val = g_strtod(value,
				 NULL);

		  if(!xmlStrncmp(specifier,
				 "28 [Hz]",
				 8)){
		    gtk_range_set_value(equalizer10->peak_28hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "56 [Hz]",
				       8)){
		    gtk_range_set_value(equalizer10->peak_56hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "112 [Hz]",
				       9)){
		    gtk_range_set_value(equalizer10->peak_112hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "224 [Hz]",
				       9)){
		    gtk_range_set_value(equalizer10->peak_224hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "448 [Hz]",
				       9)){
		    gtk_range_set_value(equalizer10->peak_448hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "896 [Hz]",
				       9)){
		    gtk_range_set_value(equalizer10->peak_896hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "1792 [Hz]",
				       10)){
		    gtk_range_set_value(equalizer10->peak_1792hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "1792 [Hz]",
				       10)){
		    gtk_range_set_value(equalizer10->peak_1792hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "3584 [Hz]",
				       10)){
		    gtk_range_set_value(equalizer10->peak_3584hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "7168 [Hz]",
				       10)){
		    gtk_range_set_value(equalizer10->peak_7168hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "14336 [Hz]",
				       11)){
		    gtk_range_set_value(equalizer10->peak_14336hz,
					val);
		  }else if(!xmlStrncmp(specifier,
				       "pressure",
				       9)){
		    gtk_range_set_value(equalizer10->pressure,
					val);
		  }
		}	
	      }
	    }
	    
	    control_node = control_node->next;
	  }
	  
	  break;
	}
      }

      child = child->next;
    }
  }
  
  void ags_simple_file_read_drum_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsDrum *drum){
    xmlChar *str;
    
    guint bank_0, bank_1;

    /* bank 0 */
    bank_0 = 0;
    str = xmlGetProp(node,
		     "bank-0");

    if(str != NULL){      
      bank_0 = g_ascii_strtod(str,
			      NULL);

      if(bank_0 < 4){
	gtk_toggle_button_set_active(drum->index0[bank_0],
				     TRUE);
      }
    }

    /* bank 1 */
    bank_1 = 0;
    str = xmlGetProp(node,
		     "bank-1");

    if(str != NULL){      
      bank_1 = g_ascii_strtod(str,
			      NULL);

      if(bank_1 < 12){
	gtk_toggle_button_set_active(drum->index1[bank_1],
				     TRUE);
      }
    }

    /* loop */
    str = xmlGetProp(node,
		     "loop");
    
    if(str != NULL &&
       !g_strcmp0(str,
		  "true")){
      gtk_toggle_button_set_active((GtkToggleButton *) drum->loop_button,
				   TRUE);
    }

    /* length */
    str = xmlGetProp(node,
		     "length");

    if(str != NULL){
      guint length;

      length = g_ascii_strtoull(str,
				NULL,
				10);
      
      gtk_adjustment_set_value(drum->length_spin->adjustment,
			       (gdouble) length);
    }
  }
  
  void ags_simple_file_read_matrix_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsMatrix *matrix){
    xmlChar *str;
    
    guint bank_0, bank_1;
    
    /* bank 1 */
    bank_1 = 0;
    str = xmlGetProp(node,
		     "bank-1");

    if(str != NULL){      
      bank_1 = g_ascii_strtod(str,
			      NULL);

      if(bank_1 < 9){
	gtk_toggle_button_set_active(matrix->index[bank_1],
				     TRUE);
      }
    }

    /* loop */
    str = xmlGetProp(node,
		     "loop");
    
    if(str != NULL &&
       !g_strcmp0(str,
		  "true")){
      gtk_toggle_button_set_active((GtkToggleButton *) matrix->loop_button,
				   TRUE);
    }

    /* length */
    str = xmlGetProp(node,
		     "length");

    if(str != NULL){
      guint length;

      length = g_ascii_strtoull(str,
				NULL,
				10);
      
      gtk_adjustment_set_value(matrix->length_spin->adjustment,
			       (gdouble) length);
    }
  }
  void ags_simple_file_read_synth_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsSynth *synth){
    xmlChar *str;
    
    /* base note */
    str = xmlGetProp(node,
		     "base-note");

    if(str != NULL){
      gdouble base_note;

      base_note = g_ascii_strtod(str,
				 NULL);

      if(base_note > AGS_SYNTH_BASE_NOTE_MIN &&
	 base_note < AGS_SYNTH_BASE_NOTE_MAX){
	gtk_adjustment_set_value(synth->lower->adjustment,
				 (gdouble) base_note);
      }
    }
  }
  
  void ags_simple_file_read_syncsynth_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsSyncsynth *syncsynth){
    GList *list, *list_start;
    GList *child_start;

    xmlChar *str;

    gdouble tmp0, tmp1;
    gdouble loop_upper;
    
    /* base note */
    str = xmlGetProp(node,
		     "base-note");

    if(str != NULL){
      gdouble base_note;

      base_note = g_ascii_strtod(str,
				 NULL);

      if(base_note > AGS_SYNCSYNTH_BASE_NOTE_MIN &&
	 base_note < AGS_SYNCSYNTH_BASE_NOTE_MAX){
	gtk_adjustment_set_value(syncsynth->lower->adjustment,
				 (gdouble) base_note);
      }
    }

    /* set range of loop start and loop end */
    ags_syncsynth_reset_loop(syncsynth);
    
    /* audio loop start */
    str = xmlGetProp(node,
		     "audio-loop-start");

    if(str != NULL){
      guint audio_loop_start;

      audio_loop_start = g_ascii_strtoull(str,
					  NULL,
					  10);
      
      gtk_adjustment_set_value(syncsynth->loop_start->adjustment,
			       (gdouble) audio_loop_start);
    }

    /* audio loop end */
    str = xmlGetProp(node,
		     "audio-loop-end");

    if(str != NULL){
      guint audio_loop_end;

      audio_loop_end = g_ascii_strtoull(str,
					NULL,
					10);
      
      gtk_adjustment_set_value(syncsynth->loop_end->adjustment,
			       (gdouble) audio_loop_end);
    }
  }

#ifdef AGS_WITH_LIBINSTPATCH
  void ags_simple_file_read_ffplayer_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsFFPlayer *ffplayer){
    GtkTreeModel *model;
    GtkTreeIter iter;

    xmlChar *str;
    gchar *value;

    str = xmlGetProp(node,
		     "filename");
    
    ags_ffplayer_open_filename(ffplayer,
			       str);

    if(ffplayer->audio_container == NULL){
      return;
    }

    /* preset */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(ffplayer->preset));

    str = xmlGetProp(node,
		     "preset");

    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) ffplayer->preset,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
    
    /* instrument */
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(ffplayer->instrument));

    str = xmlGetProp(node,
		     "instrument");

    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) ffplayer->instrument,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
  }
#endif
  
  void ags_simple_file_read_audiorec_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsAudiorec *audiorec)
  {
    xmlChar *str;
    gchar *value;

    str = xmlGetProp(node,
		     "filename");
    
    ags_audiorec_open_filename(audiorec,
			       str);
  }
  
  void ags_simple_file_read_dssi_bridge_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsDssiBridge *dssi_bridge){
    GtkTreeModel *model;
    GtkTreeIter iter;

    xmlChar *str;
    gchar *value;

    /* program */
    //NOTE:JK: work-around
    gtk_combo_box_set_active((GtkComboBox *) dssi_bridge->program,
			     0);

    model = gtk_combo_box_get_model((GtkComboBox *) dssi_bridge->program);

    str = xmlGetProp(node,
		     "program");

    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) dssi_bridge->program,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
  }

  void ags_simple_file_read_live_dssi_bridge_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsLiveDssiBridge *live_dssi_bridge){
    GtkTreeModel *model;
    GtkTreeIter iter;

    xmlChar *str;
    gchar *value;

    /* program */
    //NOTE:JK: work-around
    gtk_combo_box_set_active((GtkComboBox *) live_dssi_bridge->program,
			     0);

    model = gtk_combo_box_get_model((GtkComboBox *) live_dssi_bridge->program);

    str = xmlGetProp(node,
		     "program");

    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) live_dssi_bridge->program,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
  }

  void ags_simple_file_read_lv2_bridge_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsLv2Bridge *lv2_bridge){
    GtkTreeModel *model;
    GtkTreeIter iter;

    xmlChar *str;
    gchar *value;

    if(lv2_bridge->preset == NULL){
      return;
    }
    
    /* program */
    //NOTE:JK: work-around
    gtk_combo_box_set_active((GtkComboBox *) lv2_bridge->preset,
			     0);

    model = gtk_combo_box_get_model((GtkComboBox *) lv2_bridge->preset);

    str = xmlGetProp(node,
		     "preset");

    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) lv2_bridge->preset,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
  }

  void ags_simple_file_read_live_lv2_bridge_launch(AgsSimpleFile *simpleFile, xmlNode *node, AgsLiveLv2Bridge *live_lv2_bridge){
    GtkTreeModel *model;
    GtkTreeIter iter;

    xmlChar *str;
    gchar *value;

    if(live_lv2_bridge->preset == NULL){
      return;
    }
    
    /* program */
    //NOTE:JK: work-around
    gtk_combo_box_set_active((GtkComboBox *) live_lv2_bridge->preset,
			     0);

    model = gtk_combo_box_get_model((GtkComboBox *) live_lv2_bridge->preset);

    str = xmlGetProp(node,
		     "preset");

    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) live_lv2_bridge->preset,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
  }

  void ags_simple_file_read_effect_bridge_launch(AgsSimpleFile *simple_file, xmlNode *node, AgsEffectBridge *effect_bridge){
    AgsEffectBulk *effect_bulk;
    
    xmlNode *child;

    xmlChar *str;
    
    gboolean is_output;
	
    is_output = TRUE;
    str = xmlGetProp(node,
		     "is-output");
    
    if(str != NULL &&
       !g_strcmp0(str,
		  "false")){
      is_output = FALSE;
    }

    if(is_output){
      effect_bulk = (AgsEffectBulk *) AGS_EFFECT_BRIDGE(machine->bridge)->bulk_output;
    }else{
      effect_bulk = (AgsEffectBulk *) AGS_EFFECT_BRIDGE(machine->bridge)->bulk_input;
    }
	
    /* effect list children */
    child = node->children;
	
    while(child != NULL){
      if(child->type == XML_ELEMENT_NODE){
	if(!xmlStrncmp(child->name,
		       (xmlChar *) "ags-sf-effect",
		       14)){
	  ags_simple_file_read_effect_bulk_launch(simple_file, child, effect_bulk);
	}
      }

      child = child->next;
    }
  }
  
  void ags_simple_file_read_effect_bulk_launch(AgsSimpleFile *simple_file, xmlNode *node, AgsEffectBulk *effect_bulk){
    xmlNode *child;
    
    xmlChar *filename, *effect;

    filename = xmlGetProp(node,
			  "filename");
	      
    effect = xmlGetProp(node,
			"effect");

    /* effect list children */
    child = node->children;

    while(child != NULL){
      if(child->type == XML_ELEMENT_NODE){
	if(!xmlStrncmp(child->name,
		       (xmlChar *) "ags-sf-control",
		       15)){
	  GList *list_start, *list;
		    
	  xmlChar *specifier;

	  specifier = xmlGetProp(child,
				 "specifier");
	  
	  list_start = gtk_container_get_children(GTK_CONTAINER(effect_bulk->table));
	  list = list_start;

	  while(list != NULL){
	    if(AGS_IS_BULK_MEMBER(list->data)){
	      AgsBulkMember *bulk_member;
	      
	      bulk_member = AGS_BULK_MEMBER(list->data);

	      if(!g_strcmp0(bulk_member->filename,
			    filename) &&
		 !g_strcmp0(bulk_member->effect,
			    effect) &&
		 !g_strcmp0(bulk_member->specifier,
			    specifier)){
		ags_simple_file_read_bulk_member_launch(simple_file, child, bulk_member);
		break;
	      }
	    }

	    list = list->next;
	  }

	  g_list_free(list_start);
	}
      }

      child = child->next;
    }
  }
	
  void ags_simple_file_read_bulk_member_launch(AgsSimpleFile *simple_file, xmlNode *node, AgsBulkMember *bulk_member)
  {
    GtkWidget *child_widget;

    xmlChar *str;
    
    gdouble val;

    str = xmlGetProp(node,
		     "value");

    child_widget = gtk_bin_get_child((GtkBin *) bulk_member);

    if(GTK_IS_RANGE(child_widget)){    
      if(str != NULL){
	val = g_ascii_strtod(str,
			     NULL);
		      
	gtk_adjustment_set_value(GTK_RANGE(child_widget)->adjustment,
				 val);
      }      
    }else if(GTK_IS_SPIN_BUTTON(child_widget)){
      if(str != NULL){
	val = g_ascii_strtod(str,
			     NULL);
		      
	gtk_adjustment_set_value(GTK_SPIN_BUTTON(child_widget)->adjustment,
				 val);
      }
    }else if(AGS_IS_DIAL(child_widget)){
      if(str != NULL){
	val = g_ascii_strtod(str,
			     NULL);
		      
	gtk_adjustment_set_value(AGS_DIAL(child_widget)->adjustment,
				 val);
	ags_dial_draw((AgsDial *) child_widget);
      }
    }else if(GTK_IS_TOGGLE_BUTTON(child_widget)){
      if(str != NULL){
	gtk_toggle_button_set_active((GtkToggleButton *) child_widget,
				     ((!g_ascii_strncasecmp(str, "true", 5)) ? TRUE: FALSE));
      }
    }else{
      g_warning("ags_simple_file_read_bulk_member_launch() - unknown bulk member type");
    }			  
  }  
  
  if(AGS_IS_EQUALIZER10(machine)){
    ags_simple_file_read_equalizer10_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsEqualizer10 *) machine);
  }else if(AGS_IS_DRUM(machine)){
    ags_simple_file_read_drum_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsDrum *) machine);
  }else if(AGS_IS_MATRIX(machine)){
    ags_simple_file_read_matrix_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsMatrix *) machine);
  }else if(AGS_IS_SYNTH(machine)){
    ags_simple_file_read_synth_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsSynth *) machine);
  }else if(AGS_IS_SYNCSYNTH(machine)){
    ags_simple_file_read_syncsynth_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsSyncsynth *) machine);
#ifdef AGS_WITH_LIBINSTPATCH
  }else if(AGS_IS_FFPLAYER(machine)){
    ags_simple_file_read_ffplayer_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsFFPlayer *) machine);
#endif
  }else if(AGS_IS_AUDIOREC(machine)){
    ags_simple_file_read_audiorec_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsAudiorec *) machine);
  }else if(AGS_IS_DSSI_BRIDGE(machine)){
    ags_simple_file_read_dssi_bridge_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsDssiBridge *) machine);
  }else if(AGS_IS_DSSI_BRIDGE(machine)){
    ags_simple_file_read_live_dssi_bridge_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsDssiBridge *) machine);
  }else if(AGS_IS_LV2_BRIDGE(machine)){
    ags_simple_file_read_lv2_bridge_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsLv2Bridge *) machine);
  }else if(AGS_IS_LV2_BRIDGE(machine)){
    ags_simple_file_read_live_lv2_bridge_launch((AgsSimpleFile *) file_launch->file, file_launch->node, (AgsLv2Bridge *) machine);
  }
  
  /* children */
  child = file_launch->node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-property-list",
		     21)){
	GList *property_start, *property;

	property_start = NULL;
	ags_simple_file_read_property_list((AgsSimpleFile *) file_launch->file,
					   child,
					   &property_start);

	property = property_start;

	while(property != NULL){
	  g_object_set_property((GObject *) machine,
				((GParameter *) property->data)->name,
				&(((GParameter *) property->data)->value));
	  
	  property = property->next;
	}
	
	g_list_free_full(property_start,
			 g_free);
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-effect-list",
			   19)){
	ags_simple_file_read_effect_bridge_launch((AgsSimpleFile *) file_launch->file, child, (AgsEffectBridge *) machine->bridge);
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_pad_list(AgsSimpleFile *simple_file, xmlNode *node, GList **pad)
{
  AgsPad *current;
  
  xmlNode *child;

  GList *list;

  guint i;
  
  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-pad",
		     11)){
	current = NULL;

	if(*pad != NULL){
	  GList *iter;

	  iter = g_list_nth(*pad,
			    i);

	  if(iter != NULL){
	    current = iter->data;
	  }
	}
	
	ags_simple_file_read_pad(simple_file, child, &current);
	list = g_list_prepend(list, current);

	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *pad = list;
}

void
ags_simple_file_read_pad(AgsSimpleFile *simple_file, xmlNode *node, AgsPad **pad)
{
  AgsMachine *machine;
  AgsPad *gobject;
  
  AgsFileLaunch *file_launch;
  AgsFileIdRef *file_id_ref;
  
  xmlNode *child;

  GList *list, *list_start;
    
  xmlChar *str;

  guint nth_pad;
  
  if(pad != NULL &&
     pad[0] != NULL){
    gobject = pad[0];

    nth_pad = gobject->channel->pad;
  }else{
    file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_node(simple_file,
								       node->parent->parent);
    machine = file_id_ref->ref;
    
    if(!AGS_IS_MACHINE(machine)){
      return;
    }
    
    /* get nth pad */
    nth_pad = 0;
    str = xmlGetProp(node,
		     "nth-pad");
    
    if(str != NULL){
      nth_pad = g_ascii_strtoull(str,
				 NULL,
				 10);
    }

    /* retrieve pad */
    gobject = NULL;
    str = xmlGetProp(node->parent,
		     "is-output");

    list_start = NULL;
    
    if(!g_strcmp0(str,
		  "false")){
      if(machine->input != NULL){
	list_start = gtk_container_get_children((GtkContainer *) machine->input);
      }
    }else{
      if(machine->output != NULL){
	list_start = gtk_container_get_children((GtkContainer *) machine->output);
      }
    }

    list = g_list_nth(list_start,
		      nth_pad);

    if(list != NULL){
      gobject = AGS_PAD(list->data);
    }

    g_list_free(list_start);
  }
  
  ags_simple_file_add_id_ref(simple_file,
			     g_object_new(AGS_TYPE_FILE_ID_REF,
					  "application-context", simple_file->application_context,
					  "file", simple_file,
					  "node", node,
					  "reference", gobject,
					  NULL));

  /* children */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-line-list",
		     17)){
	GList *line;

	line = NULL;
	ags_simple_file_read_line_list(simple_file,
				       child,
				       &line);

	g_list_free(line);
      }
    }

    child = child->next;
  }

  ags_connectable_connect(AGS_CONNECTABLE(gobject));

  /* launch AgsPad */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_simple_file_read_pad_launch), gobject);
  ags_simple_file_add_launch(simple_file,
			     (GObject *) file_launch);
}

void
ags_simple_file_read_pad_launch(AgsFileLaunch *file_launch,
				AgsPad *pad)
{
  xmlNode *child;

  xmlChar *str;
    
  if(AGS_IS_PAD(pad)){
    str = xmlGetProp(file_launch->node,
		     "group");
    
    gtk_toggle_button_set_active(pad->group,
				 g_strcmp0(str,
					   "false"));

    str = xmlGetProp(file_launch->node,
		     "mute");
    
    gtk_toggle_button_set_active(pad->mute,
				 g_strcmp0(str,
					   "false"));

    str = xmlGetProp(file_launch->node,
		     "solo");
    
    gtk_toggle_button_set_active(pad->solo,
				 g_strcmp0(str,
					   "false"));
  }
  
  /* children */
  child = file_launch->node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-property-list",
		     21)){
	GList *property_start, *property;

	property_start = NULL;
	ags_simple_file_read_property_list((AgsSimpleFile *) file_launch->file,
					   child,
					   &property_start);

	property = property_start;

	while(property != NULL){
	  g_object_set_property((GObject *) pad,
				((GParameter *) property->data)->name,
				&(((GParameter *) property->data)->value));
	  
	  property = property->next;
	}
	
	g_list_free_full(property_start,
			 g_free);
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_line_list(AgsSimpleFile *simple_file, xmlNode *node, GList **line)
{
  AgsLine *current;
  GList *list;
  
  xmlNode *child;

  guint i;
  
  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-line",
		     12)){
	current = NULL;

	if(line[0] != NULL){
	  GList *iter;

	  iter = g_list_nth(line[0],
			    i);
	  
	  if(iter != NULL){
	    current = iter->data;
	  }
	}
	
	ags_simple_file_read_line(simple_file, child, &current);
	list = g_list_prepend(list, current);

	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *line = list;
}

void
ags_simple_file_read_line(AgsSimpleFile *simple_file, xmlNode *node, AgsLine **line)
{
  AgsPad *pad;
  GObject *gobject;

  AgsConfig *config;
  GObject *soundcard;
  
  AgsFileLaunch *file_launch;
  AgsFileIdRef *file_id_ref;

  xmlNode *child;

  GList *list;

  xmlChar *device;
  xmlChar *str;
  
  guint nth_line;
  guint i;
  
  auto void ags_simple_file_read_line_member(AgsSimpleFile *simple_file, xmlNode *node, AgsLineMember *line_member);
  
  void ags_simple_file_read_line_member(AgsSimpleFile *simple_file, xmlNode *node, AgsLineMember *line_member){
    GtkWidget *child_widget;

    xmlChar *str;
    
    gdouble val;

    line_member->flags |= AGS_LINE_MEMBER_APPLY_INITIAL;
    
    str = xmlGetProp(node,
		     "control-type");

    if(str != NULL){
      g_object_set(line_member,
		   "widget-type", g_type_from_name(str),
		   NULL);
    }

    child_widget = gtk_bin_get_child((GtkBin *) line_member);
    
    /* apply value */
    str = xmlGetProp(node,
		     "value");

    if(str != NULL){
       if(AGS_IS_DIAL(child_widget)){
	val = g_ascii_strtod(str,
			     NULL);
	gtk_adjustment_set_value(AGS_DIAL(child_widget)->adjustment,
				 val);
      }else if(GTK_IS_RANGE(child_widget)){
	val = g_ascii_strtod(str,
			     NULL);    
      
	gtk_adjustment_set_value(GTK_RANGE(child_widget)->adjustment,
				 val);
      }else if(GTK_IS_SPIN_BUTTON(child_widget)){
	val = g_ascii_strtod(str,
			     NULL);    
      
	gtk_adjustment_set_value(GTK_SPIN_BUTTON(child_widget)->adjustment,
				 val);
      }else if(GTK_IS_TOGGLE_BUTTON(child_widget)){
	 gtk_toggle_button_set_active((GtkToggleButton *) child_widget,
				      ((!g_ascii_strncasecmp(str, "true", 5)) ? TRUE: FALSE));
      }else{
	g_warning("ags_simple_file_read_line() - unknown line member type");
      }
    }
  }
  
  if(line != NULL &&
     line[0] != NULL){
    gobject = AGS_LINE(line[0]);

    nth_line = AGS_LINE(gobject)->channel->line;
  }else{
    GList *list_start, *list;
    
    file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_node(simple_file,
								       node->parent->parent);
    
    if(!AGS_IS_PAD(file_id_ref->ref)){
      pad = NULL;
    }else{
      pad = file_id_ref->ref;
    }
    
    /* get nth-line */
    nth_line = 0;
    str = xmlGetProp(node,
		     "nth-line");

    if(str != NULL){
      nth_line = g_ascii_strtoull(str,
				  NULL,
				  10);
    }

    /* retrieve line or channel */
    gobject = NULL;

    if(pad != NULL){
      list_start = gtk_container_get_children((GtkContainer *) pad->expander_set);

      list = list_start;

      while(list != NULL){
	if(AGS_IS_LINE(list->data) &&
	   AGS_LINE(list->data)->channel->line == nth_line){
	  gobject = list->data;
	  
	  break;
	}

	list = list->next;
      }

      if(list_start != NULL){
	g_list_free(list_start);
      }
    }else{
      AgsMachine *machine;

      gboolean is_output;

      //      "./ancestor::*[self::ags-sf-machine][1]"
	
      file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_node(simple_file,
									 node->parent->parent->parent->parent);
      machine = file_id_ref->ref;
    
      if(!AGS_IS_MACHINE(machine)){
	return;
      }

      is_output = TRUE;
      str = xmlGetProp(node->parent->parent->parent,
		       "is-output");

      if(str != NULL &&
	 !g_strcmp0(str,
		    "false")){
	is_output = FALSE;
      }

      if(is_output){
	gobject = (GObject *) ags_channel_nth(machine->audio->output,
					      nth_line);
      }else{
	gobject = (GObject *) ags_channel_nth(machine->audio->input,
					      nth_line);
      }
    }
  }

  ags_simple_file_add_id_ref(simple_file,
			     g_object_new(AGS_TYPE_FILE_ID_REF,
					  "application-context", simple_file->application_context,
					  "file", simple_file,
					  "node", node,
					  "reference", gobject,
					  NULL));

  /* device */
  config = ags_config_get_instance();
  
  /* find soundcard */
  soundcard = NULL;
  device = xmlGetProp(node,
		      "soundcard-device");  
  if(device != NULL){
    list = AGS_XORG_APPLICATION_CONTEXT(simple_file->application_context)->soundcard;
  
    for(i = 0; list != NULL; i++){
      str = ags_soundcard_get_device(AGS_SOUNDCARD(list->data));
      
      if(str != NULL &&
	 !g_ascii_strcasecmp(str,
			     device)){
	soundcard = list->data;
	break;
      }

      /* iterate soundcard */
      list = list->next;
    }
  }
  
  if(soundcard == NULL &&
     AGS_XORG_APPLICATION_CONTEXT(simple_file->application_context)->soundcard != NULL){
    soundcard = AGS_XORG_APPLICATION_CONTEXT(simple_file->application_context)->soundcard->data;
  }

  if(AGS_IS_LINE(gobject)){
    g_object_set(AGS_LINE(gobject)->channel,
		 "output-soundcard", soundcard,
		 NULL);
  }else if(AGS_IS_CHANNEL(gobject)){
    g_object_set(AGS_CHANNEL(gobject),
		 "output-soundcard", soundcard,
		 NULL);
  }
    
  /* children */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-effect-list",
		     19) &&
	 AGS_IS_LINE(gobject)){
	xmlNode *effect_list_child;

	GList *mapped_filename, *mapped_effect;
	
	/* effect list children */
	effect_list_child = child->children;

	mapped_filename = NULL;
	mapped_effect = NULL;
	
	while(effect_list_child != NULL){
	  if(effect_list_child->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(effect_list_child->name,
			   (xmlChar *) "ags-sf-effect",
			   14)){
	      xmlNode *effect_child;

	      xmlChar *filename, *effect;

	      filename = xmlGetProp(effect_list_child,
				    "filename");
	      
	      effect = xmlGetProp(effect_list_child,
				  "effect");
	      
	      if(filename != NULL &&
		 effect != NULL){
		if(g_list_find_custom(mapped_filename,
				      filename,
				      (GCompareFunc) g_strcmp0) == NULL ||
		   g_list_find_custom(mapped_effect,
				      effect,
				      (GCompareFunc) g_strcmp0) == NULL){
		  mapped_filename = g_list_prepend(mapped_filename,
						   filename);
		  mapped_effect = g_list_prepend(mapped_effect,
						 effect);

		  if(AGS_IS_LINE(gobject)){
		    GList *recall_list;
		    
		    recall_list = ags_channel_add_effect(AGS_LINE(gobject)->channel,
							 filename,
							 effect);
		    g_list_free(recall_list);
		  }else if(AGS_IS_CHANNEL(gobject)){
		    GList *recall_list;

		    recall_list = ags_channel_add_effect((AgsChannel *) gobject,
							 filename,
							 effect);
		    g_list_free(recall_list);
		  }
		}
	      }
	      
	      /* effect list children */
	      effect_child = effect_list_child->children;

	      while(effect_child != NULL){
		if(effect_child->type == XML_ELEMENT_NODE){
		  if(!xmlStrncmp(effect_child->name,
				 (xmlChar *) "ags-sf-control",
				 15)){
		    AgsLineMember *line_member;

		    GList *list_start, *list;
		    
		    xmlChar *specifier;

		    specifier = xmlGetProp(effect_child,
					   "specifier");
		    		       
		    list =
		      list_start = gtk_container_get_children((GtkContainer *) AGS_LINE(gobject)->expander->table);

		    while(list != NULL){
		      if(AGS_IS_LINE_MEMBER(list->data)){
			line_member = AGS_LINE_MEMBER(list->data);
			
			if(((filename == NULL && effect == NULL) ||
			    (strlen(filename) == 0 && strlen(effect) == 0) ||
			     (!g_strcmp0(line_member->filename,
					 filename) &&
			      !g_strcmp0(line_member->effect,
					 effect))) &&
			   !g_strcmp0(line_member->specifier,
				      specifier)){
			  ags_simple_file_read_line_member(simple_file,
							   effect_child,
							   line_member);
			    
			  break;
			}
		      }
			
		      list = list->next;		    
		    }

		    if(list_start != NULL){
		      g_list_free(list_start);
		    }
		  }
		}

		effect_child = effect_child->next;
	      }
	    }
	  }

	  effect_list_child = effect_list_child->next;
	}
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-oscillator",
			   15)){	
	if(AGS_IS_SYNTH_INPUT_LINE(gobject)){
	  ags_simple_file_read_oscillator(simple_file, child, &(AGS_SYNTH_INPUT_LINE(gobject)->oscillator));
	}
      }else if(!xmlStrncmp(child->name,
			   (xmlChar *) "ags-sf-property-list",
			   14)){
	GList *property_start, *property;

	property_start = NULL;
	ags_simple_file_read_property_list((AgsSimpleFile *) file_launch->file,
					   child,
					   &property_start);

	property = property_start;

	while(property != NULL){
	  g_object_set_property((GObject *) pad,
				((GParameter *) property->data)->name,
				&(((GParameter *) property->data)->value));
	  
	  property = property->next;
	}
	
	g_list_free_full(property_start,
			 g_free);
      }
    }

    child = child->next;
  }

  //  ags_connectable_connect(AGS_CONNECTABLE(gobject));
  
  /* launch AgsLine */
  if(AGS_IS_LINE(gobject)){
    file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
						 "file", simple_file,
						 "node", node,
						 NULL);
    g_signal_connect(G_OBJECT(file_launch), "start",
		     G_CALLBACK(ags_simple_file_read_line_launch), gobject);
    ags_simple_file_add_launch(simple_file,
			       (GObject *) file_launch);
  }else if(AGS_IS_CHANNEL(gobject)){
    file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
						 "file", simple_file,
						 "node", node,
						 NULL);
    g_signal_connect(G_OBJECT(file_launch), "start",
		     G_CALLBACK(ags_simple_file_read_channel_line_launch), gobject);
    ags_simple_file_add_launch(simple_file,
			       (GObject *) file_launch);
  }else{
    g_warning("ags_simple_file_read_line() - failed");
  }
}

void
ags_simple_file_read_line_launch(AgsFileLaunch *file_launch,
				 AgsLine *line)
{
  AgsMachine *machine;
  AgsChannel *channel, *link;
  
  xmlNode *child;

  GList *xpath_result;

  xmlChar *str;
  
  guint nth_line;
  gboolean is_output;

  machine = (AgsMachine *) gtk_widget_get_ancestor((GtkWidget *) line,
						   AGS_TYPE_MACHINE);
  
  /* link or file */
  is_output = TRUE;
  str = xmlGetProp(file_launch->node->parent->parent->parent,
		   "is-output");

  if(str != NULL &&
     !g_strcmp0(str,
		"false")){
    is_output = FALSE;
  }

  nth_line = 0;
  str = xmlGetProp(file_launch->node,
		   "nth-line");

  if(str != NULL){
    nth_line = g_ascii_strtoull(str,
				NULL,
				10);
  }
  
  if(is_output){
    channel = ags_channel_nth(machine->audio->output,
			      nth_line);
  }else{
    channel = ags_channel_nth(machine->audio->input,
			      nth_line);
  }

  str = xmlGetProp(file_launch->node,
		   "filename");
    
  if(str != NULL &&
     !g_ascii_strncasecmp(str,
			  "file://",
			  7)){
    AgsAudioFile *audio_file;
    AgsAudioFileLink *file_link;
    
    GList *audio_signal_list;
    
    gchar *filename;

    guint file_channel;

    /* filename */
    filename = g_strdup(&(str[7]));

    /* audio channel to read */
    file_channel = 0;

    str = xmlGetProp(file_launch->node,
		     "file-channel");

    if(str != NULL){
      file_channel = g_ascii_strtoull(str,
				      NULL,
				      10);
    }

    /* read audio signal */
    audio_file = ags_audio_file_new(filename,
				    machine->audio->output_soundcard,
				    file_channel);

    ags_audio_file_open(audio_file);
    ags_audio_file_read_audio_signal(audio_file);

    /* add audio signal */
    audio_signal_list = audio_file->audio_signal;

    file_link = g_object_new(AGS_TYPE_AUDIO_FILE_LINK,
			     "filename", filename,
			     "audio-channel", file_channel,
			     NULL);
    g_object_set(G_OBJECT(channel),
		 "file-link", file_link,
		 NULL);

    if(audio_signal_list != NULL){
      AGS_AUDIO_SIGNAL(audio_signal_list->data)->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
      
      ags_recycling_add_audio_signal(channel->first_recycling,
				     audio_signal_list->data);
    }
  }else{
    str = xmlGetProp(file_launch->node,
		     "link");
    xpath_result = NULL;
    
    if(str != NULL){
      xpath_result = ags_simple_file_find_id_ref_by_xpath((AgsSimpleFile *) file_launch->file,
							  str);
    }
    
    while(xpath_result != NULL){
      AgsFileIdRef *file_id_ref;

      file_id_ref = xpath_result->data;

      if(AGS_IS_CHANNEL(file_id_ref->ref)){      
	GError *error;
    
	link = file_id_ref->ref;
	
	error = NULL;
	ags_channel_set_link(channel,
			     link,
			     &error);
	
	if(error != NULL){
	  g_warning("ags_simple_file_read_line_launch() - %s", error->message);
	}

	break;
      }else if(AGS_IS_LINE(file_id_ref->ref)){
	GError *error;
    
	link = AGS_LINE(file_id_ref->ref)->channel;
	
	error = NULL;
	ags_channel_set_link(channel,
			     link,
			     &error);

	if(error != NULL){
	  g_warning("ags_simple_file_read_line_launch() - %s", error->message);
	}

	break;
      }
      
      xpath_result = xpath_result->next;
    }
  }
  
  /* is-grouped */
  if(AGS_IS_LINE(line)){
    gtk_toggle_button_set_active(line->group,
				 g_strcmp0(xmlGetProp(file_launch->node,
						      "group"),
					   "false"));
  }
  
  /* children */
  child = file_launch->node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-property-list",
		     21)){
	GList *property_start, *property;

	property_start = NULL;
	ags_simple_file_read_property_list((AgsSimpleFile *) file_launch->file,
					   child,
					   &property_start);

	property = property_start;

	while(property != NULL){
	  g_object_set_property((GObject *) line,
				((GParameter *) property->data)->name,
				&(((GParameter *) property->data)->value));
	  
	  property = property->next;
	}
	
	g_list_free_full(property_start,
			 g_free);
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_channel_line_launch(AgsFileLaunch *file_launch,
					 AgsChannel *channel)
{
  AgsChannel *link;
  
  xmlNode *child;

  GList *xpath_result;

  xmlChar *str;

  str = xmlGetProp(file_launch->node,
		   "filename");
    
  if(str != NULL &&
     !g_ascii_strncasecmp(str,
			  "file://",
			  7)){
    AgsAudioFile *audio_file;
    AgsAudioFileLink *file_link;
    
    GList *audio_signal_list;
    
    gchar *filename;

    guint file_channel;

    /* filename */
    filename = g_strdup(&(str[7]));

    /* audio channel to read */
    file_channel = 0;

    str = xmlGetProp(file_launch->node,
		     "file-channel");

    if(str != NULL){
      file_channel = g_ascii_strtoull(str,
				      NULL,
				      10);
    }

    /* read audio signal */
    audio_file = ags_audio_file_new(filename,
				    channel->output_soundcard,
				    file_channel);

    ags_audio_file_open(audio_file);
    ags_audio_file_read_audio_signal(audio_file);

    /* add audio signal */
    audio_signal_list = audio_file->audio_signal;

    file_link = g_object_new(AGS_TYPE_AUDIO_FILE_LINK,
			     "filename", filename,
			     "audio-channel", file_channel,
			     NULL);
    g_object_set(G_OBJECT(channel),
		 "file-link", file_link,
		 NULL);

    if(audio_signal_list != NULL){
      AGS_AUDIO_SIGNAL(audio_signal_list->data)->flags |= AGS_AUDIO_SIGNAL_TEMPLATE;
      
      ags_recycling_add_audio_signal(channel->first_recycling,
				     audio_signal_list->data);
    }
  }else{
    str = xmlGetProp(file_launch->node,
		     "link");
    xpath_result = NULL;
    
    if(str != NULL){
      xpath_result = ags_simple_file_find_id_ref_by_xpath((AgsSimpleFile *) file_launch->file,
							  str);
    }
    
    while(xpath_result != NULL){
      AgsFileIdRef *file_id_ref;

      file_id_ref = xpath_result->data;

      if(AGS_IS_CHANNEL(file_id_ref->ref)){      
	GError *error;
    
	link = file_id_ref->ref;
	
	error = NULL;
	ags_channel_set_link(channel,
			     link,
			     &error);
	
	if(error != NULL){
	  g_warning("ags_simple_file_read_line_launch() - %s", error->message);
	}

	break;
      }else if(AGS_IS_LINE(file_id_ref->ref)){
	GError *error;
    
	link = AGS_LINE(file_id_ref->ref)->channel;
	
	error = NULL;
	ags_channel_set_link(channel,
			     link,
			     &error);

	if(error != NULL){
	  g_warning("ags_simple_file_read_line_launch() - %s", error->message);
	}

	break;
      }
      
      xpath_result = xpath_result->next;
    }
  }
}

void
ags_simple_file_read_effect_pad_list(AgsSimpleFile *simple_file, xmlNode *node, GList **effect_pad)
{
  AgsEffectPad *current;
  GList *list;
  
  xmlNode *child;

  child = node->children;
  list = NULL;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-effect-pad",
		     11)){
	current = NULL;
	ags_simple_file_read_effect_pad(simple_file, child, &current);
	list = g_list_prepend(list, current);
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *effect_pad = list;
}

void
ags_simple_file_read_effect_pad(AgsSimpleFile *simple_file, xmlNode *node, AgsEffectPad **effect_pad)
{
  AgsEffectPad *gobject;

  AgsFileLaunch *file_launch;

  xmlNode *child;
  
  if(*effect_pad != NULL){
    gobject = *effect_pad;
  }else{
    return;
  }

  /* children */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-effect-line-list",
		     24)){
	GList *list;

	list = NULL;
	ags_simple_file_read_effect_line_list(simple_file, child, &list);

	g_list_free(list);
      }
    }

    child = child->next;
  }

  /* launch AgsEffectPad */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_simple_file_read_effect_pad_launch), gobject);
  ags_simple_file_add_launch(simple_file,
			     (GObject *) file_launch);
}

void
ags_simple_file_read_effect_pad_launch(AgsFileLaunch *file_launch,
				       AgsEffectPad *effect_pad)
{
  /* empty */
}

void
ags_simple_file_read_effect_line_list(AgsSimpleFile *simple_file, xmlNode *node, GList **effect_line)
{
  AgsEffectLine *current;
  
  xmlNode *child;

  GList *list;
  
  guint i;

  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-effect-line",
		     11)){
	current = NULL;

	if(*effect_line != NULL){
	  GList *iter;

	  iter = g_list_nth(*effect_line,
			    i);

	  if(iter != NULL){
	    current = iter->data;
	  }
	}
	
	ags_simple_file_read_effect_line(simple_file, child, &current);
	list = g_list_prepend(list, current);

	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *effect_line = list;
}

void
ags_simple_file_read_effect_line(AgsSimpleFile *simple_file, xmlNode *node, AgsEffectLine **effect_line)
{
  AgsEffectLine *gobject;

  AgsFileLaunch *file_launch;

  xmlNode *child;

  xmlChar *str;
  
  if(*effect_line != NULL){
    gobject = *effect_line;
  }else{
    return;
  }

  /* children */
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-effect-list",
		     19)){
	xmlNode *effect_list_child;

	/* effect list children */
	effect_list_child = child->children;

	while(effect_list_child != NULL){
	  if(effect_list_child->type == XML_ELEMENT_NODE){
	    if(!xmlStrncmp(effect_list_child->name,
			   (xmlChar *) "ags-sf-effect",
			   14)){
	      xmlNode *effect_child;
	      
	      xmlChar *filename, *effect;

	      filename = xmlGetProp(effect_list_child,
				    "filename");
	      
	      effect = xmlGetProp(effect_list_child,
				  "effect");
	      
	      /* effect list children */
	      effect_child = child->children;

	      while(effect_child != NULL){
		if(effect_child->type == XML_ELEMENT_NODE){
		  if(!xmlStrncmp(effect_child->name,
				 (xmlChar *) "ags-sf-control",
				 15)){
		    AgsLineMember *line_member;
		    GtkWidget *child_widget;

		    GList *list_start, *list;
		    
		    xmlChar *specifier;

		    gdouble val;

		    specifier = xmlGetProp(effect_child,
					   "specifier");

		    str = xmlGetProp(effect_child,
				     "value");

		    if(str != NULL){
		      val = g_ascii_strtod(str,
					   NULL);
		      
		      list =
			list_start = gtk_container_get_children((GtkContainer *) gobject->table);

		      while(list != NULL){
			if(AGS_IS_LINE_MEMBER(list->data)){
			  line_member = AGS_LINE_MEMBER(list->data);
			
			  if(!g_strcmp0(line_member->filename,
					filename) &&
			     !g_strcmp0(line_member->effect,
					effect) &&
			     !g_strcmp0(line_member->specifier,
					specifier)){
			    child_widget = gtk_bin_get_child((GtkBin *) line_member);

			    if(GTK_IS_RANGE(child_widget)){
			      gtk_adjustment_set_value(GTK_RANGE(child_widget)->adjustment,
						       val);
			    }else if(GTK_IS_SPIN_BUTTON(child_widget)){
			      gtk_adjustment_set_value(GTK_SPIN_BUTTON(child_widget)->adjustment,
						       val);
			    }else if(AGS_IS_DIAL(child_widget)){
			      gtk_adjustment_set_value(AGS_DIAL(child_widget)->adjustment,
						       val);
			      ags_dial_draw((AgsDial *) child_widget);
			    }else if(GTK_IS_TOGGLE_BUTTON(child_widget)){
			      gtk_toggle_button_set_active((GtkToggleButton *) child_widget,
							   ((val != 0.0) ? TRUE: FALSE));
			    }else{
			      g_warning("ags_simple_file_read_effect_line() - unknown line member type");
			    }
			  
			    break;
			  }
			}

			list = list->next;
		      }
		    
		      g_list_free(list_start);
		    }
		  }
		}

		effect_child = effect_child->next;
	      }
	    }
	  }

	  effect_list_child = effect_list_child->next;
	}
      }
    }

    child = child->next;
  }

  /* launch AgsEffectLine */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_simple_file_read_effect_line_launch), gobject);
  ags_simple_file_add_launch(simple_file,
			     (GObject *) file_launch);
}

void
ags_simple_file_read_effect_line_launch(AgsFileLaunch *file_launch,
					AgsEffectLine *effect_line)
{
  /* empty */
}

void
ags_simple_file_read_oscillator_list(AgsSimpleFile *simple_file, xmlNode *node, GList **oscillator)
{
  AgsOscillator *current;
  
  xmlNode *child;

  GList *list;
  
  guint i;

  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-oscillator",
		     14)){
	current = NULL;

	if(*oscillator != NULL){
	  GList *iter;

	  iter = g_list_nth(*oscillator,
			    i);

	  if(iter != NULL){
	    current = iter->data;
	  }
	}
	
	ags_simple_file_read_oscillator(simple_file, child, &current);
	list = g_list_prepend(list, current);

	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *oscillator = list;
}

void
ags_simple_file_read_oscillator(AgsSimpleFile *simple_file, xmlNode *node, AgsOscillator **oscillator)
{
  AgsOscillator *gobject;
  
  xmlChar *str;
  
  gdouble val;
  guint nth;
  
  if(*oscillator != NULL){
    gobject = AGS_OSCILLATOR(oscillator[0]);
  }else{
    gobject = ags_oscillator_new();

    *oscillator = gobject;
  }

  str = xmlGetProp(node,
		   "wave");

  if(str != NULL){      
    nth = g_ascii_strtoull(str,
			   NULL,
			   10);
    gtk_combo_box_set_active(gobject->wave,
			     nth);
  }

  str = xmlGetProp(node,
		   "attack");

  if(str != NULL){
    val = g_ascii_strtod(str,
			 NULL);
    gtk_adjustment_set_value(gobject->attack->adjustment,
			     val);
  }

  str = xmlGetProp(node,
		   "frequency");

  if(str != NULL){
    val = g_ascii_strtod(str,
			 NULL);
    gtk_adjustment_set_value(gobject->frequency->adjustment,
			     val);
  }

  str = xmlGetProp(node,
		   "length");

  if(str != NULL){
    val = g_ascii_strtod(str,
			 NULL);
    gtk_adjustment_set_value(gobject->frame_count->adjustment,
			     val);
  }

  str = xmlGetProp(node,
		   "phase");

  if(str != NULL){
    val = g_ascii_strtod(str,
			 NULL);
    gtk_adjustment_set_value(gobject->phase->adjustment,
			     val);
  }

  str = xmlGetProp(node,
		   "volume");

  if(str != NULL){
    val = g_ascii_strtod(str,
			 NULL);
    gtk_adjustment_set_value(gobject->volume->adjustment,
			     val);
  }

  str = xmlGetProp(node,
		   "sync");

  if(str != NULL &&
     !xmlStrncmp(str,
		 "true",
		 5)){
    gtk_toggle_button_set_active(gobject->do_sync,
				 TRUE);
  }

  str = xmlGetProp(node,
		   "sync-point");

  if(str != NULL){
    gchar *endptr;

    gdouble current;
    guint i;

    endptr = str;
    
    for(i = 0; *endptr != '\0' && i < 2 * gobject->sync_point_count; i++){
      current = g_strtod(endptr,
			 &endptr);

      gtk_spin_button_set_value(gobject->sync_point[i],
				current);
    }
  }
}

void
ags_simple_file_read_notation_editor(AgsSimpleFile *simple_file, xmlNode *node, AgsNotationEditor **notation_editor)
{
  AgsNotationEditor *gobject;
  
  AgsFileLaunch *file_launch;

  if(*notation_editor != NULL){
    gobject = *notation_editor;
  }else{
    return;
  }
  
  /* launch AgsLine */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "file", simple_file,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_simple_file_read_notation_editor_launch), gobject);
  ags_simple_file_add_launch(simple_file,
			     (GObject *) file_launch);
}

void
ags_simple_file_read_notation_editor_launch(AgsFileLaunch *file_launch,
					    AgsNotationEditor *notation_editor)
{
  xmlNode *child;
  
  xmlChar *str;

  str = xmlGetProp(file_launch->node,
		   "zoom");

  if(str != NULL){
    GtkTreeModel *model;
    GtkTreeIter iter;

    gchar *value;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(notation_editor->notation_toolbar->zoom));
    
    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) notation_editor->notation_toolbar->zoom,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
  }

  /* children */
  child = file_launch->node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-property-list",
		     21)){
	GList *list_start, *list;
	GList *property_start, *property;

	property_start = NULL;
	ags_simple_file_read_property_list((AgsSimpleFile *) file_launch->file,
					   child,
					   &property_start);

	property = property_start;

	while(property != NULL){
  	  if(!g_strcmp0(((GParameter *) property->data)->name,
			"machine")){
	    AgsMachine *machine;

	    GList *file_id_ref;
	    GList *list_start, *list;

	    str = g_value_get_string(&(((GParameter *) property->data)->value));

	    if(str != NULL){
	      file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_xpath((AgsSimpleFile *) file_launch->file,
										  str);
	      machine = AGS_FILE_ID_REF(file_id_ref->data)->ref;
	    
	      ags_machine_selector_add_index(notation_editor->machine_selector);

	      list_start = gtk_container_get_children((GtkContainer *) notation_editor->machine_selector);
	      list = g_list_last(list_start);

	      gtk_button_clicked(list->data);
	      ags_machine_selector_link_index(notation_editor->machine_selector,
					      machine);
	      
	      g_list_free(list_start);
	    }
	  }else{
	    g_object_set_property((GObject *) notation_editor,
				  ((GParameter *) property->data)->name,
				  &(((GParameter *) property->data)->value));
	  }
	  
	  property = property->next;
	}

	list_start = gtk_container_get_children((GtkContainer *) notation_editor->machine_selector);
	list = list_start->next;

	if(list != NULL){
	  gtk_button_clicked(list->data);
	}
	
	g_list_free(list_start);
	g_list_free_full(property_start,
			 g_free);
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_automation_editor(AgsSimpleFile *simple_file, xmlNode *node, AgsAutomationEditor **automation_editor)
{
  AgsAutomationEditor *gobject;
  
  AgsFileLaunch *file_launch;

  if(*automation_editor != NULL){
    gobject = *automation_editor;
  }else{
    return;
  }

  /* launch AgsLine */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "file", simple_file,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_simple_file_read_automation_editor_launch), gobject);
  ags_simple_file_add_launch(simple_file,
			     (GObject *) file_launch);
}

void
ags_simple_file_read_automation_editor_launch(AgsFileLaunch *file_launch,
					      AgsAutomationEditor *automation_editor)
{
  xmlNode *child;
  
  xmlChar *str;

  str = xmlGetProp(file_launch->node,
		   "zoom");
  
  if(str != NULL){
    GtkTreeModel *model;
    GtkTreeIter iter;

    gchar *value;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(automation_editor->automation_toolbar->zoom));
    
    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) automation_editor->automation_toolbar->zoom,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
  }

  /* children */
  child = file_launch->node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-property-list",
		     21)){
	GList *list_start, *list;
	GList *property_start, *property;

	property_start = NULL;
	ags_simple_file_read_property_list((AgsSimpleFile *) file_launch->file,
					   child,
					   &property_start);

	property = property_start;

	while(property != NULL){
	  if(!g_strcmp0(((GParameter *) property->data)->name,
			"machine")){
	    AgsMachine *machine;

	    xmlNode *machine_child;
	    
	    GList *file_id_ref;
	    GList *list_start, *list;

	    gchar *str;
	    
	    str = g_value_get_string(&(((GParameter *) property->data)->value));
	    
	    if(str != NULL){
	      file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_xpath((AgsSimpleFile *) file_launch->file,
										  str);
	      machine = AGS_FILE_ID_REF(file_id_ref->data)->ref;

	      ags_machine_selector_add_index(automation_editor->machine_selector);

	      list_start = gtk_container_get_children((GtkContainer *) automation_editor->machine_selector);
	      list = g_list_last(list_start);

	      gtk_button_clicked(list->data);
	      ags_machine_selector_link_index(automation_editor->machine_selector,
					      machine);

	      /* apply automation ports */
	      machine_child = AGS_FILE_ID_REF(file_id_ref->data)->node->children;

	      while(machine_child != NULL){
		if(machine_child->type == XML_ELEMENT_NODE){
		  if(!xmlStrncmp(machine_child->name,
				 "ags-sf-automation-port-list",
				 27)){
		    xmlNode *automation_port;

		    automation_port = machine_child->children;

		    while(automation_port != NULL){
		      if(automation_port->type == XML_ELEMENT_NODE){
			if(!xmlStrncmp(automation_port->name,
				       "ags-sf-automation-port",
				       22)){
			  gchar *scope;
			  gchar *specifier;

			  specifier = xmlGetProp(automation_port,
						 "specifier");

			  specifier = xmlGetProp(automation_port,
						 "scope");

			  if(specifier != NULL){
			    GtkTreeModel *model;
			    GtkTreeIter iter;

			    gchar *value0, *value1;
			  
			    model = gtk_combo_box_get_model(GTK_COMBO_BOX(automation_editor->automation_toolbar->port));
			  
			    if(gtk_tree_model_get_iter_first(model, &iter)){
			      do{
				gtk_tree_model_get(model, &iter,
						   1, &value0,
						   2, &value1,
						   -1);

				if((scope == NULL ||
				    !g_ascii_strcasecmp(scope,
							value0)) &&
				   !g_ascii_strcasecmp(specifier,
						       value1)){
				  gtk_combo_box_set_active_iter(automation_editor->automation_toolbar->port,
								&iter);
				}
			      }while(gtk_tree_model_iter_next(model,
							      &iter));
			    }
			  }
			}
		      }

		      automation_port = automation_port->next;
		    }
		  }
		}
	      
		machine_child = machine_child->next;
	      }
	    
	      g_list_free(list_start);
	    }
	  }else{
	    g_object_set_property((GObject *) automation_editor,
				  ((GParameter *) property->data)->name,
				  &(((GParameter *) property->data)->value));
	  }
	  
	  property = property->next;
	}

	list_start = gtk_container_get_children((GtkContainer *) automation_editor->machine_selector);
	list = list_start->next;

	if(list != NULL){
	  gtk_button_clicked(list->data);
	}
	
	g_list_free(list_start);
	g_list_free_full(property_start,
			 g_free);
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_wave_editor(AgsSimpleFile *simple_file, xmlNode *node, AgsWaveEditor **wave_editor)
{
  AgsWaveEditor *gobject;
  
  AgsFileLaunch *file_launch;

  if(*wave_editor != NULL){
    gobject = *wave_editor;
  }else{
    return;
  }

  /* launch AgsLine */
  file_launch = (AgsFileLaunch *) g_object_new(AGS_TYPE_FILE_LAUNCH,
					       "file", simple_file,
					       "node", node,
					       NULL);
  g_signal_connect(G_OBJECT(file_launch), "start",
		   G_CALLBACK(ags_simple_file_read_wave_editor_launch), gobject);
  ags_simple_file_add_launch(simple_file,
			     (GObject *) file_launch);
}

void
ags_simple_file_read_wave_editor_launch(AgsFileLaunch *file_launch,
					AgsWaveEditor *wave_editor)
{
  xmlNode *child;
  
  xmlChar *str;

  str = xmlGetProp(file_launch->node,
		   "zoom");
  
  if(str != NULL){
    GtkTreeModel *model;
    GtkTreeIter iter;

    gchar *value;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(wave_editor->wave_toolbar->zoom));
    
    if(gtk_tree_model_get_iter_first(model, &iter)){
      do{
	gtk_tree_model_get(model, &iter,
			   0, &value,
			   -1);

	if(!g_strcmp0(str,
		      value)){
	  gtk_combo_box_set_active_iter((GtkComboBox *) wave_editor->wave_toolbar->zoom,
					&iter);
	  break;
	}
      }while(gtk_tree_model_iter_next(model,
				      &iter));
    }
  }

  /* children */
  child = file_launch->node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-property-list",
		     21)){
	GList *list_start, *list;
	GList *property_start, *property;

	property_start = NULL;
	ags_simple_file_read_property_list((AgsSimpleFile *) file_launch->file,
					   child,
					   &property_start);

	property = property_start;

	while(property != NULL){
	  if(!g_strcmp0(((GParameter *) property->data)->name,
			"machine")){
	    AgsMachine *machine;

	    xmlNode *machine_child;
	    
	    GList *file_id_ref;
	    GList *list_start, *list;

	    gchar *str;
	    
	    str = g_value_get_string(&(((GParameter *) property->data)->value));
	    
	    if(str != NULL){
	      file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_xpath((AgsSimpleFile *) file_launch->file,
										  str);
	      machine = AGS_FILE_ID_REF(file_id_ref->data)->ref;

	      ags_machine_selector_add_index(wave_editor->machine_selector);

	      list_start = gtk_container_get_children((GtkContainer *) wave_editor->machine_selector);
	      list = g_list_last(list_start);

	      gtk_button_clicked(list->data);
	      ags_machine_selector_link_index(wave_editor->machine_selector,
					      machine);
	    
	      g_list_free(list_start);
	    }
	  }else{
	    g_object_set_property((GObject *) wave_editor,
				  ((GParameter *) property->data)->name,
				  &(((GParameter *) property->data)->value));
	  }
	  
	  property = property->next;
	}

	list_start = gtk_container_get_children((GtkContainer *) wave_editor->machine_selector);
	list = list_start->next;

	if(list != NULL){
	  gtk_button_clicked(list->data);
	}
	
	g_list_free(list_start);
	g_list_free_full(property_start,
			 g_free);
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_notation_list(AgsSimpleFile *simple_file, xmlNode *node, GList **notation)
{
  AgsNotation *current;

  xmlNode *child;

  GList *list;

  guint i;

  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-notation",
		     11)){
	current = NULL;

	if(*notation != NULL){
	  GList *iter;

	  iter = g_list_nth(*notation,
			    i);

	  if(iter != NULL){
	    current = iter->data;
	  }
	}
	
	ags_simple_file_read_notation(simple_file, child, &current);
	list = g_list_prepend(list, current);

	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *notation = list;
}

void
ags_simple_file_read_notation_list_fixup_1_0_to_1_2(AgsSimpleFile *simple_file, xmlNode *node, GList **notation)
{
  AgsNotation *current;

  xmlNode *child;

  GList *list;

  guint i;

  auto void ags_simple_file_read_notation_fixup_1_0_to_1_2(AgsSimpleFile *simple_file, xmlNode *node, GList **notation);

  void ags_simple_file_read_notation_fixup_1_0_to_1_2(AgsSimpleFile *simple_file, xmlNode *node, GList **notation){
    AgsMachine *machine;
    
    AgsNotation *gobject;
    AgsNote *note;

    AgsTimestamp *timestamp;
    AgsFileIdRef *file_id_ref;

    xmlNode *child;

    GList *notation_list;
    
    xmlChar *str;
  
    guint audio_channel;
    gboolean found_timestamp;
    
    file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_node(simple_file,
								       node->parent->parent);
    machine = file_id_ref->ref;
    
    audio_channel = 0;
    str = xmlGetProp(node,
		     "channel");

    if(str != NULL){
      audio_channel = g_ascii_strtoull(str,
				       NULL,
				       10);
    }
    
    /* children */    
    timestamp = ags_timestamp_new();

    timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
    timestamp->flags |= AGS_TIMESTAMP_OFFSET;

    timestamp->timer.ags_offset.offset = 0;
    
    child = node->children;

    while(child != NULL){
      if(child->type == XML_ELEMENT_NODE){
	if(!xmlStrncmp(child->name,
		       "ags-sf-note",
		       12)){
	  note = ags_note_new();

	  /* position and offset */
	  str = xmlGetProp(child,
			   "x0");

	  if(str != NULL){
	    note->x[0] = g_ascii_strtoull(str,
					  NULL,
					  10);
	  }
	
	  str = xmlGetProp(child,
			   "x1");

	  if(str != NULL){
	    note->x[1] = g_ascii_strtoull(str,
					  NULL,
					  10);
	  }

	  str = xmlGetProp(child,
			   "y");

	  if(str != NULL){
	    note->y = g_ascii_strtoull(str,
				       NULL,
				       10);
	  }

	  /* envelope */
	  str = xmlGetProp(child,
			   "envelope");

	  if(str != NULL &&
	     !g_ascii_strncasecmp(str,
				  "true",
				  5)){
	    note->flags |= AGS_NOTE_ENVELOPE;
	  }

	  str = xmlGetProp(child,
			   "attack");

	  if(str != NULL){
	    sscanf(str, "%f %f", &(note->attack[0]), &(note->attack[1]));
	  }

	  str = xmlGetProp(child,
			   "decay");

	  if(str != NULL){
	    sscanf(str, "%f %f", &(note->decay[0]), &(note->decay[1]));
	  }

	  str = xmlGetProp(child,
			   "sustain");

	  if(str != NULL){
	    sscanf(str, "%f %f", &(note->sustain[0]), &(note->sustain[1]));
	  }

	  str = xmlGetProp(child,
			   "release");

	  if(str != NULL){
	    sscanf(str, "%f %f", &(note->release[0]), &(note->release[1]));
	  }

	  str = xmlGetProp(child,
			   "ratio");

	  if(str != NULL){
	    sscanf(str, "%f %f", &(note->ratio[0]), &(note->ratio[1]));
	  }

	  timestamp->timer.ags_offset.offset = AGS_NOTATION_DEFAULT_OFFSET * floor(note->x[0] / AGS_NOTATION_DEFAULT_OFFSET);

	  notation_list = ags_notation_find_near_timestamp(notation[0], audio_channel,
							   timestamp);
	  
	  if(notation_list != NULL){
	    gobject = notation_list->data;
	  }else{
	    gobject = g_object_new(AGS_TYPE_NOTATION,
				   "audio", machine->audio,
				   "audio-channel", audio_channel,
				   NULL);

	    gobject->timestamp->timer.ags_offset.offset = timestamp->timer.ags_offset.offset;
	    
	    notation[0] = ags_notation_add(notation[0],
					   gobject);
	  }
	  
	  /* add */
	  ags_notation_add_note(gobject,
				note,
				FALSE);
	}
      }

      child = child->next;
    }

    g_object_unref(timestamp);
  }

  if(notation == NULL){
    return;
  }
  
  child = node->children;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-notation",
		     11)){
	ags_simple_file_read_notation_fixup_1_0_to_1_2(simple_file, child, notation);

	i++;
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_notation(AgsSimpleFile *simple_file, xmlNode *node, AgsNotation **notation)
{
  AgsNotation *gobject;
  AgsNote *note;

  xmlNode *child;
  
  xmlChar *str;
  
  guint audio_channel;
  gboolean found_timestamp;
  
  if(*notation != NULL){
    gobject = *notation;

    audio_channel = gobject->audio_channel;
  }else{
    AgsMachine *machine;
    
    AgsFileIdRef *file_id_ref;
    
    file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_node(simple_file,
								       node->parent->parent);
    machine = file_id_ref->ref;

    if(!AGS_IS_MACHINE(machine)){
      return;
    }
    
    audio_channel = 0;
    str = xmlGetProp(node,
		     "channel");

    if(str != NULL){
      audio_channel = g_ascii_strtoull(str,
				       NULL,
				       10);
    }
    
    gobject = g_object_new(AGS_TYPE_NOTATION,
			   "audio", machine->audio,
			   "audio-channel", audio_channel,
			   NULL);

    *notation = gobject;
  }

  /* children */
  child = node->children;

  found_timestamp = FALSE;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-sf-timestamp",
		     17)){
	found_timestamp = TRUE;
	
	/* offset */
	str = xmlGetProp(child,
			 "offset");

	if(str != NULL){
	  gobject->timestamp->timer.ags_offset.offset = g_ascii_strtoull(str,
									 NULL,
									 10);
	}	
      }
    }

    child = child->next;
  }
  
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-sf-note",
		     12)){
	note = ags_note_new();

	/* position and offset */
	str = xmlGetProp(child,
			 "x0");

	if(str != NULL){
	  note->x[0] = g_ascii_strtoull(str,
				       NULL,
				       10);
	}
	
	str = xmlGetProp(child,
			 "x1");

	if(str != NULL){
	  note->x[1] = g_ascii_strtoull(str,
				       NULL,
				       10);
	}

	str = xmlGetProp(child,
			 "y");

	if(str != NULL){
	  note->y = g_ascii_strtoull(str,
				     NULL,
				     10);
	}

	/* envelope */
	str = xmlGetProp(child,
			 "envelope");

	if(str != NULL &&
	   !g_ascii_strncasecmp(str,
				"true",
				5)){
	  note->flags |= AGS_NOTE_ENVELOPE;
	}

	str = xmlGetProp(child,
			 "attack");

	if(str != NULL){
	  sscanf(str, "%f %f", &(note->attack[0]), &(note->attack[1]));
	}

	str = xmlGetProp(child,
			 "decay");

	if(str != NULL){
	  sscanf(str, "%f %f", &(note->decay[0]), &(note->decay[1]));
	}

	str = xmlGetProp(child,
			 "sustain");

	if(str != NULL){
	  sscanf(str, "%f %f", &(note->sustain[0]), &(note->sustain[1]));
	}

	str = xmlGetProp(child,
			 "release");

	if(str != NULL){
	  sscanf(str, "%f %f", &(note->release[0]), &(note->release[1]));
	}

	str = xmlGetProp(child,
			 "ratio");

	if(str != NULL){
	  sscanf(str, "%f %f", &(note->ratio[0]), &(note->ratio[1]));
	}

	/* add */
	ags_notation_add_note(gobject,
			      note,
			      FALSE);
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_automation_list(AgsSimpleFile *simple_file, xmlNode *node, GList **automation)
{
  AgsAutomation *current;

  xmlNode *child;

  GList *list;

  guint i;

  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-automation",
		     11)){
	current = NULL;

	if(*automation != NULL){
	  GList *iter;

	  iter = g_list_nth(*automation,
			    i);

	  if(iter != NULL){
	    current = iter->data;
	  }
	}
	
	ags_simple_file_read_automation(simple_file, child, &current);
	list = g_list_prepend(list, current);

	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *automation = list;
}

void
ags_simple_file_read_automation_list_fixup_1_0_to_1_3(AgsSimpleFile *simple_file, xmlNode *node, GList **automation)
{
  AgsAutomation *current;

  xmlNode *child;

  GList *list;

  guint i;

  auto void ags_simple_file_read_automation_fixup_1_0_to_1_3(AgsSimpleFile *simple_file, xmlNode *node, GList **automation);

  void ags_simple_file_read_automation_fixup_1_0_to_1_3(AgsSimpleFile *simple_file, xmlNode *node, GList **automation){
    AgsMachine *machine;
    
    AgsAutomation *gobject;
    AgsAcceleration *acceleration;

    AgsTimestamp *timestamp;
    AgsFileIdRef *file_id_ref;

    xmlNode *child;

    GList *automation_list;
    
    xmlChar *str;
    gchar *control_name;
  
    GType channel_type;
  
    guint line;
    gboolean found_timestamp;

    file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_node(simple_file,
								       node->parent->parent);
    machine = file_id_ref->ref;
    
    line = 0;
    str = xmlGetProp(node,
		     "line");

    str = xmlGetProp(node,
		     "channel-type");
    channel_type = g_type_from_name(str);
     
    control_name = xmlGetProp(node,
			      "control-name");

    if(str != NULL){
      line = g_ascii_strtoull(str,
			      NULL,
			      10);
    }
    
    /* children */    
    timestamp = ags_timestamp_new();

    timestamp->flags &= (~AGS_TIMESTAMP_UNIX);
    timestamp->flags |= AGS_TIMESTAMP_OFFSET;

    timestamp->timer.ags_offset.offset = 0;
    
    child = node->children;

    while(child != NULL){
      if(child->type == XML_ELEMENT_NODE){
	if(!xmlStrncmp(child->name,
		       "ags-sf-acceleration",
		       12)){
	  acceleration = ags_acceleration_new();

	  /* position and offset */
	  str = xmlGetProp(child,
			   "x");

	  if(str != NULL){
	    acceleration->x = g_ascii_strtoull(str,
					       NULL,
					       10);
	  }	

	  str = xmlGetProp(child,
			   "y");

	  if(str != NULL){
	    acceleration->y = g_ascii_strtoull(str,
					       NULL,
					       10);
	  }

	  timestamp->timer.ags_offset.offset = AGS_AUTOMATION_DEFAULT_OFFSET * floor(acceleration->x / AGS_AUTOMATION_DEFAULT_OFFSET);

	  automation_list = ags_automation_find_near_timestamp_extended(automation[0], line,
									channel_type, control_name,
									timestamp);
	  
	  if(automation_list != NULL){
	    gobject = automation_list->data;

	    channel_type = gobject->channel_type;
	    control_name = gobject->control_name;
	  }else{	    
	    gobject = g_object_new(AGS_TYPE_AUTOMATION,
				   "audio", machine->audio,
				   "line", line,
				   "channel-type", channel_type,
				   "control-name", control_name,
				   NULL);

	    gobject->timestamp->timer.ags_offset.offset = timestamp->timer.ags_offset.offset;
	    
	    automation[0] = ags_automation_add(automation[0],
					       gobject);
	  }
	  
	  /* add */
	  ags_automation_add_acceleration(gobject,
					  acceleration,
					  FALSE);
	}
      }

      child = child->next;
    }

    g_object_unref(timestamp);
  }

  if(automation == NULL){
    return;
  }
  
  child = node->children;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-automation",
		     11)){
	ags_simple_file_read_automation_fixup_1_0_to_1_3(simple_file, child, automation);

	i++;
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_automation(AgsSimpleFile *simple_file, xmlNode *node, AgsAutomation **automation)
{
  AgsAutomation *gobject;
  AgsAcceleration *acceleration;

  xmlNode *child;
  
  xmlChar *str;
  gchar *control_name;
  
  GType channel_type;

  guint line;
  gboolean found_timestamp;
  
  if(*automation != NULL){
    gobject = *automation;

    line = gobject->line;

    channel_type = gobject->channel_type;
    control_name = gobject->control_name;
  }else{
    AgsMachine *machine;
    
    AgsFileIdRef *file_id_ref;
    
    file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_node(simple_file,
								       node->parent->parent);
    machine = file_id_ref->ref;

    if(!AGS_IS_MACHINE(machine)){
      return;
    }
    
    line = 0;
    str = xmlGetProp(node,
		     "line");

    if(str != NULL){
      line = g_ascii_strtoull(str,
			      NULL,
			      10);
    }

    str = xmlGetProp(node,
		     "channel-type");
    channel_type = g_type_from_name(str);
     
    control_name = xmlGetProp(node,
			      "control-name");
    
    gobject = g_object_new(AGS_TYPE_AUTOMATION,
			   "audio", machine->audio,
			   "line", line,
			   "channel-type", channel_type,
			   "control-name", control_name,
			   NULL);

    *automation = gobject;
  }

  /* children */
  child = node->children;

  found_timestamp = FALSE;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-sf-timestamp",
		     17)){
	found_timestamp = TRUE;
	
	/* offset */
	str = xmlGetProp(child,
			 "offset");

	if(str != NULL){
	  gobject->timestamp->timer.ags_offset.offset = g_ascii_strtoull(str,
									 NULL,
									 10);
	}	
      }
    }

    child = child->next;
  }
  
  child = node->children;

  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-sf-acceleration",
		     12)){
	acceleration = ags_acceleration_new();

	/* position and offset */
	str = xmlGetProp(child,
			 "x");

	if(str != NULL){
	  acceleration->x = g_ascii_strtoull(str,
					     NULL,
					     10);
	}	

	str = xmlGetProp(child,
			 "y");

	if(str != NULL){
	  acceleration->y = g_ascii_strtoull(str,
					     NULL,
					     10);
	}

	/* add */
	ags_automation_add_acceleration(gobject,
					acceleration,
					FALSE);
      }
    }

    child = child->next;
  }
}

void
ags_simple_file_read_preset_list(AgsSimpleFile *simple_file, xmlNode *node, GList **preset)
{
  AgsPreset *current;
  
  xmlNode *child;

  GList *list;
  
  guint i;
  
  child = node->children;
  list = NULL;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     (xmlChar *) "ags-sf-preset",
		     11)){
	current = NULL;

	if(*preset != NULL){
	  GList *iter;

	  iter = g_list_nth(*preset,
			    i);

	  if(iter != NULL){
	    current = iter->data;
	  }
	}

	ags_simple_file_read_preset(simple_file, child, &current);
	list = g_list_prepend(list, current);
	
	i++;
      }
    }

    child = child->next;
  }

  list = g_list_reverse(list);
  *preset = list;
}

void
ags_simple_file_read_preset(AgsSimpleFile *simple_file, xmlNode *node, AgsPreset **preset)
{
  AgsMachine *machine;

  AgsPreset *gobject;

  AgsFileIdRef *file_id_ref;

  xmlNode *child;
  
  xmlChar *str;

  guint i;	
  
  file_id_ref = (AgsFileIdRef *) ags_simple_file_find_id_ref_by_node(simple_file,
								     node->parent->parent);
  machine = file_id_ref->ref;
  
  if(*preset != NULL){
    gobject = *preset;
  }else{
    gobject = ags_preset_new();
    
    *preset = gobject;
  }

  /* scope */
  str = xmlGetProp(node,
		   "scope");

  if(str != NULL){
    g_object_set(gobject,
		 "scope", str,
		 NULL);
  }

  /* preset name */
  str = xmlGetProp(node,
		   "preset-name");

  if(str != NULL){
    g_object_set(gobject,
		 "preset-name", str,
		 NULL);
  }

  /* audio channel start */
  str = xmlGetProp(node,
		   "audio-channel-start");

  if(str != NULL){
    g_object_set(gobject,
		 "audio-channel-start", g_ascii_strtoull(str,
							 NULL,
							 10),
		 NULL);
  }

  /* audio channel end */
  str = xmlGetProp(node,
		   "audio-channel-end");

  if(str != NULL){
    g_object_set(gobject,
		 "audio-channel-end", g_ascii_strtoull(str,
						       NULL,
						       10),
		 NULL);
  }

  /* pad start */
  str = xmlGetProp(node,
		   "pad-start");

  if(str != NULL){
    g_object_set(gobject,
		 "pad-start", g_ascii_strtoull(str,
					       NULL,
					       10),
		 NULL);
  }

  /* pad end */
  str = xmlGetProp(node,
		   "pad-end");

  if(str != NULL){
    g_object_set(gobject,
		 "pad-end", g_ascii_strtoull(str,
					     NULL,
					     10),
		 NULL);
  }

  /* x start */
  str = xmlGetProp(node,
		   "x-start");

  if(str != NULL){
    g_object_set(gobject,
		 "x-start", g_ascii_strtoull(str,
					     NULL,
					     10),
		 NULL);
  }

  /* x end */
  str = xmlGetProp(node,
		   "x-end");

  if(str != NULL){
    g_object_set(gobject,
		 "x-end", g_ascii_strtoull(str,
					   NULL,
					   10),
		 NULL);
  }

  /* children */
  child = node->children;

  i = 0;
  
  while(child != NULL){
    if(child->type == XML_ELEMENT_NODE){
      if(!xmlStrncmp(child->name,
		     "ags-sf-strv",
		     12)){
	ags_simple_file_read_strv(simple_file, child, &(gobject->parameter_name));
      }else if(!xmlStrncmp(child->name,
			   "ags-sf-value",
			   13)){
	if(gobject->value == NULL){
	  gobject->value = g_new0(GValue,
				  1);
	}else{
	  gobject->value = g_renew(GValue,
				   gobject->value,
				   i + 1);
	  memset(&(gobject->value[i]), 0, sizeof(GValue));
	}
	
	ags_simple_file_read_value(simple_file, child, &(gobject->value[i]));

	i++;
      }
    }

    child = child->next;
  }
}

xmlNode*
ags_simple_file_write_config(AgsSimpleFile *simple_file, xmlNode *parent, AgsConfig *ags_config)
{
  xmlNode *node;
  xmlNode *cdata;

  gchar *id;
  char *buffer;
  gsize buffer_length;

  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-sf-config");

  ags_simple_file_add_id_ref(simple_file,
			     g_object_new(AGS_TYPE_FILE_ID_REF,
					  "application-context", simple_file->application_context,
					  "file", simple_file,
					  "node", node,
					  "xpath", g_strdup_printf("xpath=//*[@id='%s']", id),
					  "reference", ags_config,
					  NULL));

  xmlNewProp(node,
	     AGS_FILE_ID_PROP,
	     id);

  xmlNewProp(node,
	     AGS_FILE_VERSION_PROP,
	     AGS_CONFIG(ags_config)->version);

  xmlNewProp(node,
	     AGS_FILE_BUILD_ID_PROP,
	     AGS_CONFIG(ags_config)->build_id);

  xmlAddChild(parent,
	      node);

  /* cdata */
  ags_config_to_data(ags_config,
		     &buffer,
		     &buffer_length);

  cdata = xmlNewCDataBlock(simple_file->doc,
			   buffer,
			   buffer_length);

  /* add to parent */
  xmlAddChild(node,
	      cdata);

  return(node);
}

xmlNode*
ags_simple_file_write_property_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *property)
{
  xmlNode *node;
  xmlNode *child;
  
  node = xmlNewNode(NULL,
		    "ags-sf-property-list");

  while(property != NULL){
    ags_simple_file_write_property(simple_file,
				   node,
				   property->data);

    property = property->next;
  }

  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_property(AgsSimpleFile *simple_file, xmlNode *parent, GParameter *property)
{
  xmlNode *node;
  
  xmlChar *type_name;
  xmlChar *val;
  
  if(G_VALUE_HOLDS_BOOLEAN(&(property->value))){
    type_name = g_type_name(G_TYPE_BOOLEAN);

    if(g_value_get_boolean(&(property->value))){
      val = g_strdup("true");
    }else{
      val = g_strdup("false");
    }
  }else if(G_VALUE_HOLDS_UINT(&(property->value))){
    type_name = g_type_name(G_TYPE_UINT);

    val = g_strdup_printf("%u",
			  g_value_get_uint(&(property->value)));
  }else if(G_VALUE_HOLDS_INT(&(property->value))){
    type_name = g_type_name(G_TYPE_INT);

    val = g_strdup_printf("%d",
			  g_value_get_int(&(property->value)));
  }else if(G_VALUE_HOLDS_DOUBLE(&(property->value))){
    type_name = g_type_name(G_TYPE_DOUBLE);

    val = g_strdup_printf("%f",
			  g_value_get_double(&(property->value)));
  }else if(G_VALUE_HOLDS(&(property->value),
			 AGS_TYPE_COMPLEX)){
    AgsComplex *z;
    
    type_name = g_type_name(AGS_TYPE_COMPLEX);

    z = g_value_get_boxed(&(property->value));
    val = g_strdup_printf("%f %f",
			  z[0][0], z[0][1]);
  }else{
    g_warning("ags_simple_file_write_property() - unsupported type");
    
    return(NULL);
  }

  node = xmlNewNode(NULL,
		    "ags-sf-property");
  
  xmlNewProp(node,
	     "type",
	     type_name);

  xmlNewProp(node,
	     "name",
	     property->name);

  xmlNewProp(node,
	     "value",
	     val);
  
  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_strv(AgsSimpleFile *simple_file, xmlNode *parent, gchar **strv)
{
  xmlNode *node;
  xmlNode *child;
  
  gchar **current;

  if(strv == NULL){
    return(NULL);
  }
  
  node = xmlNewNode(NULL,
		    "ags-sf-strv");
  
  /* add to parent */
  xmlAddChild(parent,
	      node);

  current = strv;

  while(*current != NULL){
    child = xmlNewNode(NULL,
		      "ags-sf-str");
    
    xmlNodeAddContent(child,
		      *current);
    
    /* add to parent */
    xmlAddChild(node,
		child);

    current++;
  }

  return(node);
}

xmlNode*
ags_simple_file_write_value(AgsSimpleFile *simple_file, xmlNode *parent, GValue *value)
{
  xmlNode *node;
  
  xmlChar *type_name;
  xmlChar *val;
  
  if(G_VALUE_HOLDS_BOOLEAN(value)){
    type_name = g_type_name(G_TYPE_BOOLEAN);

    if(g_value_get_boolean(value)){
      val = g_strdup("true");
    }else{
      val = g_strdup("false");
    }
  }else if(G_VALUE_HOLDS_UINT(value)){
    type_name = g_type_name(G_TYPE_UINT);

    val = g_strdup_printf("%u",
			  g_value_get_uint(value));
  }else if(G_VALUE_HOLDS_INT(value)){
    type_name = g_type_name(G_TYPE_INT);

    val = g_strdup_printf("%d",
			  g_value_get_int(value));
  }else if(G_VALUE_HOLDS_DOUBLE(value)){
    type_name = g_type_name(G_TYPE_DOUBLE);

    val = g_strdup_printf("%f",
			  g_value_get_double(value));
  }else if(G_VALUE_HOLDS(value,
			 AGS_TYPE_COMPLEX)){
    AgsComplex *z;
    
    type_name = g_type_name(AGS_TYPE_COMPLEX);

    z = g_value_get_boxed(value);
    val = g_strdup_printf("%f %f",
			  z[0][0], z[0][1]);
  }else{
    g_warning("ags_simple_file_write_property() - unsupported type");
    
    return(NULL);
  }

  node = xmlNewNode(NULL,
		    "ags-sf-value");
  
  xmlNewProp(node,
	     "type",
	     type_name);

  xmlNewProp(node,
	     "value",
	     val);
  
  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_window(AgsSimpleFile *simple_file, xmlNode *parent, AgsWindow *window)
{
  xmlNode *node;

  GList *list;
  
  node = xmlNewNode(NULL,
		    "ags-sf-window");

  xmlNewProp(node,
	     "filename",
	     simple_file->filename);

  xmlNewProp(node,
	     "bpm",
	     g_strdup_printf("%f",
			     window->navigation->bpm->adjustment->value));

  xmlNewProp(node,
	     "loop",
	     ((gtk_toggle_button_get_active((GtkToggleButton *) window->navigation->loop)) ? g_strdup("true"): g_strdup("false")));

  xmlNewProp(node,
	     "loop-start",
	     g_strdup_printf("%f",
			     window->navigation->loop_left_tact->adjustment->value));

  xmlNewProp(node,
	     "loop-end",
	     g_strdup_printf("%f",
			     window->navigation->loop_right_tact->adjustment->value));

  /* children */
  list = gtk_container_get_children((GtkContainer *) window->machines);
  ags_simple_file_write_machine_list(simple_file,
				     node,
				     list);
  g_list_free(list);

  ags_simple_file_write_notation_editor(simple_file,
					node,
					window->notation_editor);

  ags_simple_file_write_automation_editor(simple_file,
					  node,
					  window->automation_window->automation_editor);

  ags_simple_file_write_wave_editor(simple_file,
				    node,
				    window->wave_window->wave_editor);
  
  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_machine_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *machine)
{
  xmlNode *node;
  
  node = xmlNewNode(NULL,
		    "ags-sf-machine-list");

  while(machine != NULL){
    ags_simple_file_write_machine(simple_file,
				  node,
				  machine->data);

    machine = machine->next;
  }

  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_machine(AgsSimpleFile *simple_file, xmlNode *parent, AgsMachine *machine)
{  
  xmlNode *node;
  xmlNode *pad_list;    

  GList *list;

  gchar *id;
  xmlChar *str;
  
  auto gboolean ags_simple_file_write_machine_inline_pad(AgsSimpleFile *simple_file, xmlNode *parent, AgsChannel *channel);
  auto xmlNode* ags_simple_file_write_control(AgsSimpleFile *simple_file, xmlNode *parent, AgsBulkMember *bulk_member);
  auto xmlNode* ags_simple_file_write_effect_list(AgsSimpleFile *simple_file, xmlNode *parent, AgsEffectBulk *effect_bulk);
  auto xmlNode* ags_simple_file_write_automation_port(AgsSimpleFile *simple_file, xmlNode *parent, GList *automation_port);
  
  gboolean ags_simple_file_write_machine_inline_pad(AgsSimpleFile *simple_file, xmlNode *parent, AgsChannel *channel){
    AgsChannel *next_pad;

    xmlNode *pad;
    xmlNode *line_list;
    xmlNode *line;

    gboolean found_pad_content, found_line_content;
    
    if(channel == NULL){
      return(FALSE);
    }

    found_pad_content = FALSE;
    
    while(channel != NULL){
      
      next_pad = channel->next_pad;

      pad = xmlNewNode(NULL,
		       "ags-sf-pad");

      xmlNewProp(pad,
		 "nth-pad",
		 g_strdup_printf("%d",
				 channel->pad));

      line_list = xmlNewNode(NULL,
			     "ags-sf-line-list");
      found_line_content = FALSE;

      while(channel != next_pad){
	gchar *id;
	
	id = ags_id_generator_create_uuid();
	  
	line = xmlNewNode(NULL,
			  "ags-sf-line");

	xmlNewProp(line,
		   "id",
		   id);

	if(channel->link != NULL){
	  ags_simple_file_add_id_ref(simple_file,
				     g_object_new(AGS_TYPE_FILE_ID_REF,
						  "application-context", simple_file->application_context,
						  "file", simple_file,
						  "node", line,
						  "reference", channel,
						  NULL));

	  found_line_content = TRUE;
	}

	xmlNewProp(line,
		   "nth-line",
		   g_strdup_printf("%d",
				   channel->line));

	/* device */
	if(channel != NULL &&
	   channel->output_soundcard != NULL){
	  gchar *device;

	  device = ags_soundcard_get_device(AGS_SOUNDCARD(channel->output_soundcard));
    
	  if(device != NULL){
	    xmlNewProp(line,
		       (xmlChar *) "soundcard-device",
		       (xmlChar *) g_strdup(device));
	  }
	}

	/* link */
	if(channel->link != NULL){
	  AgsFileLookup *file_lookup;

	  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
						       "file", simple_file,
						       "node", line,
						       "reference", channel,
						       NULL);
	  ags_simple_file_add_lookup(simple_file, (GObject *) file_lookup);
	  g_signal_connect(G_OBJECT(file_lookup), "resolve",
			   G_CALLBACK(ags_simple_file_write_line_resolve_link), channel);
	}

	/* add to parent */
	xmlAddChild(line_list,
		    line);

	/* iterate */
	channel = channel->next;
      }

      if(found_line_content){
	found_pad_content = TRUE;
	
	/* add to parent */
	xmlAddChild(pad,
		    line_list);
      
	/* add to parent */
	xmlAddChild(parent,
		    pad);
      }else{
	xmlFreeNode(line_list);
	xmlFreeNode(pad);
      }
    }

    if(found_pad_content){
      return(TRUE);
    }else{
      return(FALSE);
    }
  }

  xmlNode* ags_simple_file_write_control(AgsSimpleFile *simple_file, xmlNode *parent, AgsBulkMember *bulk_member){
    GtkWidget *child_widget;
    
    xmlNode *control_node;
    
    /* control node */
    child_widget = gtk_bin_get_child((GtkBin *) bulk_member);
    
    if(GTK_IS_TOGGLE_BUTTON(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");
      
      xmlNewProp(control_node,
		 "control-type",
		 G_OBJECT_TYPE_NAME(child_widget));
      
      xmlNewProp(control_node,
		 "value",
		 ((gtk_toggle_button_get_active((GtkToggleButton *) child_widget)) ? g_strdup("true"): g_strdup("false")));
    }else if(AGS_IS_DIAL(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "control-type",
		 G_OBJECT_TYPE_NAME(child_widget));
      
      xmlNewProp(control_node,
		 "value",
		 g_strdup_printf("%f", AGS_DIAL(child_widget)->adjustment->value));
    }else{
      g_warning("ags_file_write_effect_list() - unknown child of AgsBulkMember type");

      return(NULL);
    }

    xmlNewProp(control_node,
	       "specifier",
	       bulk_member->specifier);

    xmlAddChild(parent,
		control_node);

    return(control_node);
  }
  
  xmlNode* ags_simple_file_write_effect_list(AgsSimpleFile *simple_file, xmlNode *parent, AgsEffectBulk *effect_bulk){
    xmlNode *effect_list_node;
    xmlNode *effect_node;
    
    GList *filename;
    GList *effect;
    GList *list_start, *list;
    
    effect_list_node = NULL;
    effect_node = NULL;
    
    filename = NULL;
    effect = NULL;
    
    list =
      list_start = gtk_container_get_children((GtkContainer *) effect_bulk->table);
    
    while(list != NULL){
      if(AGS_IS_BULK_MEMBER(list->data)){
	if(g_list_find_custom(filename,
			      AGS_BULK_MEMBER(list->data)->filename,
			      (GCompareFunc) g_strcmp0) == NULL ||
	   g_list_find_custom(effect,
			      AGS_BULK_MEMBER(list->data)->effect,
			      (GCompareFunc) g_strcmp0) == NULL){
	  GtkWidget *child_widget;

	  if(effect_list_node == NULL){
	    effect_list_node = xmlNewNode(NULL,
					  "ags-sf-effect-list");
	  }
	  
	  effect_node = xmlNewNode(NULL,
				   "ags-sf-effect");

	  xmlNewProp(effect_node,
		     "filename",
		     AGS_BULK_MEMBER(list->data)->filename);
	  filename = g_list_prepend(filename,
				    AGS_BULK_MEMBER(list->data)->filename);

	  xmlNewProp(effect_node,
		     "effect",
		     AGS_BULK_MEMBER(list->data)->effect);
	  effect = g_list_prepend(effect,
				  AGS_BULK_MEMBER(list->data)->effect);
	  
	  ags_simple_file_write_control(simple_file, effect_node, list->data);
	  
	  /* add to parent */
	  xmlAddChild(effect_list_node,
		      effect_node);
	}else{
	  ags_simple_file_write_control(simple_file, effect_node, list->data);
	}
      }

      list = list->next;
    }

    g_list_free(list_start);

    /* add to parent */
    if(effect_list_node != NULL){
      xmlAddChild(parent,
		  effect_list_node);
    }

    return(effect_list_node);
  }

  xmlNode* ags_simple_file_write_automation_port(AgsSimpleFile *simple_file, xmlNode *parent, GList *automation_port){
    xmlNode *node, *child;    

    node = NULL;
    
    if(automation_port != NULL){
      node = xmlNewNode(NULL,
			"ags-sf-automation-port-list");
      
      while(automation_port != NULL){
	gchar *scope;
	
	child = xmlNewNode(NULL,
			   "ags-sf-automation-port");

	scope = NULL;

	if(AGS_MACHINE_AUTOMATION_PORT(automation_port->data)->channel_type == G_TYPE_NONE){
	  scope = "audio";
	}else if(AGS_MACHINE_AUTOMATION_PORT(automation_port->data)->channel_type == AGS_TYPE_OUTPUT){
	  scope = "output";
	}else if(AGS_MACHINE_AUTOMATION_PORT(automation_port->data)->channel_type == AGS_TYPE_INPUT){
	  scope = "input";
	}
	
	xmlNewProp(child,
		   "scope",
		   scope);

	xmlNewProp(child,
		   "specifier",
		   AGS_MACHINE_AUTOMATION_PORT(automation_port->data)->control_name);
	
	xmlAddChild(node,
		    child);

	automation_port = automation_port->next;
      }
    }
    
    /* add to parent */
    if(node != NULL){
      xmlAddChild(parent,
		  node);
    }
    
    return(node);
  }

  /* node and uuid */
  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-sf-machine");
  
  xmlNewProp(node,
	     (xmlChar *) AGS_FILE_ID_PROP,
	     (xmlChar *) id);

  xmlNewProp(node,
	     (xmlChar *) AGS_FILE_TYPE_PROP,
	     (xmlChar *) G_OBJECT_TYPE_NAME(machine));

  xmlNewProp(node,
	     (xmlChar *) AGS_FILE_NAME_PROP,
	     (xmlChar *) machine->machine_name);

  ags_simple_file_add_id_ref(simple_file,
			     g_object_new(AGS_TYPE_FILE_ID_REF,
					  "application-context", simple_file->application_context,
					  "file", simple_file,
					  "node", node,
					  "reference", machine,
					  NULL));
  
  /* device */
  if(machine->audio != NULL &&
     machine->audio->output_soundcard != NULL){
    gchar *device;

    device = ags_soundcard_get_device(AGS_SOUNDCARD(machine->audio->output_soundcard));
    
    if(device != NULL){
      xmlNewProp(node,
		 (xmlChar *) "soundcard-device",
		 (xmlChar *) g_strdup(device));
    }
  }
  
  /* bank and mapping */
  xmlNewProp(node,
	     (xmlChar *) "bank_0",
	     (xmlChar *) g_strdup_printf("%d", machine->bank_0));

  xmlNewProp(node,
	     (xmlChar *) "bank_1",
	     (xmlChar *) g_strdup_printf("%d", machine->bank_1));
  
  if((AGS_SOUND_BEHAVIOUR_REVERSE_MAPPING & (machine->audio->behaviour_flags)) != 0){
    xmlNewProp(node,
	       "reverse-mapping",
	       "true");
  }

  /* channels and pads */
  xmlNewProp(node,
	     (xmlChar *) "channels",
	     (xmlChar *) g_strdup_printf("%d", machine->audio->audio_channels));

  xmlNewProp(node,
	     (xmlChar *) "output-pads",
	     (xmlChar *) g_strdup_printf("%d", machine->audio->output_pads));

  xmlNewProp(node,
	     (xmlChar *) "input-pads",
	     (xmlChar *) g_strdup_printf("%d", machine->audio->input_pads));

  /* midi mapping */
  if((AGS_MACHINE_IS_SYNTHESIZER & (machine->flags)) != 0){
    xmlNewProp(node,
	       "audio-start-mapping",
	       g_strdup_printf("%d", machine->audio->audio_start_mapping));

    xmlNewProp(node,
	       "audio-end-mapping",
	       g_strdup_printf("%d", machine->audio->audio_end_mapping));

    xmlNewProp(node,
	       "midi-start-mapping",
	       g_strdup_printf("%d", machine->audio->midi_start_mapping));

    xmlNewProp(node,
	       "midi-end-mapping",
	       g_strdup_printf("%d", machine->audio->midi_end_mapping));
  }
  
  /* machine specific */
  if(AGS_IS_EQUALIZER10(machine)){
    AgsEqualizer10 *equalizer10;

    xmlNode *child;
    xmlNode *control_node;
    
    equalizer10 = (AgsEqualizer10 *) machine;

    child = xmlNewNode(NULL,
		       "ags-sf-equalizer10");
    xmlAddChild(node,
		child);
    
    /* peak 28hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);
    
    xmlNewProp(control_node,
	       "specifier",
	       "28 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_28hz)));

    /* peak 56hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "56 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_56hz)));

    /* peak 112hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "112 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_112hz)));

    /* peak 224hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "224 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_224hz)));

    /* peak 448hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "448 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_448hz)));

    /* peak 896hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "896 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_896hz)));

    /* peak 1792hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "1792 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_1792hz)));

    /* peak 3584hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "3584 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_3584hz)));

    /* peak 7168hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "7168 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_7168hz)));

    /* peak 14336hz */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "14336 [Hz]");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->peak_14336hz)));

    /* pressure */
    control_node = xmlNewNode(NULL,
			      "ags-sf-control");
    xmlAddChild(child,
		control_node);

    xmlNewProp(control_node,
	       "specifier",
	       "pressure");
    
    xmlNewProp(control_node,
	       "control-type",
	       "GtkScale");
    
    xmlNewProp(control_node,
	       "value",
	       g_strdup_printf("%f", gtk_range_get_value(equalizer10->pressure)));
  }else if(AGS_IS_DRUM(machine)){
    AgsDrum *drum;

    drum = (AgsDrum *) machine;
    
    if(gtk_toggle_button_get_active((GtkToggleButton *) drum->loop_button)){
      xmlNewProp(node,
		 "loop",
		 "true");
    }

    xmlNewProp(node,
	       "length",
	       g_strdup_printf("%u", (guint) drum->length_spin->adjustment->value));
  }else if(AGS_IS_MATRIX(machine)){
    AgsMatrix *matrix;

    matrix = (AgsMatrix *) machine;
    
    if(gtk_toggle_button_get_active((GtkToggleButton *) matrix->loop_button)){
      xmlNewProp(node,
		 "loop",
		 "true");
    }    


    xmlNewProp(node,
	       "length",
	       g_strdup_printf("%u", (guint) matrix->length_spin->adjustment->value));
  }else if(AGS_IS_SYNTH(machine)){
    AgsSynth *synth;

    synth = (AgsSynth *) machine;
    
    xmlNewProp(node,
	       "base-note",
	       g_strdup_printf("%f", synth->lower->adjustment->value));
  }else if(AGS_IS_SYNCSYNTH(machine)){
    AgsSyncsynth *syncsynth;

    syncsynth = (AgsSyncsynth *) machine;
    
    xmlNewProp(node,
	       "base-note",
	       g_strdup_printf("%f", syncsynth->lower->adjustment->value));

    xmlNewProp(node,
	       "audio-loop-start",
	       g_strdup_printf("%u", (guint) syncsynth->loop_start->adjustment->value));

    xmlNewProp(node,
	       "audio-loop-end",
	       g_strdup_printf("%u", (guint) syncsynth->loop_end->adjustment->value));
#ifdef AGS_WITH_LIBINSTPATCH
  }else if(AGS_IS_FFPLAYER(machine)){
    AgsFFPlayer *ffplayer;

    ffplayer = (AgsFFPlayer *) machine;

    if(ffplayer->audio_container != NULL &&
       ffplayer->audio_container->filename != NULL){
      xmlNewProp(node,
		 "filename",
		 ffplayer->audio_container->filename);

      xmlNewProp(node,
		 "preset",
		 gtk_combo_box_text_get_active_text(ffplayer->preset));

      xmlNewProp(node,
		 "instrument",
		 gtk_combo_box_text_get_active_text(ffplayer->instrument));
    }
#endif
  }else if(AGS_IS_AUDIOREC(machine)){
    AgsAudiorec *audiorec;

    audiorec = (AgsAudiorec *) machine;

    xmlNewProp(node,
	       "filename",
	       gtk_entry_get_text(audiorec->filename));
  }else if(AGS_IS_LADSPA_BRIDGE(machine)){
    AgsLadspaBridge *ladspa_bridge;

    ladspa_bridge = (AgsLadspaBridge *) machine;

    xmlNewProp(node,
	       "plugin-file",
	       ladspa_bridge->filename);

    xmlNewProp(node,
	       "effect",
	       ladspa_bridge->effect);
  }else if(AGS_IS_DSSI_BRIDGE(machine)){
    AgsDssiBridge *dssi_bridge;

    dssi_bridge = (AgsDssiBridge *) machine;

    xmlNewProp(node,
	       "plugin-file",
	       dssi_bridge->filename);

    xmlNewProp(node,
	       "effect",
	       dssi_bridge->effect);

    xmlNewProp(node,
	       "program",
	       gtk_combo_box_text_get_active_text(dssi_bridge->program));
  }else if(AGS_IS_LIVE_DSSI_BRIDGE(machine)){
    AgsLiveDssiBridge *live_dssi_bridge;

    live_dssi_bridge = (AgsLiveDssiBridge *) machine;

    xmlNewProp(node,
	       "plugin-file",
	       live_dssi_bridge->filename);

    xmlNewProp(node,
	       "effect",
	       live_dssi_bridge->effect);

    xmlNewProp(node,
	       "program",
	       gtk_combo_box_text_get_active_text(live_dssi_bridge->program));
  }else if(AGS_IS_LV2_BRIDGE(machine)){
    AgsLv2Bridge *lv2_bridge;

    lv2_bridge = (AgsLv2Bridge *) machine;

    xmlNewProp(node,
	       "plugin-file",
	       lv2_bridge->filename);

    xmlNewProp(node,
	       "effect",
	       lv2_bridge->effect);

    if(lv2_bridge->preset != NULL){
      xmlNewProp(node,
		 "preset",
		 gtk_combo_box_text_get_active_text(lv2_bridge->preset));
    }
  }else if(AGS_IS_LIVE_LV2_BRIDGE(machine)){
    AgsLiveLv2Bridge *live_lv2_bridge;

    live_lv2_bridge = (AgsLiveLv2Bridge *) machine;

    xmlNewProp(node,
	       "plugin-file",
	       live_lv2_bridge->filename);

    xmlNewProp(node,
	       "effect",
	       live_lv2_bridge->effect);

    if(live_lv2_bridge->preset != NULL){
      xmlNewProp(node,
		 "preset",
		 gtk_combo_box_text_get_active_text(live_lv2_bridge->preset));
    }
  }
  
  /* input */
  if(machine->input != NULL){
    list = gtk_container_get_children((GtkContainer *) machine->input);
  
    pad_list =  ags_simple_file_write_pad_list(simple_file,
					       node,
					       list);
    xmlNewProp(pad_list,
	       "is-output",
		 g_strdup("false"));
    
    g_list_free(list);
  }else{
    AgsChannel *channel;
    
    channel = machine->audio->input;

    if(channel != NULL){      
      pad_list = xmlNewNode(NULL,
			    "ags-sf-pad-list");
      xmlNewProp(pad_list,
		 "is-output",
		 g_strdup("false"));

      if(ags_simple_file_write_machine_inline_pad(simple_file,
						  pad_list,
						  channel)){
	/* add to parent */
	xmlAddChild(node,
		    pad_list);
      }else{
	xmlFreeNode(pad_list);
      }
    }
  }
  
  /* output */
  if(machine->output != NULL){
    list = gtk_container_get_children((GtkContainer *) machine->output);
  
    pad_list = ags_simple_file_write_pad_list(simple_file,
					      node,
					      list);
    xmlNewProp(pad_list,
	       "is-output",
	       g_strdup("true"));
    
    g_list_free(list);
  }else{
    AgsChannel *channel;
    
    channel = machine->audio->output;

    if(channel != NULL){
      pad_list = xmlNewNode(NULL,
			    "ags-sf-pad-list");
      xmlNewProp(pad_list,
		 "is-output",
		 g_strdup("true"));

      if(ags_simple_file_write_machine_inline_pad(simple_file,
						  pad_list,
						  channel)){            
	/* add to parent */
	xmlAddChild(node,
		    pad_list);
      }else{
	xmlFreeNode(pad_list);
      }
    }
  }

  /* effect list */
  if(machine->bridge != NULL){
    if(AGS_EFFECT_BRIDGE(machine->bridge)->bulk_input != NULL){
      xmlNode *child;
      
      child = ags_simple_file_write_effect_list(simple_file, node, (AgsEffectBulk *) AGS_EFFECT_BRIDGE(machine->bridge)->bulk_input);

      if(child != NULL){
	xmlNewProp(child,
		   "is-output",
		   "false");
      }
    }

    if(AGS_EFFECT_BRIDGE(machine->bridge)->bulk_output != NULL){
      xmlNode *child;

      child = ags_simple_file_write_effect_list(simple_file, node, (AgsEffectBulk *) AGS_EFFECT_BRIDGE(machine->bridge)->bulk_output);

      if(child != NULL){
	xmlNewProp(child,
		   "is-output",
		   "true");
      }
    }

    if(AGS_EFFECT_BRIDGE(machine->bridge)->input != NULL){
      xmlNode *child;
      
      GList *list;

      list = gtk_container_get_children((GtkContainer *) AGS_EFFECT_BRIDGE(machine->bridge)->input);
      
      child = ags_simple_file_write_effect_pad_list(simple_file,
						    node,
						    list);
      xmlNewProp(child,
		 "is-output",
		 "false");

      g_list_free(list);
    }

    if(AGS_EFFECT_BRIDGE(machine->bridge)->output != NULL){
      xmlNode *child;
      
      GList *list;

      list = gtk_container_get_children((GtkContainer *) AGS_EFFECT_BRIDGE(machine->bridge)->output);
      
      child = ags_simple_file_write_effect_pad_list(simple_file,
						    node,
						    list);
      xmlNewProp(child,
		 "is-output",
		 "true");
      
      g_list_free(list);
    }
  }

  /* oscillator list */
  if(AGS_IS_SYNCSYNTH(machine)){
    xmlNode *child;
    
    GList *list, *list_start, *next, *oscillator;

    list_start = 
      list = gtk_container_get_children((GtkContainer *) AGS_SYNCSYNTH(machine)->oscillator);

    oscillator = NULL;

    while(list != NULL){
      next = gtk_container_get_children(list->data);

      oscillator = g_list_prepend(oscillator,
				  next->next->data);

      g_list_free(next);
      
      list = list->next;
    }
    
    child = ags_simple_file_write_oscillator_list(simple_file,
						  node,
						  oscillator);
    
    g_list_free(list_start);
    g_list_free(oscillator);
  }
  
  /* pattern list */
  if(machine->audio->input != NULL){
    AgsChannel *channel;

    xmlNode *pattern_list_node;
    
    channel = machine->audio->input;
    pattern_list_node = NULL;
    
    while(channel != NULL){
      if(channel->pattern != NULL){
	AgsPattern *pattern;

	guint i, j, k;
	
	pattern = channel->pattern->data;

	for(i = 0; i < pattern->dim[0]; i++){
	  for(j = 0; j < pattern->dim[1]; j++){
	    if(!ags_pattern_is_empty(pattern,
				     i,
				     j)){
	      xmlNode *pattern_node;

	      xmlChar *content;
	      
	      if(pattern_list_node == NULL){
		pattern_list_node = xmlNewNode(NULL,
					       "ags-sf-pattern-list");
	      }

	      pattern_node = xmlNewNode(NULL,
					"ags-sf-pattern");

	      /* content */
	      content = malloc((pattern->dim[2] + 1) * sizeof(xmlChar));
	      content[pattern->dim[2]] = '\0';
	      
	      for(k = 0; k < pattern->dim[2]; k++){
		if(ags_pattern_get_bit(pattern,
				       i,
				       j,
				       k)){
		  content[k] = '1';
		}else{
		  content[k] = '0';
		}
	      }

	      xmlNodeAddContent(pattern_node,
				content);

	      /* attributes */
	      xmlNewProp(pattern_node,
			 "nth-line",
			 g_strdup_printf("%d",
					 channel->line));

	      xmlNewProp(pattern_node,
			 "bank-0",
			 g_strdup_printf("%d",
					 i));

	      xmlNewProp(pattern_node,
			 "bank-1",
			 g_strdup_printf("%d",
					 j));

	      /* add child */
	      xmlAddChild(pattern_list_node,
			  pattern_node);
	    }
	  }
	}
      }

      channel = channel->next;
    }

    if(pattern_list_node != NULL){
      xmlAddChild(node,
		  pattern_list_node);
    }
  }

  /* notation and automation list */
  if(machine->enabled_automation_port != NULL){
    ags_simple_file_write_automation_port(simple_file,
					  node,
					  machine->enabled_automation_port);
  }
  
  if(machine->audio->notation != NULL){
    ags_simple_file_write_notation_list(simple_file,
					node,
					machine->audio->notation);
  }

  if(machine->audio->automation != NULL){
    ags_simple_file_write_automation_list(simple_file,
					  node,
					  machine->audio->automation);
  }

  if(machine->audio->preset != NULL){
    ags_simple_file_write_preset_list(simple_file,
				      node,
				      machine->audio->preset);
  }

  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_pad_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *pad)
{
  xmlNode *node;

  gboolean found_content;

  found_content = FALSE;
  
  node = xmlNewNode(NULL,
		    "ags-sf-pad-list");

  while(pad != NULL){
    if(ags_simple_file_write_pad(simple_file,
				 node,
				 pad->data) != NULL){
      found_content = TRUE;
    }

    pad = pad->next;
  }

  if(found_content){
    /* add to parent */
    xmlAddChild(parent,
		node);

    return(node);
  }else{
    xmlFreeNode(node);
    
    return(NULL);
  }
}

xmlNode*
ags_simple_file_write_pad(AgsSimpleFile *simple_file, xmlNode *parent, AgsPad *pad)
{
  xmlNode *node;

  GList *list;

  gchar *id;

  gboolean found_content;

  found_content = FALSE;
  
  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-sf-pad");
  
  xmlNewProp(node,
	     (xmlChar *) AGS_FILE_ID_PROP,
	     (xmlChar *) id);

  xmlNewProp(node,
	     "nth-pad",
	     g_strdup_printf("%d", pad->channel->pad));
  
  /* group */
  if(!gtk_toggle_button_get_active(pad->group)){
    xmlNewProp(node,
	       "group",
	       g_strdup("false"));
    found_content = TRUE;
  }else{
    xmlNewProp(node,
	       "group",
	       g_strdup("true"));
  }

  /* mute */
  if(!gtk_toggle_button_get_active(pad->mute)){
    xmlNewProp(node,
	       "mute",
	       g_strdup("false"));
  }else{
    xmlNewProp(node,
	       "mute",
	       g_strdup("true"));
    found_content = TRUE;
  }

  /* solo */
  if(!gtk_toggle_button_get_active(pad->solo)){
    xmlNewProp(node,
	       "solo",
	       g_strdup("false"));
  }else{
    xmlNewProp(node,
	       "solo",
	       g_strdup("true"));
    found_content = TRUE;
  }

  /* children */
  if(pad->expander_set != NULL){
    list = gtk_container_get_children((GtkContainer *) pad->expander_set);
    
    if(ags_simple_file_write_line_list(simple_file,
				       node,
				       g_list_reverse(list)) != NULL){
      found_content = TRUE;
    }

    g_list_free(list);
  }

  if(found_content){
    /* add to parent */
    xmlAddChild(parent,
		node);

    return(node);
  }else{
    xmlFreeNode(node);
    
    return(NULL);
  }
}

xmlNode*
ags_simple_file_write_line_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *line)
{
  xmlNode *node;

  gboolean found_content;
  
  node = xmlNewNode(NULL,
		    "ags-sf-line-list");
  found_content = FALSE;
  
  while(line != NULL){
    if(ags_simple_file_write_line(simple_file,
				  node,
				  line->data) != NULL){
      found_content = TRUE;
    }

    line = line->next;
  }

  if(found_content){
    /* add to parent */
    xmlAddChild(parent,
		node);

    return(node);
  }else{
    xmlFreeNode(node);
    
    return(NULL);
  }
}

xmlNode*
ags_simple_file_write_line(AgsSimpleFile *simple_file, xmlNode *parent, AgsLine *line)
{
  xmlNode *node;
  xmlNode *effect_list_node;
  xmlNode *effect_node;
    
  GList *filename;
  GList *effect;
  GList *list_start, *list;

  gchar *id;

  gboolean found_content;
  
  auto void ags_simple_file_write_control(AgsSimpleFile *simple_file, xmlNode *parent, AgsLineMember *line_member);
  
  void ags_simple_file_write_control(AgsSimpleFile *simple_file, xmlNode *parent, AgsLineMember *line_member){
    GtkWidget *child_widget;
    
    xmlNode *control_node;
    
    /* control node */
    child_widget = gtk_bin_get_child((GtkBin *) line_member);

    if(GTK_IS_TOGGLE_BUTTON(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "control-type",
		 G_OBJECT_TYPE_NAME(child_widget));
    
      xmlNewProp(control_node,
		 "value",
		 ((gtk_toggle_button_get_active((GtkToggleButton *) child_widget)) ? g_strdup("true"): g_strdup("false")));
    }else if(AGS_IS_DIAL(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "control-type",
		 G_OBJECT_TYPE_NAME(child_widget));

      xmlNewProp(control_node,
		 "value",
		 g_strdup_printf("%f", AGS_DIAL(child_widget)->adjustment->value));
    }else if(GTK_IS_RANGE(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "control-type",
		 G_OBJECT_TYPE_NAME(child_widget));

      xmlNewProp(control_node,
		 "value",
		 g_strdup_printf("%f", GTK_RANGE(child_widget)->adjustment->value));
    }else if(GTK_IS_SPIN_BUTTON(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "control-type",
		 G_OBJECT_TYPE_NAME(child_widget));

      xmlNewProp(control_node,
		 "value",
		 g_strdup_printf("%f", GTK_SPIN_BUTTON(child_widget)->adjustment->value));
    }else{
      g_warning("ags_file_write_effect_list() - unknown child of AgsLineMember type");

      return;
    }

    xmlNewProp(control_node,
	       "specifier",
	       line_member->specifier);

    xmlAddChild(parent,
		control_node);
  }
  
  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-sf-line");
  
  xmlNewProp(node,
	     (xmlChar *) AGS_FILE_ID_PROP,
	     (xmlChar *) id);

  found_content = FALSE;

  if(line->channel->link != NULL){
    ags_simple_file_add_id_ref(simple_file,
			       g_object_new(AGS_TYPE_FILE_ID_REF,
					    "application-context", simple_file->application_context,
					    "file", simple_file,
					    "node", node,
					    "reference", line->channel,
					    NULL));

    found_content = TRUE;
  }
  
  xmlNewProp(node,
	     "nth-line",
	     g_strdup_printf("%d", line->channel->line));
  
  /* device */
  if(line->channel != NULL &&
     line->channel->output_soundcard != NULL){
    gchar *device;

    device = ags_soundcard_get_device(AGS_SOUNDCARD(line->channel->output_soundcard));
    
    if(device != NULL){
      xmlNewProp(node,
		 (xmlChar *) "soundcard-device",
		 (xmlChar *) g_strdup(device));
    }
  }

  /* group */
  if(!gtk_toggle_button_get_active(line->group)){
    xmlNewProp(node,
	       "is-grouped",
	       g_strdup("false"));

    found_content = TRUE;
  }else{
    xmlNewProp(node,
	       "is-grouped",
	       g_strdup("true"));
  }

  /* link or file */
  if(line->channel->link != NULL){
    AgsFileLookup *file_lookup;

    file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
						 "file", simple_file,
						 "node", node,
						 "reference", line,
						 NULL);
    ags_simple_file_add_lookup(simple_file, (GObject *) file_lookup);
    g_signal_connect(G_OBJECT(file_lookup), "resolve",
		     G_CALLBACK(ags_simple_file_write_line_resolve_link), line->channel);
  }else if(AGS_IS_INPUT(line->channel) &&
	   AGS_INPUT(line->channel)->file_link != NULL &&
	   ags_audio_file_check_suffix(AGS_FILE_LINK(AGS_INPUT(line->channel)->file_link)->filename)){
    xmlNewProp(node,
	       "filename",
	       g_strdup_printf("file://%s", AGS_FILE_LINK(AGS_INPUT(line->channel)->file_link)->filename));

    xmlNewProp(node,
	       "file-channel",
	       g_strdup_printf("%d", AGS_AUDIO_FILE_LINK(AGS_INPUT(line->channel)->file_link)->audio_channel));

    found_content = TRUE;
  }

  /* machine specific */
  if(AGS_IS_SYNTH_INPUT_LINE(line)){
    ags_simple_file_write_oscillator(simple_file,
				     node,
				     AGS_SYNTH_INPUT_LINE(line)->oscillator);
    found_content = TRUE;
  }
  
  /* effect list */
  effect_list_node = NULL;
  effect_node = NULL;
    
  filename = NULL;
  effect = NULL;

  list_start = 
    list = g_list_reverse(gtk_container_get_children((GtkContainer *) line->expander->table));
  
  while(list != NULL){
    if(AGS_IS_LINE_MEMBER(list->data)){
      AgsLineMember *line_member;

      line_member = list->data;

      if(g_list_find(filename,
		     AGS_LINE_MEMBER(list->data)->filename) == NULL ||
	 g_list_find(effect,
		     AGS_LINE_MEMBER(list->data)->effect) == NULL){
	GtkWidget *child_widget;

	if(effect_list_node == NULL){
	  effect_list_node = xmlNewNode(NULL,
					"ags-sf-effect-list");
	}
	  
	effect_node = xmlNewNode(NULL,
				 "ags-sf-effect");

	filename = g_list_prepend(filename,
				  AGS_LINE_MEMBER(list->data)->filename);

	effect = g_list_prepend(effect,
				AGS_LINE_MEMBER(list->data)->effect);

	xmlNewProp(effect_node,
		   "filename",
		   AGS_LINE_MEMBER(list->data)->filename);

	xmlNewProp(effect_node,
		   "effect",
		   AGS_LINE_MEMBER(list->data)->effect);
	
	ags_simple_file_write_control(simple_file, effect_node, list->data);
	  
	/* add to parent */
	xmlAddChild(effect_list_node,
		    effect_node);
      }else{
	ags_simple_file_write_control(simple_file, effect_node, list->data);
      }

      found_content = TRUE;
    }
    
    list = list->next;
  }

  if(list_start != NULL){
    g_list_free(list_start);
  }
  
  /* add to parent */
  if(effect_list_node != NULL){
    xmlAddChild(node,
		effect_list_node);
  }

  if(found_content){
    /* add to parent */
    xmlAddChild(parent,
		node);

    return(node);
  }else{
    xmlFreeNode(node);
    
    return(NULL);
  }
}

void
ags_simple_file_write_line_resolve_link(AgsFileLookup *file_lookup,
					AgsChannel *channel)
{
  GList *list;
  
  xmlChar *xpath;

  list = ags_simple_file_find_id_ref_by_reference((AgsSimpleFile *) file_lookup->file,
						  channel->link);
  
  if(list != NULL){
    xpath = g_strdup_printf("xpath=//ags-sf-line[@id='%s']",
			    xmlGetProp(AGS_FILE_ID_REF(list->data)->node,
				       "id"));
    xmlNewProp(file_lookup->node,
	       "link",
	       xpath);
  }
}

xmlNode*
ags_simple_file_write_effect_pad_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *effect_pad)
{
  xmlNode *node;

  gboolean found_content;

  found_content = FALSE;
  
  node = xmlNewNode(NULL,
		    "ags-sf-effect-pad-list");

  while(effect_pad != NULL){
    if(ags_simple_file_write_effect_pad(simple_file,
					node,
					effect_pad->data) != NULL){
      found_content = TRUE;
    }

    effect_pad = effect_pad->next;
  }

  if(found_content){
    /* add to parent */
    xmlAddChild(parent,
		node);
    
    return(node);
  }else{
    xmlFreeNode(node);
    
    return(NULL);
  }
}

xmlNode*
ags_simple_file_write_effect_pad(AgsSimpleFile *simple_file, xmlNode *parent, AgsEffectPad *effect_pad)
{
  xmlNode *node;

  GList *list;

  gchar *id;

  gboolean found_content;

  found_content = FALSE;
  
  id = ags_id_generator_create_uuid();

  node = xmlNewNode(NULL,
		    "ags-sf-effect-pad");
  
  xmlNewProp(node,
	     (xmlChar *) AGS_FILE_ID_PROP,
	     (xmlChar *) id);


  /* children */
  if(effect_pad->table != NULL){
    list = gtk_container_get_children((GtkContainer *) effect_pad->table);

    if(ags_simple_file_write_effect_line_list(simple_file,
					      node,
					      list) != NULL){
      found_content = TRUE;
    }

    g_list_free(list);
  }

  if(found_content){
    /* add to parent */
    xmlAddChild(parent,
		node);

    return(node);
  }else{
    xmlFreeNode(node);

    return(NULL);
  }
}

xmlNode*
ags_simple_file_write_effect_line_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *effect_line)
{
  xmlNode *node;

  gboolean found_content;

  found_content = FALSE;
  
  node = xmlNewNode(NULL,
		    "ags-sf-effect-line-list");

  while(effect_line != NULL){
    if(ags_simple_file_write_effect_line(simple_file,
					 node,
					 effect_line->data) != NULL){
      found_content = TRUE;
    }
    
    effect_line = effect_line->next;
  }
  
  if(found_content){
    /* add to parent */
    xmlAddChild(parent,
		node);
    
    return(node);
  }else{
    xmlFreeNode(node);
    
    return(NULL);
  }
}

xmlNode*
ags_simple_file_write_effect_line(AgsSimpleFile *simple_file, xmlNode *parent, AgsEffectLine *effect_line)
{
  xmlNode *node;
  xmlNode *effect_list_node;
  xmlNode *effect_node;
    
  GList *filename;
  GList *effect;
  GList *list_start, *list;

  gchar *id;

  gboolean found_content;
  
  auto void ags_simple_file_write_control(AgsSimpleFile *simple_file, xmlNode *parent, AgsLineMember *line_member);
  
  void ags_simple_file_write_control(AgsSimpleFile *simple_file, xmlNode *parent, AgsLineMember *line_member){
    GtkWidget *child_widget;
    
    xmlNode *control_node;
    
    /* control node */
    child_widget = gtk_bin_get_child((GtkBin *) line_member);
    
    if(GTK_IS_TOGGLE_BUTTON(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "value",
		 ((gtk_toggle_button_get_active((GtkToggleButton *) child_widget)) ? g_strdup("true"): g_strdup("false")));
    }else if(AGS_IS_DIAL(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "value",
		 g_strdup_printf("%f", AGS_DIAL(child_widget)->adjustment->value));
    }else if(GTK_IS_RANGE(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "value",
		 g_strdup_printf("%f", GTK_RANGE(child_widget)->adjustment->value));
    }else if(GTK_IS_SPIN_BUTTON(child_widget)){
      control_node = xmlNewNode(NULL,
				"ags-sf-control");

      xmlNewProp(control_node,
		 "value",
		 g_strdup_printf("%f", GTK_SPIN_BUTTON(child_widget)->adjustment->value));
    }else{
      g_warning("ags_file_write_effect_list() - unknown child of AgsLineMember type");

      return;
    }

    xmlNewProp(control_node,
	       "specifier",
	       line_member->specifier);

    xmlAddChild(parent,
		control_node);
  }

  found_content = FALSE;

  id = ags_id_generator_create_uuid();
  
  node = xmlNewNode(NULL,
		    "ags-sf-effect-line");
  
  xmlNewProp(node,
	     (xmlChar *) AGS_FILE_ID_PROP,
	     (xmlChar *) id);

  /* effect list */
  effect_list_node = NULL;
  effect_node = NULL;
    
  filename = NULL;
  effect = NULL;

  list_start = 
    list = gtk_container_get_children((GtkContainer *) effect_line->table);

  while(list != NULL){
    if(AGS_IS_LINE_MEMBER(list->data)){
      AgsLineMember *line_member;

      line_member = list->data;

      if(g_list_find(filename,
		     AGS_LINE_MEMBER(list->data)->filename) == NULL ||
	 g_list_find(effect,
		     AGS_LINE_MEMBER(list->data)->effect) == NULL){
	GtkWidget *child_widget;

	if(effect_list_node == NULL){
	  effect_list_node = xmlNewNode(NULL,
					"ags-sf-effect-list");
	}
	
	effect_node = xmlNewNode(NULL,
				 "ags-sf-effect");

	filename = g_list_prepend(filename,
				  AGS_LINE_MEMBER(list->data)->filename);

	effect = g_list_prepend(effect,
				AGS_LINE_MEMBER(list->data)->effect);
	  
	ags_simple_file_write_control(simple_file, effect_node, list->data);
	  
	/* add to parent */
	xmlAddChild(effect_list_node,
		    effect_node);
      }else{
	ags_simple_file_write_control(simple_file, effect_node, list->data);
      }
      
      found_content = TRUE;
    }
    
    list = list->next;
  }
  
  g_list_free(list_start);
  
  /* add to parent */
  if(effect_list_node != NULL){
    xmlAddChild(node,
		effect_list_node);
  }
  
  g_list_free(list_start);

  if(found_content){
    /* add to parent */
    xmlAddChild(parent,
		node);

    return(node);
  }else{
    xmlFreeNode(node);
    
    return(NULL);
  }
}

xmlNode*
ags_simple_file_write_oscillator_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *oscillator)
{
  xmlNode *node;
    
  node = xmlNewNode(NULL,
		    "ags-oscillator-list");

  while(oscillator != NULL){
    ags_simple_file_write_oscillator(simple_file,
				     node,
				     oscillator->data);
       
    oscillator = oscillator->next;
  }

  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_oscillator(AgsSimpleFile *simple_file, xmlNode *parent, AgsOscillator *oscillator)
{
  xmlNode *node;

  xmlChar *str, *tmp;

  guint i;
  
  node = xmlNewNode(NULL,
		    "ags-oscillator");
    
  xmlNewProp(node,
	     "wave",
	     g_strdup_printf("%d", gtk_combo_box_get_active(oscillator->wave)));

  xmlNewProp(node,
	     "attack",
	     g_strdup_printf("%f", oscillator->attack->adjustment->value));

  xmlNewProp(node,
	     "length",
	     g_strdup_printf("%f", oscillator->frame_count->adjustment->value));

  xmlNewProp(node,
	     "frequency",
	     g_strdup_printf("%f", oscillator->frequency->adjustment->value));

  xmlNewProp(node,
	     "phase",
	     g_strdup_printf("%f", oscillator->phase->adjustment->value));

  xmlNewProp(node,
	     "volume",
	     g_strdup_printf("%f", oscillator->volume->adjustment->value));

  xmlNewProp(node,
	     "sync",
	     g_strdup_printf("%s", (gtk_toggle_button_get_active(oscillator->do_sync) ? "true": "false")));

  str = NULL;

  for(i = 0; i < oscillator->sync_point_count; i++){
    tmp = str;

    if(str != NULL){
      str = g_strdup_printf("%s %f",
			    str, gtk_spin_button_get_value(oscillator->sync_point[i]));
    }else{
      str = g_strdup_printf("%f",
			    gtk_spin_button_get_value(oscillator->sync_point[i]));
    }
    
    g_free(tmp);
  }

  xmlNewProp(node,
	     "sync-point",
	     str);
  
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_notation_editor(AgsSimpleFile *simple_file, xmlNode *parent, AgsNotationEditor *notation_editor)
{
  AgsFileLookup *file_lookup;

  xmlNode *node;

  node = xmlNewNode(NULL,
		    "ags-sf-notation-editor");

  xmlNewProp(node,
	     "zoom",
	     gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(notation_editor->notation_toolbar->zoom)));
  
  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file", simple_file,
					       "node", node,
					       "reference", notation_editor,
					       NULL);
  ags_simple_file_add_lookup(simple_file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve",
		   G_CALLBACK(ags_simple_file_write_notation_editor_resolve_machine), notation_editor);
  
  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

void
ags_simple_file_write_notation_editor_resolve_machine(AgsFileLookup *file_lookup,
						      AgsNotationEditor *notation_editor)
{
  xmlNode *node;
  xmlNode *property_list;
  xmlNode *property;
  
  GList *list_start, *list;
  GList *file_id_ref;

  xmlChar *xpath;
  
  node = file_lookup->node;
  property_list = NULL;
  
  list_start = gtk_container_get_children((GtkContainer *) notation_editor->machine_selector);
  list = list_start->next;

  if(list != NULL){
    property_list = xmlNewNode(NULL,
			       "ags-sf-property-list");
  }
  
  while(list != NULL){    
    if(AGS_MACHINE_RADIO_BUTTON(list->data)->machine != NULL){
      property = xmlNewNode(NULL,
			    "ags-sf-property");
      
      xmlNewProp(property,
		 "name",
		 "machine");

      file_id_ref = ags_simple_file_find_id_ref_by_reference((AgsSimpleFile *) file_lookup->file,
							     AGS_MACHINE_RADIO_BUTTON(list->data)->machine);
      if(file_id_ref != NULL){
	xpath = g_strdup_printf("xpath=//ags-sf-machine[@id='%s']",
				xmlGetProp(AGS_FILE_ID_REF(file_id_ref->data)->node,
					   "id"));
      }else{
	xpath = g_strdup("(null)");
      }
      
      xmlNewProp(property,
		 "value",
		 xpath);

      /* add to parent */
      xmlAddChild(property_list,
		  property);
    }else{
      property = xmlNewNode(NULL,
			    "ags-sf-property");
      
      xmlNewProp(property,
		 "name",
		 "machine");

      xmlNewProp(property,
		 "value",
		 "(null)");

      /* add to parent */
      xmlAddChild(property_list,
		  property);
    }
    
    list = list->next;
  }

  /* add to parent */
  xmlAddChild(node,
	      property_list);
  
  g_list_free(list_start);
}

xmlNode*
ags_simple_file_write_automation_editor(AgsSimpleFile *simple_file, xmlNode *parent, AgsAutomationEditor *automation_editor)
{
  AgsFileLookup *file_lookup;

  xmlNode *node;
  
  node = xmlNewNode(NULL,
		    "ags-sf-automation-editor");

  xmlNewProp(node,
	     "zoom",
	     gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(automation_editor->automation_toolbar->zoom)));

  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file", simple_file,
					       "node", node,
					       "reference", automation_editor,
					       NULL);
  ags_simple_file_add_lookup(simple_file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve",
		   G_CALLBACK(ags_simple_file_write_automation_editor_resolve_machine), automation_editor);
  
  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

void
ags_simple_file_write_automation_editor_resolve_machine(AgsFileLookup *file_lookup,
							AgsAutomationEditor *automation_editor)
{
  xmlNode *node;
  xmlNode *property_list;
  xmlNode *property;
  
  GList *list_start, *list;
  GList *file_id_ref;

  xmlChar *xpath;
  
  node = file_lookup->node;
  property_list = NULL;
  
  list_start = gtk_container_get_children((GtkContainer *) automation_editor->machine_selector);
  list = list_start->next;

  if(list != NULL){
    property_list = xmlNewNode(NULL,
			       "ags-sf-property-list");
  }
  
  while(list != NULL){    
    if(AGS_MACHINE_RADIO_BUTTON(list->data)->machine != NULL){
      property = xmlNewNode(NULL,
			    "ags-sf-property");
      
      xmlNewProp(property,
		 "name",
		 "machine");

      file_id_ref = ags_simple_file_find_id_ref_by_reference((AgsSimpleFile *) file_lookup->file,
							     AGS_MACHINE_RADIO_BUTTON(list->data)->machine);
      if(file_id_ref != NULL){
	xpath = g_strdup_printf("xpath=//ags-sf-machine[@id='%s']",
				xmlGetProp(AGS_FILE_ID_REF(file_id_ref->data)->node,
					   "id"));
      }else{
	xpath = g_strdup("(null)");
      }
      
      xmlNewProp(property,
		 "value",
		 xpath);

      /* add to parent */
      xmlAddChild(property_list,
		  property);
    }else{
      property = xmlNewNode(NULL,
			    "ags-sf-property");
      
      xmlNewProp(property,
		 "name",
		 "machine");

      xmlNewProp(property,
		 "value",
		 "(null)");

      /* add to parent */
      xmlAddChild(property_list,
		  property);
    }
    
    list = list->next;
  }

  /* add to parent */
  xmlAddChild(node,
	      property_list);
  
  g_list_free(list_start);
}

xmlNode*
ags_simple_file_write_wave_editor(AgsSimpleFile *simple_file, xmlNode *parent, AgsWaveEditor *wave_editor)
{
  AgsFileLookup *file_lookup;

  xmlNode *node;
  
  node = xmlNewNode(NULL,
		    "ags-sf-wave-editor");

  xmlNewProp(node,
	     "zoom",
	     gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(wave_editor->wave_toolbar->zoom)));

  file_lookup = (AgsFileLookup *) g_object_new(AGS_TYPE_FILE_LOOKUP,
					       "file", simple_file,
					       "node", node,
					       "reference", wave_editor,
					       NULL);
  ags_simple_file_add_lookup(simple_file, (GObject *) file_lookup);
  g_signal_connect(G_OBJECT(file_lookup), "resolve",
		   G_CALLBACK(ags_simple_file_write_wave_editor_resolve_machine), wave_editor);
  
  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

void
ags_simple_file_write_wave_editor_resolve_machine(AgsFileLookup *file_lookup,
						  AgsWaveEditor *wave_editor)
{
  xmlNode *node;
  xmlNode *property_list;
  xmlNode *property;
  
  GList *list_start, *list;
  GList *file_id_ref;

  xmlChar *xpath;
  
  node = file_lookup->node;
  property_list = NULL;
  
  list_start = gtk_container_get_children((GtkContainer *) wave_editor->machine_selector);
  list = list_start->next;

  if(list != NULL){
    property_list = xmlNewNode(NULL,
			       "ags-sf-property-list");
  }
  
  while(list != NULL){    
    if(AGS_MACHINE_RADIO_BUTTON(list->data)->machine != NULL){
      property = xmlNewNode(NULL,
			    "ags-sf-property");
      
      xmlNewProp(property,
		 "name",
		 "machine");

      file_id_ref = ags_simple_file_find_id_ref_by_reference((AgsSimpleFile *) file_lookup->file,
							     AGS_MACHINE_RADIO_BUTTON(list->data)->machine);
      if(file_id_ref != NULL){
	xpath = g_strdup_printf("xpath=//ags-sf-machine[@id='%s']",
				xmlGetProp(AGS_FILE_ID_REF(file_id_ref->data)->node,
					   "id"));
      }else{
	xpath = g_strdup("(null)");
      }
      
      xmlNewProp(property,
		 "value",
		 xpath);

      /* add to parent */
      xmlAddChild(property_list,
		  property);
    }else{
      property = xmlNewNode(NULL,
			    "ags-sf-property");
      
      xmlNewProp(property,
		 "name",
		 "machine");

      xmlNewProp(property,
		 "value",
		 "(null)");

      /* add to parent */
      xmlAddChild(property_list,
		  property);
    }
    
    list = list->next;
  }

  /* add to parent */
  xmlAddChild(node,
	      property_list);
  
  g_list_free(list_start);
}

xmlNode*
ags_simple_file_write_notation_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *notation)
{
  xmlNode *node;
  
  node = xmlNewNode(NULL,
		    "ags-sf-notation-list");

  while(notation != NULL){
    ags_simple_file_write_notation(simple_file,
				   node,
				   notation->data);

    notation = notation->next;
  }

  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_notation(AgsSimpleFile *simple_file, xmlNode *parent, AgsNotation *notation)
{
  xmlNode *node;
  xmlNode *child;

  GList *list;

  node = xmlNewNode(NULL,
		    "ags-sf-notation");

  xmlNewProp(node,
	     "channel",
	     g_strdup_printf("%d", notation->audio_channel));

  /* timestamp */
  child = xmlNewNode(NULL,
		     "ags-sf-timestamp");

  xmlNewProp(child,
	     "offset",
	     g_strdup_printf("%lu",
			     notation->timestamp->timer.ags_offset.offset));
    
  /* add to parent */
  xmlAddChild(node,
	      child);

  /* note */
  list = notation->note;

  while(list != NULL){
    child = xmlNewNode(NULL,
		       "ags-sf-note");
    
    xmlNewProp(child,
	       "x0",
	       g_strdup_printf("%d",
			       AGS_NOTE(list->data)->x[0]));

    xmlNewProp(child,
	       "x1",
	       g_strdup_printf("%d",
			       AGS_NOTE(list->data)->x[1]));

    xmlNewProp(child,
	       "y",
	       g_strdup_printf("%d",
			       AGS_NOTE(list->data)->y));

    if((AGS_NOTE_ENVELOPE & (AGS_NOTE(list->data)->flags)) != 0){
      xmlNewProp(child,
		 "envelope",
		 "true");
    }
    
    xmlNewProp(child,
	       "attack",
	       g_strdup_printf("%f %f",
			       AGS_NOTE(list->data)->attack[0],
			       AGS_NOTE(list->data)->attack[1]));
    
    xmlNewProp(child,
	       "decay",
	       g_strdup_printf("%f %f",
			       AGS_NOTE(list->data)->decay[0],
			       AGS_NOTE(list->data)->decay[1]));
    
    xmlNewProp(child,
	       "sustain",
	       g_strdup_printf("%f %f",
			       AGS_NOTE(list->data)->sustain[0],
			       AGS_NOTE(list->data)->sustain[1]));
    
    xmlNewProp(child,
	       "release",
	       g_strdup_printf("%f %f",
    			       AGS_NOTE(list->data)->release[0],
			       AGS_NOTE(list->data)->release[1]));
			       
    xmlNewProp(child,
	       "ratio",
	       g_strdup_printf("%f %f",
			       AGS_NOTE(list->data)->ratio[0],
			       AGS_NOTE(list->data)->ratio[1]));
    
    /* add to parent */
    xmlAddChild(node,
		child);

    list = list->next;
  }

  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_automation_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *automation)
{
  xmlNode *node;
  
  node = xmlNewNode(NULL,
		    "ags-sf-automation-list");

  while(automation != NULL){
    ags_simple_file_write_automation(simple_file,
				     node,
				     automation->data);

    automation = automation->next;
  }

  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_automation(AgsSimpleFile *simple_file, xmlNode *parent, AgsAutomation *automation)
{
  xmlNode *node;
  xmlNode *child;

  GList *list;

  node = xmlNewNode(NULL,
		    "ags-sf-automation");

  xmlNewProp(node,
	     "line",
	     g_strdup_printf("%d", automation->line));

  /* timestamp */
  child = xmlNewNode(NULL,
		     "ags-sf-timestamp");

  xmlNewProp(child,
	     "offset",
	     g_strdup_printf("%lu",
			     automation->timestamp->timer.ags_offset.offset));
    
  /* add to parent */
  xmlAddChild(node,
	      child);

  /* acceleration */
  list = automation->acceleration;

  while(list != NULL){
    child = xmlNewNode(NULL,
		       "ags-sf-acceleration");
    
    xmlNewProp(child,
	       "x",
	       g_strdup_printf("%d",
			       AGS_ACCELERATION(list->data)->x));

    xmlNewProp(child,
	       "y",
	       g_strdup_printf("%d",
			       AGS_ACCELERATION(list->data)->y));

    /* add to parent */
    xmlAddChild(node,
		child);

    list = list->next;
  }

  /* add to parent */
  xmlAddChild(parent,
	      node);

  return(node);
}

xmlNode*
ags_simple_file_write_preset_list(AgsSimpleFile *simple_file, xmlNode *parent, GList *preset)
{
  xmlNode *node;

  gboolean found_node;
  
  node = xmlNewNode(NULL,
		    "ags-sf-preset-list");

  found_node = FALSE;
  
  while(preset != NULL){
    if(ags_simple_file_write_preset(simple_file,
				    node,
				    preset->data) != NULL){
      found_node = TRUE;
    }

    preset = preset->next;
  }

  if(found_node){
    /* add to parent */
    xmlAddChild(parent,
		node);

    return(node);
  }else{
    xmlFreeNode(node);

    return(NULL);
  }
}

xmlNode*
ags_simple_file_write_preset(AgsSimpleFile *simple_file, xmlNode *parent, AgsPreset *preset)
{
  xmlNode *node;
  xmlNode *child;
  
  guint i;
  
  node = xmlNewNode(NULL,
		    "ags-sf-preset");

  /* scope and preset name */
  xmlNewProp(node,
	     "scope",
	     g_strdup(preset->scope));

  xmlNewProp(node,
	     "preset-name",
	     g_strdup(preset->preset_name));

  /* mapping */
  xmlNewProp(node,
	     "audio-channel-start",
	     g_strdup_printf("%d", preset->audio_channel_start));

  xmlNewProp(node,
	     "audio-channel-end",
	     g_strdup_printf("%d", preset->audio_channel_end));

  xmlNewProp(node,
	     "pad-start",
	     g_strdup_printf("%d", preset->pad_start));

  xmlNewProp(node,
	     "pad-end",
	     g_strdup_printf("%d", preset->pad_end));

  xmlNewProp(node,
	     "x-start",
	     g_strdup_printf("%d", preset->x_start));

  xmlNewProp(node,
	     "x-end",
	     g_strdup_printf("%d", preset->x_end));

  /* parameter */
  ags_simple_file_write_strv(simple_file, node, preset->parameter_name);
  
  for(i = 0; i < preset->n_params; i++){
    ags_simple_file_write_value(simple_file, node, &(preset->value[i]));
  }
  
  if(preset->n_params > 0){
    /* add to parent */
    xmlAddChild(parent,
		node);

    return(node);
  }else{
    xmlFreeNode(node);
    
    return(NULL);
  }
}

AgsSimpleFile*
ags_simple_file_new()
{
  AgsSimpleFile *simple_file;

  simple_file = (AgsSimpleFile *) g_object_new(AGS_TYPE_SIMPLE_FILE,
					       NULL);

  return(simple_file);
}
