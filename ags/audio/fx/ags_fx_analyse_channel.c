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

#include <ags/audio/fx/ags_fx_analyse_channel.h>

#include <ags/audio/task/ags_reset_fx_analyse.h>

#include <ags/i18n.h>

void ags_fx_analyse_channel_class_init(AgsFxAnalyseChannelClass *fx_analyse_channel);
void ags_fx_analyse_channel_init(AgsFxAnalyseChannel *fx_analyse_channel);
void ags_fx_analyse_channel_set_property(GObject *gobject,
					 guint prop_id,
					 const GValue *value,
					 GParamSpec *param_spec);
void ags_fx_analyse_channel_get_property(GObject *gobject,
					 guint prop_id,
					 GValue *value,
					 GParamSpec *param_spec);
void ags_fx_analyse_channel_dispose(GObject *gobject);
void ags_fx_analyse_channel_finalize(GObject *gobject);

void ags_fx_analyse_channel_notify_samplerate_callback(GObject *gobject,
						       GParamSpec *pspec,
						       gpointer user_data);
void ags_fx_analyse_channel_notify_buffer_size_callback(GObject *gobject,
							GParamSpec *pspec,
							gpointer user_data);

/**
 * SECTION:ags_fx_analyse_channel
 * @short_description: fx analyse channel
 * @title: AgsFxAnalyseChannel
 * @section_id:
 * @include: ags/audio/fx/ags_fx_analyse_channel.h
 *
 * The #AgsFxAnalyseChannel class provides ports to the effect processor.
 */

static gpointer ags_fx_analyse_channel_parent_class = NULL;

static const gchar *ags_fx_analyse_channel_plugin_name = "ags-fx-analyse";

static const gchar *ags_fx_analyse_channel_specifier[] = {
  "./frequency[0]",
  "./magnitude[0]",
  NULL,
};

static const gchar *ags_fx_analyse_channel_control_port[] = {
  "1/2",
  "2/2",
  NULL,
};

enum{
  PROP_0,
  PROP_FREQUENCY,
  PROP_MAGNITUDE,
};

GType
ags_fx_analyse_channel_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_fx_analyse_channel = 0;

    static const GTypeInfo ags_fx_analyse_channel_info = {
      sizeof (AgsFxAnalyseChannelClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_fx_analyse_channel_class_init,
      NULL, /* class_finalize */
      NULL, /* class_channel */
      sizeof (AgsFxAnalyseChannel),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_fx_analyse_channel_init,
    };

    ags_type_fx_analyse_channel = g_type_register_static(AGS_TYPE_RECALL_CHANNEL,
							 "AgsFxAnalyseChannel",
							 &ags_fx_analyse_channel_info,
							 0);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_fx_analyse_channel);
  }

  return g_define_type_id__volatile;
}

void
ags_fx_analyse_channel_class_init(AgsFxAnalyseChannelClass *fx_analyse_channel)
{
  GObjectClass *gobject;

  GParamSpec *param_spec;

  ags_fx_analyse_channel_parent_class = g_type_class_peek_parent(fx_analyse_channel);

  /* GObjectClass */
  gobject = (GObjectClass *) fx_analyse_channel;

  gobject->set_property = ags_fx_analyse_channel_set_property;
  gobject->get_property = ags_fx_analyse_channel_get_property;

  gobject->dispose = ags_fx_analyse_channel_dispose;
  gobject->finalize = ags_fx_analyse_channel_finalize;

  /* properties */
  /**
   * AgsFxAnalyseChannel:frequency:
   *
   * The beats per minute.
   * 
   * Since: 3.3.0
   */
  param_spec = g_param_spec_object("frequency",
				   i18n_pspec("frequency of recall"),
				   i18n_pspec("The recall's frequency"),
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_FREQUENCY,
				  param_spec);

  /**
   * AgsFxAnalyseChannel:magnitude:
   *
   * The beats per minute.
   * 
   * Since: 3.3.0
   */
  param_spec = g_param_spec_object("magnitude",
				   i18n_pspec("magnitude of recall"),
				   i18n_pspec("The recall's magnitude"),
				   AGS_TYPE_PORT,
				   G_PARAM_READABLE | G_PARAM_WRITABLE);
  g_object_class_install_property(gobject,
				  PROP_MAGNITUDE,
				  param_spec);
}

void
ags_fx_analyse_channel_init(AgsFxAnalyseChannel *fx_analyse_channel)
{
  AgsResetFxAnalyse *reset_fx_analyse;

  double correction;
  guint samplerate;
  guint buffer_size;
  guint i;

  g_signal_connect(fx_analyse_channel, "notify::samplerate",
		   G_CALLBACK(ags_fx_analyse_channel_notify_samplerate_callback), NULL);
  
  g_signal_connect(fx_analyse_channel, "notify::buffer-size",
		   G_CALLBACK(ags_fx_analyse_channel_notify_buffer_size_callback), NULL);

  AGS_RECALL(fx_analyse_channel)->name = "ags-fx-analyse";
  AGS_RECALL(fx_analyse_channel)->version = AGS_RECALL_DEFAULT_VERSION;
  AGS_RECALL(fx_analyse_channel)->build_id = AGS_RECALL_DEFAULT_BUILD_ID;
  AGS_RECALL(fx_analyse_channel)->xml_type = "ags-fx-analyse-channel";

  samplerate = AGS_SOUNDCARD_DEFAULT_SAMPLERATE;
  buffer_size = AGS_SOUNDCARD_DEFAULT_BUFFER_SIZE;

  fx_analyse_channel->magnitude_cleared = TRUE;
  
  g_object_get(fx_analyse_channel,
	       "samplerate", &samplerate,
	       "buffer-size", &buffer_size,
	       NULL);
  
  /* frequency */
  fx_analyse_channel->frequency = g_object_new(AGS_TYPE_PORT,
					       "plugin-name", ags_fx_analyse_channel_plugin_name,
					       "specifier", ags_fx_analyse_channel_specifier[0],
					       "control-port", ags_fx_analyse_channel_control_port[0],
					       "port-value-is-pointer", TRUE,
					       "port-value-type", G_TYPE_DOUBLE,
					       "port-value-size", sizeof(gdouble),
					       "port-value-length", buffer_size,
					       NULL);
  ags_port_set_flags(fx_analyse_channel->frequency, AGS_PORT_IS_OUTPUT);
  
  fx_analyse_channel->frequency->port_value.ags_port_double_ptr = ags_stream_alloc(buffer_size,
										   AGS_SOUNDCARD_DOUBLE);

  ags_recall_add_port((AgsRecall *) fx_analyse_channel,
		      fx_analyse_channel->frequency);
  
  correction = (double) samplerate / (double) buffer_size;

  for(i = 0; i < buffer_size; i++){
    fx_analyse_channel->frequency->port_value.ags_port_double_ptr[i] = (double) i * (double) correction;
  }
  
  /* magnitude */
  fx_analyse_channel->magnitude = g_object_new(AGS_TYPE_PORT,
					       "plugin-name", ags_fx_analyse_channel_plugin_name,
					       "specifier", ags_fx_analyse_channel_specifier[1],
					       "control-port", ags_fx_analyse_channel_control_port[1],
					       "port-value-is-pointer", TRUE,
					       "port-value-type", G_TYPE_DOUBLE,
					       "port-value-size", sizeof(gdouble),
					       "port-value-length", buffer_size,
					       NULL);
  ags_port_set_flags(fx_analyse_channel->magnitude, AGS_PORT_IS_OUTPUT);
  
  fx_analyse_channel->magnitude->port_value.ags_port_double_ptr = ags_stream_alloc(buffer_size,
										   AGS_SOUNDCARD_DOUBLE);

  ags_recall_add_port((AgsRecall *) fx_analyse_channel,
		      fx_analyse_channel->magnitude);

  /* input data */
  for(i = 0; i < AGS_SOUND_SCOPE_LAST; i++){
    fx_analyse_channel->input_data[i] = ags_fx_analyse_channel_input_data_alloc();
      
    fx_analyse_channel->input_data[i]->parent = fx_analyse_channel;

    /* FFTW */
    fx_analyse_channel->input_data[i]->in = (double *) fftw_malloc(buffer_size * sizeof(double));
    fx_analyse_channel->input_data[i]->out = (double *) fftw_malloc(buffer_size * sizeof(double));

    fx_analyse_channel->input_data[i]->comout = (fftw_complex *) fftw_malloc(buffer_size * sizeof(fftw_complex));

    fx_analyse_channel->input_data[i]->plan = fftw_plan_r2r_1d(buffer_size, fx_analyse_channel->input_data[i]->in, fx_analyse_channel->input_data[i]->out, FFTW_R2HC, FFTW_ESTIMATE);
  }

  /* add to reset analyse task */
  reset_fx_analyse = ags_reset_fx_analyse_get_instance();

  ags_reset_fx_analyse_add(reset_fx_analyse,
			   fx_analyse_channel);
}

void
ags_fx_analyse_channel_set_property(GObject *gobject,
				    guint prop_id,
				    const GValue *value,
				    GParamSpec *param_spec)
{
  AgsFxAnalyseChannel *fx_analyse_channel;

  GRecMutex *recall_mutex;

  fx_analyse_channel = AGS_FX_ANALYSE_CHANNEL(gobject);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(fx_analyse_channel);

  switch(prop_id){
  case PROP_FREQUENCY:
  {
    AgsPort *port;

    port = (AgsPort *) g_value_get_object(value);

    g_rec_mutex_lock(recall_mutex);

    if(port == fx_analyse_channel->frequency){
      g_rec_mutex_unlock(recall_mutex);	

      return;
    }

    if(fx_analyse_channel->frequency != NULL){
      g_object_unref(G_OBJECT(fx_analyse_channel->frequency));
    }
      
    if(port != NULL){
      g_object_ref(G_OBJECT(port));
    }

    fx_analyse_channel->frequency = port;
      
    g_rec_mutex_unlock(recall_mutex);	
  }
  break;
  case PROP_MAGNITUDE:
  {
    AgsPort *port;

    port = (AgsPort *) g_value_get_object(value);

    g_rec_mutex_lock(recall_mutex);

    if(port == fx_analyse_channel->magnitude){
      g_rec_mutex_unlock(recall_mutex);	

      return;
    }

    if(fx_analyse_channel->magnitude != NULL){
      g_object_unref(G_OBJECT(fx_analyse_channel->magnitude));
    }
      
    if(port != NULL){
      g_object_ref(G_OBJECT(port));
    }

    fx_analyse_channel->magnitude = port;
      
    g_rec_mutex_unlock(recall_mutex);	
  }
  break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }  
}

void
ags_fx_analyse_channel_get_property(GObject *gobject,
				    guint prop_id,
				    GValue *value,
				    GParamSpec *param_spec)
{
  AgsFxAnalyseChannel *fx_analyse_channel;

  GRecMutex *recall_mutex;

  fx_analyse_channel = AGS_FX_ANALYSE_CHANNEL(gobject);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(fx_analyse_channel);

  switch(prop_id){
  case PROP_FREQUENCY:
  {
    g_rec_mutex_lock(recall_mutex);

    g_value_set_object(value, fx_analyse_channel->frequency);
      
    g_rec_mutex_unlock(recall_mutex);	
  }
  break;
  case PROP_MAGNITUDE:
  {
    g_rec_mutex_lock(recall_mutex);

    g_value_set_object(value, fx_analyse_channel->magnitude);
      
    g_rec_mutex_unlock(recall_mutex);	
  }
  break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(gobject, prop_id, param_spec);
    break;
  }
}

void
ags_fx_analyse_channel_dispose(GObject *gobject)
{
  AgsFxAnalyseChannel *fx_analyse_channel;
  
  fx_analyse_channel = AGS_FX_ANALYSE_CHANNEL(gobject);

  /* frequency */
  if(fx_analyse_channel->frequency != NULL){
    g_object_unref(G_OBJECT(fx_analyse_channel->frequency));

    fx_analyse_channel->frequency = NULL;
  }  
  

  /* magnitude */
  if(fx_analyse_channel->magnitude != NULL){
    g_object_unref(G_OBJECT(fx_analyse_channel->magnitude));

    fx_analyse_channel->magnitude = NULL;
  }  
  
  /* call parent */
  G_OBJECT_CLASS(ags_fx_analyse_channel_parent_class)->dispose(gobject);
}

void
ags_fx_analyse_channel_finalize(GObject *gobject)
{
  AgsFxAnalyseChannel *fx_analyse_channel;

  AgsResetFxAnalyse *reset_fx_analyse;

  guint i;
  
  fx_analyse_channel = AGS_FX_ANALYSE_CHANNEL(gobject);

  /* frequency */
  if(fx_analyse_channel->frequency != NULL){
    g_object_unref(G_OBJECT(fx_analyse_channel->frequency));
  }

  /* magnitude */
  if(fx_analyse_channel->magnitude != NULL){
    g_object_unref(G_OBJECT(fx_analyse_channel->magnitude));
  }

  /* input data */
  for(i = 0; i < AGS_SOUND_SCOPE_LAST; i++){
    ags_fx_analyse_channel_input_data_free(fx_analyse_channel->input_data[i]);
  }

  /* reset ags-fx-analyse task */
  reset_fx_analyse = ags_reset_fx_analyse_get_instance();
  
  ags_reset_fx_analyse_remove(reset_fx_analyse,
			      fx_analyse_channel);
  
  /* call parent */
  G_OBJECT_CLASS(ags_fx_analyse_channel_parent_class)->finalize(gobject);
}

void
ags_fx_analyse_channel_notify_samplerate_callback(GObject *gobject,
						  GParamSpec *pspec,
						  gpointer user_data)
{
  AgsFxAnalyseChannel *fx_analyse_channel;
  AgsPort *frequency;

  double correction;
  guint samplerate;
  guint buffer_size;
  guint i;
  
  GRecMutex *port_mutex;
  
  fx_analyse_channel = AGS_FX_ANALYSE_CHANNEL(gobject);

  /* get buffer size */
  frequency = NULL;
  
  samplerate = AGS_SOUNDCARD_DEFAULT_SAMPLERATE;
  buffer_size = AGS_SOUNDCARD_DEFAULT_BUFFER_SIZE;

  g_object_get(fx_analyse_channel,
	       "frequency", &frequency,
	       "samplerate", &samplerate,
	       "buffer-size", &buffer_size,
	       NULL);

  /* realloc port - frequency */
  correction = (double) samplerate / (double) buffer_size;
    
  if(frequency != NULL){
    port_mutex = AGS_PORT_GET_OBJ_MUTEX(frequency);

    g_rec_mutex_lock(port_mutex);

    for(i = 0; i < buffer_size; i++){
      frequency->port_value.ags_port_double_ptr[i] = (double) i * (double) correction;
    }
    
    g_rec_mutex_unlock(port_mutex);
  }

  /* unref */
  if(frequency != NULL){
    g_object_unref(frequency);
  }
}

void
ags_fx_analyse_channel_notify_buffer_size_callback(GObject *gobject,
						   GParamSpec *pspec,
						   gpointer user_data)
{
  AgsFxAnalyseChannel *fx_analyse_channel;
  AgsPort *frequency;
  AgsPort *magnitude;

  double correction;
  guint samplerate;
  guint buffer_size;
  guint i;
  
  GRecMutex *recall_mutex;
  GRecMutex *port_mutex;
  
  fx_analyse_channel = AGS_FX_ANALYSE_CHANNEL(gobject);

  /* get recall mutex */
  recall_mutex = AGS_RECALL_GET_OBJ_MUTEX(fx_analyse_channel);

  /* get buffer size */
  frequency = NULL;
  magnitude = NULL;

  samplerate = AGS_SOUNDCARD_DEFAULT_SAMPLERATE;
  buffer_size = AGS_SOUNDCARD_DEFAULT_BUFFER_SIZE;
  
  g_object_get(fx_analyse_channel,
	       "frequency", &frequency,
	       "magnitude", &magnitude,
	       "samplerate", &samplerate,
	       "buffer-size", &buffer_size,
	       NULL);

  /* realloc port - frequency */
  correction = (double) samplerate / (double) buffer_size;
    
  if(frequency != NULL){
    port_mutex = AGS_PORT_GET_OBJ_MUTEX(frequency);

    g_rec_mutex_lock(port_mutex);

    ags_stream_free(frequency->port_value.ags_port_double_ptr);

    if(buffer_size > 0){
      frequency->port_value.ags_port_double_ptr = ags_stream_alloc(buffer_size,
								   AGS_SOUNDCARD_DOUBLE);
    }else{
      frequency->port_value.ags_port_double_ptr = NULL;
    }

    for(i = 0; i < buffer_size; i++){
      frequency->port_value.ags_port_double_ptr[i] = (double) i * (double) correction;
    }
    
    g_rec_mutex_unlock(port_mutex);
  }

  /* realloc port - magnitude */
  if(magnitude != NULL){
    port_mutex = AGS_PORT_GET_OBJ_MUTEX(magnitude);

    g_rec_mutex_lock(port_mutex);

    ags_stream_free(fx_analyse_channel->magnitude->port_value.ags_port_double_ptr);

    if(buffer_size > 0){
      fx_analyse_channel->magnitude->port_value.ags_port_double_ptr = ags_stream_alloc(buffer_size,
										       AGS_SOUNDCARD_DOUBLE);
    }else{
      fx_analyse_channel->magnitude->port_value.ags_port_double_ptr = NULL;
    }
    
    g_rec_mutex_unlock(port_mutex);
  }
  
  /* reallocate buffer - apply buffer size */
  g_rec_mutex_lock(recall_mutex);

  for(i = 0; i < AGS_SOUND_SCOPE_LAST; i++){
    AgsFxAnalyseChannelInputData *input_data;

    input_data = fx_analyse_channel->input_data[i];

    /* FFTW */
    fftw_free(input_data->in);
    fftw_free(input_data->out);

    fftw_free(input_data->comout);

    fftw_destroy_plan(input_data->plan);
    
    if(buffer_size > 0){
      input_data->in = (double *) fftw_malloc(buffer_size * sizeof(double));
      input_data->out = (double *) fftw_malloc(buffer_size * sizeof(double));

      input_data->comout = (fftw_complex *) fftw_malloc(buffer_size * sizeof(fftw_complex));

      input_data->plan = fftw_plan_r2r_1d(buffer_size, input_data->in, input_data->out, FFTW_R2HC, FFTW_ESTIMATE);
    }else{
      input_data->in = NULL;
      input_data->out = NULL;

      input_data->comout = NULL;

      input_data->plan = NULL;
    }
  }
  
  g_rec_mutex_unlock(recall_mutex);

  /* unref */
  if(frequency != NULL){
    g_object_unref(frequency);
  }

  if(magnitude != NULL){
    g_object_unref(magnitude);
  }
}

/**
 * ags_fx_analyse_channel_input_data_alloc:
 * 
 * Allocate #AgsFxAnalyseChannelInputData-struct
 * 
 * Returns: (type gpointer) (transfer full): the new #AgsFxAnalyseChannelInputData-struct
 * 
 * Since: 3.3.0
 */
AgsFxAnalyseChannelInputData*
ags_fx_analyse_channel_input_data_alloc()
{
  AgsFxAnalyseChannelInputData *input_data;

  input_data = (AgsFxAnalyseChannelInputData *) g_malloc(sizeof(AgsFxAnalyseChannelInputData));

  input_data->parent = NULL;

  input_data->plan = NULL;
  input_data->comout = NULL;

  input_data->in = NULL;
  input_data->out = NULL;

  return(input_data);
}

/**
 * ags_fx_analyse_channel_input_data_free:
 * @input_data: (type gpointer) (transfer full): the #AgsFxAnalyseChannelInputData-struct
 * 
 * Free @input_data.
 * 
 * Since: 3.3.0
 */
void
ags_fx_analyse_channel_input_data_free(AgsFxAnalyseChannelInputData *input_data)
{
  if(input_data == NULL){
    return;
  }

  fftw_free(input_data->in);
  fftw_free(input_data->out);

  fftw_free(input_data->comout);

  fftw_destroy_plan(input_data->plan);    
  
  g_free(input_data);
}

/**
 * ags_fx_analyse_channel_new:
 * @channel: the #AgsChannel
 *
 * Create a new instance of #AgsFxAnalyseChannel
 *
 * Returns: the new #AgsFxAnalyseChannel
 *
 * Since: 3.3.0
 */
AgsFxAnalyseChannel*
ags_fx_analyse_channel_new(AgsChannel *channel)
{
  AgsFxAnalyseChannel *fx_analyse_channel;

  fx_analyse_channel = (AgsFxAnalyseChannel *) g_object_new(AGS_TYPE_FX_ANALYSE_CHANNEL,
							    "source", channel,
							    NULL);

  return(fx_analyse_channel);
}