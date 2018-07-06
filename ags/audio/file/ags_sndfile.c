/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
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

#include <ags/audio/file/ags_sndfile.h>

#include <ags/libags.h>

#include <ags/audio/ags_playable.h>
#include <ags/audio/ags_audio_buffer_util.h>

#include <string.h>

#include <ags/i18n.h>

void ags_sndfile_class_init(AgsSndfileClass *sndfile);
void ags_sndfile_connectable_interface_init(AgsConnectableInterface *connectable);
void ags_sndfile_playable_interface_init(AgsPlayableInterface *playable);
void ags_sndfile_init(AgsSndfile *sndfile);
void ags_sndfile_finalize(GObject *gobject);

void ags_sndfile_connect(AgsConnectable *connectable);
void ags_sndfile_disconnect(AgsConnectable *connectable);

gboolean ags_sndfile_open(AgsPlayable *playable, gchar *name);
gboolean ags_sndfile_rw_open(AgsPlayable *playable, gchar *name,
			     gboolean create,
			     guint samplerate, guint channels,
			     guint frames,
			     guint format);
guint ags_sndfile_level_count(AgsPlayable *playable);
gchar** ags_sndfile_sublevel_names(AgsPlayable *playable);
void ags_sndfile_iter_start(AgsPlayable *playable);
gboolean ags_sndfile_iter_next(AgsPlayable *playable);
void ags_sndfile_info(AgsPlayable *playable,
		      guint *channels, guint *frames,
		      guint *loop_start, guint *loop_end,
		      GError **error);
guint ags_sndfile_get_samplerate(AgsPlayable *playable);
guint ags_sndfile_get_format(AgsPlayable *playable);
double* ags_sndfile_read(AgsPlayable *playable, guint channel, GError **error);
int* ags_sndfile_read_int(AgsPlayable *playable, guint channel, GError **error);
void ags_sndfile_write(AgsPlayable *playable, double *buffer, guint buffer_length);
void ags_sndfile_write_int(AgsPlayable *playable, int *buffer, guint buffer_length);
void ags_sndfile_flush(AgsPlayable *playable);
void ags_sndfile_seek(AgsPlayable *playable, guint frames, gint whence);
void ags_sndfile_close(AgsPlayable *playable);

sf_count_t ags_sndfile_vio_get_filelen(void *user_data);
sf_count_t ags_sndfile_vio_seek(sf_count_t offset, int whence, void *user_data);
sf_count_t ags_sndfile_vio_read(void *ptr, sf_count_t count, void *user_data);
sf_count_t ags_sndfile_vio_write(const void *ptr, sf_count_t count, void *user_data);
sf_count_t ags_sndfile_vio_tell(const void *ptr, sf_count_t count, void *user_data);

/**
 * SECTION:ags_sndfile
 * @short_description: Libsndfile wrapper
 * @title: AgsSndfile
 * @section_id:
 * @include: ags/audio/file/ags_sndfile.h
 *
 * #AgsSndfile is the base object to ineract with libsndfile.
 */

static gpointer ags_sndfile_parent_class = NULL;
static AgsConnectableInterface *ags_sndfile_parent_connectable_interface;
static AgsPlayableInterface *ags_sndfile_parent_playable_interface;
static SF_VIRTUAL_IO *ags_sndfile_virtual_io = NULL;

GType
ags_sndfile_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_sndfile;

    static const GTypeInfo ags_sndfile_info = {
      sizeof (AgsSndfileClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_sndfile_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsSndfile),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_sndfile_init,
    };

    static const GInterfaceInfo ags_connectable_interface_info = {
      (GInterfaceInitFunc) ags_sndfile_connectable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    static const GInterfaceInfo ags_playable_interface_info = {
      (GInterfaceInitFunc) ags_sndfile_playable_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };

    ags_type_sndfile = g_type_register_static(G_TYPE_OBJECT,
					      "AgsSndfile",
					      &ags_sndfile_info,
					      0);

    g_type_add_interface_static(ags_type_sndfile,
				AGS_TYPE_CONNECTABLE,
				&ags_connectable_interface_info);

    g_type_add_interface_static(ags_type_sndfile,
				AGS_TYPE_PLAYABLE,
				&ags_playable_interface_info);

    g_once_init_leave (&g_define_type_id__volatile, ags_type_sndfile);
  }

  return g_define_type_id__volatile;
}

void
ags_sndfile_class_init(AgsSndfileClass *sndfile)
{
  GObjectClass *gobject;

  ags_sndfile_parent_class = g_type_class_peek_parent(sndfile);

  gobject = (GObjectClass *) sndfile;

  gobject->finalize = ags_sndfile_finalize;

  /* sndfile callbacks */
  if(ags_sndfile_virtual_io == NULL){
    ags_sndfile_virtual_io = (SF_VIRTUAL_IO *) malloc(sizeof(SF_VIRTUAL_IO));

    ags_sndfile_virtual_io->get_filelen = ags_sndfile_vio_get_filelen;
    ags_sndfile_virtual_io->seek = ags_sndfile_vio_seek;
    ags_sndfile_virtual_io->read = ags_sndfile_vio_read;
    ags_sndfile_virtual_io->write = ags_sndfile_vio_write;
    ags_sndfile_virtual_io->tell = ags_sndfile_vio_tell;
  }
}

void
ags_sndfile_connectable_interface_init(AgsConnectableInterface *connectable)
{
  ags_sndfile_parent_connectable_interface = g_type_interface_peek_parent(connectable);

  connectable->connect = ags_sndfile_connect;
  connectable->disconnect = ags_sndfile_disconnect;
}

void
ags_sndfile_playable_interface_init(AgsPlayableInterface *playable)
{
  ags_sndfile_parent_playable_interface = g_type_interface_peek_parent(playable);

  playable->open = ags_sndfile_open;
  playable->rw_open = ags_sndfile_rw_open;

  playable->level_count = ags_sndfile_level_count;
  playable->sublevel_names = ags_sndfile_sublevel_names;
  playable->level_select = NULL;

  playable->iter_start = ags_sndfile_iter_start;
  playable->iter_next = ags_sndfile_iter_next;

  playable->info = ags_sndfile_info;

  playable->get_samplerate = ags_sndfile_get_samplerate;
  playable->get_format = ags_sndfile_get_format;

  playable->read = ags_sndfile_read;
  playable->read_int = ags_sndfile_read_int;

  playable->write = ags_sndfile_write;
  playable->write_int = ags_sndfile_write_int;
  playable->flush = ags_sndfile_flush;

  playable->seek = ags_sndfile_seek;

  playable->close = ags_sndfile_close;
}

void
ags_sndfile_init(AgsSndfile *sndfile)
{
  sndfile->flags = 0;

  sndfile->info = NULL;
  sndfile->file = NULL;

  sndfile->pointer = NULL;
  sndfile->current = NULL;
  sndfile->length = 0;

  sndfile->buffer = NULL;
}

void
ags_sndfile_connect(AgsConnectable *connectable)
{
  ags_sndfile_parent_connectable_interface->connect(connectable);

  /* empty */
}

void
ags_sndfile_disconnect(AgsConnectable *connectable)
{
  ags_sndfile_parent_connectable_interface->disconnect(connectable);

  /* empty */
}

gboolean
ags_sndfile_open(AgsPlayable *playable, gchar *name)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  sndfile->info = (SF_INFO *) malloc(sizeof(SF_INFO));
  sndfile->info->format = 0;
  sndfile->info->channels = 0;
  sndfile->info->samplerate = 0;

  if((AGS_SNDFILE_VIRTUAL & (sndfile->flags)) == 0){
    if(name != NULL){
      sndfile->file = (SNDFILE *) sf_open(name, SFM_READ, sndfile->info);
    }
  }else{
    sndfile->file = (SNDFILE *) sf_open_virtual(ags_sndfile_virtual_io, SFM_READ, sndfile->info, sndfile);
  }

#ifdef AGS_DEBUG
  g_message("ags_sndfile_open(): channels %d frames %d", sndfile->info->channels, sndfile->info->frames);
#endif
  
  if(sndfile->file == NULL){
    return(FALSE);
  }else{
    return(TRUE);
  }
}

gboolean
ags_sndfile_rw_open(AgsPlayable *playable, gchar *name,
		    gboolean create,
		    guint samplerate, guint channels,
		    guint frames,
		    guint format)
{
  AgsSndfile *sndfile;
  //  sf_count_t multi_frames;
  
  sndfile = AGS_SNDFILE(playable);

  sndfile->info = (SF_INFO *) malloc(sizeof(SF_INFO));

  sndfile->info->samplerate = (int) samplerate;
  sndfile->info->channels = (int) channels;
  sndfile->info->format = (int) format;
  sndfile->info->frames = 0;
  sndfile->info->seekable = 0;
  sndfile->info->sections = 0;

  g_message("export to: %s\n  samplerate: %d\n  channels: %d\n  format: %x",
	    name,
	    samplerate,
	    channels,
	    format);

  if(!sf_format_check(sndfile->info)){
    g_warning("invalid format");
  }

  if((AGS_SNDFILE_VIRTUAL & (sndfile->flags)) == 0){
    if(name != NULL){
      sndfile->file = (SNDFILE *) sf_open(name, SFM_RDWR, sndfile->info);
    }
  }else{
    sndfile->file = (SNDFILE *) sf_open_virtual(ags_sndfile_virtual_io, SFM_RDWR, sndfile->info, sndfile);
  }

  //  multi_frames = frames * sndfile->info->channels;
  //  sf_command(sndfile->file, SFC_FILE_TRUNCATE, &(multi_frames), sizeof(multi_frames));
  //  sf_command (sndfile, SFC_SET_SCALE_INT_FLOAT_WRITE, NULL, SF_TRUE);
  //  sf_seek(sndfile->file, 0, SEEK_SET);

  //  sndfile->info->frames = multi_frames;

  if(sndfile->file == NULL){
    return(FALSE);
  }else{
    return(TRUE);
  }
}

guint
ags_sndfile_level_count(AgsPlayable *playable)
{
  return(1);
}

gchar**
ags_sndfile_sublevel_names(AgsPlayable *playable)
{
  return(NULL);
}

void
ags_sndfile_iter_start(AgsPlayable *playable)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  sndfile->flags |= AGS_SNDFILE_ITER_START;
}

gboolean
ags_sndfile_iter_next(AgsPlayable *playable)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  if((AGS_SNDFILE_ITER_START & (sndfile->flags)) != 0){
    sndfile->flags &= (~AGS_SNDFILE_ITER_START);

    return(TRUE);
  }else{
    return(FALSE);
  }
}

void
ags_sndfile_info(AgsPlayable *playable,
		 guint *channels, guint *frames,
		 guint *loop_start, guint *loop_end,
		 GError **error)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  *channels = sndfile->info->channels;
  *frames = sndfile->info->frames;
  *loop_start = 0;
  *loop_end = 0;
}

guint
ags_sndfile_get_samplerate(AgsPlayable *playable)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  return(sndfile->info->samplerate);
}

guint
ags_sndfile_get_format(AgsPlayable *playable)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  switch(((SF_FORMAT_PCM_S8 |
	   SF_FORMAT_PCM_16 |
	   SF_FORMAT_PCM_24 |
	   SF_FORMAT_PCM_32 |
	   SF_FORMAT_FLOAT |
	   SF_FORMAT_DOUBLE ) & sndfile->info->format)){
  case SF_FORMAT_PCM_S8:
    return(AGS_AUDIO_BUFFER_UTIL_S8);
  case SF_FORMAT_PCM_16:
    return(AGS_AUDIO_BUFFER_UTIL_S16);
  case SF_FORMAT_PCM_24:
    return(AGS_AUDIO_BUFFER_UTIL_S24);
  case SF_FORMAT_PCM_32:
    return(AGS_AUDIO_BUFFER_UTIL_S32);
  case SF_FORMAT_FLOAT:
    return(AGS_AUDIO_BUFFER_UTIL_FLOAT);
  case SF_FORMAT_DOUBLE:
    return(AGS_AUDIO_BUFFER_UTIL_DOUBLE);
  default:
    g_warning("ags_sndfile_get_format() - unknown format");
    return(0);
  }
}

double*
ags_sndfile_read(AgsPlayable *playable, guint channel, GError **error)
{
  AgsSndfile *sndfile;
  double *buffer, *source;
  guint i;
  guint num_read;

  sndfile = AGS_SNDFILE(playable);

  if(sndfile->buffer == NULL){
    sndfile->buffer = 
      source = (double *) malloc((size_t) sndfile->info->channels *
				  sndfile->info->frames *
				  sizeof(double));
    ags_audio_buffer_util_clear_double(source, sndfile->info->channels,
				       sndfile->info->frames);
    
    //FIXME:JK: work-around comment me out
    //    sf_seek(sndfile->file, 0, SEEK_SET);  
    num_read = sf_read_double(sndfile->file, source, sndfile->info->frames * sndfile->info->channels);

    if(num_read != sndfile->info->frames * sndfile->info->channels){
      g_warning("ags_sndfile_read(): read to many items");
    }
  }else{
    source = sndfile->buffer;
  }

  if(sndfile->info->frames != 0){
    buffer = (double *) malloc((size_t) sndfile->info->frames *
			       sizeof(double));
    ags_audio_buffer_util_clear_double(buffer, 1,
				       sndfile->info->frames);
    
    ags_audio_buffer_util_copy_double_to_double(buffer, 1,
						&(source[channel]), sndfile->info->channels,
						sndfile->info->frames);
  }else{
    buffer = NULL;
  }
  
  return(buffer);
}

int*
ags_sndfile_read_int(AgsPlayable *playable, guint channel, GError **error)
{
  AgsSndfile *sndfile;
  int *buffer, *source;
  guint i;
  guint num_read;

  sndfile = AGS_SNDFILE(playable);

  if(sndfile->buffer == NULL){
    sndfile->buffer = 
      source = (int *) malloc((size_t) sndfile->info->channels *
			      sndfile->info->frames *
			      sizeof(int));
    if(sizeof(int) == 8){
      ags_audio_buffer_util_clear_buffer(source, sndfile->info->channels,
					 sndfile->info->frames, AGS_AUDIO_BUFFER_UTIL_S64);
    }else{
      ags_audio_buffer_util_clear_buffer(source, sndfile->info->channels,
	sndfile->info->frames, AGS_AUDIO_BUFFER_UTIL_S32);
    }
    
    //FIXME:JK: work-around comment me out
    //    sf_seek(sndfile->file, 0, SEEK_SET);  
    num_read = sf_read_int(sndfile->file, source, sndfile->info->frames * sndfile->info->channels);

    if(num_read != sndfile->info->frames * sndfile->info->channels){
      g_warning("ags_sndfile_read(): read to many items");
    }
  }else{
    source = sndfile->buffer;
  }

  if(sndfile->info->frames != 0){
    buffer = (int *) malloc((size_t) sndfile->info->frames *
			    sizeof(int));
    if(sizeof(int) == 8){
      ags_audio_buffer_util_clear_buffer(buffer, 1,
	sndfile->info->frames, AGS_AUDIO_BUFFER_UTIL_S64);
    }else{
      ags_audio_buffer_util_clear_buffer(buffer, 1,
					 sndfile->info->frames, AGS_AUDIO_BUFFER_UTIL_S32);
    }
    
    if(sizeof(int) == 8){
    ags_audio_buffer_util_copy_s64_to_s64(buffer, 1,
					  &(source[channel]), sndfile->info->channels,
					  sndfile->info->frames);
  }else{
      ags_audio_buffer_util_copy_s32_to_s32(buffer, 1,
	&(source[channel]), sndfile->info->channels,
	sndfile->info->frames);
    }
  }else{
    buffer = NULL;
  }
  
  return(buffer);
}

void
ags_sndfile_write(AgsPlayable *playable, double *buffer, guint buffer_length)
{
  AgsSndfile *sndfile;
  sf_count_t multi_frames, retval;

  sndfile = AGS_SNDFILE(playable);

  multi_frames = buffer_length * sndfile->info->channels;

  retval = sf_write_double(sndfile->file, buffer, multi_frames);

  if(retval > multi_frames){
    g_warning("retval > multi_frames");
    //    sf_seek(sndfile->file, (multi_frames - retval), SEEK_CUR);
  }
}

void
ags_sndfile_write_int(AgsPlayable *playable, int *buffer, guint buffer_length)
{
  AgsSndfile *sndfile;
  sf_count_t multi_frames, retval;

  sndfile = AGS_SNDFILE(playable);

  multi_frames = buffer_length * sndfile->info->channels * sizeof(int);

  retval = sf_write_int(sndfile->file, buffer, multi_frames);

  if(retval > multi_frames){
    g_warning("retval > multi_frames");
    //    sf_seek(sndfile->file, (multi_frames - retval), SEEK_CUR);
  }
}

void
ags_sndfile_flush(AgsPlayable *playable)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  sf_write_sync(sndfile->file);
}

void
ags_sndfile_seek(AgsPlayable *playable, guint frames, gint whence)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  sf_seek(sndfile->file, frames, whence);
}

void
ags_sndfile_close(AgsPlayable *playable)
{
  AgsSndfile *sndfile;

  sndfile = AGS_SNDFILE(playable);

  sf_close(sndfile->file);
  free(sndfile->info);
}

void
ags_sndfile_finalize(GObject *gobject)
{
  G_OBJECT_CLASS(ags_sndfile_parent_class)->finalize(gobject);

  /* empty */
}

sf_count_t
ags_sndfile_vio_get_filelen(void *user_data)
{
  return(AGS_SNDFILE(user_data)->length);
}

sf_count_t
ags_sndfile_vio_seek(sf_count_t offset, int whence, void *user_data)
{
  switch(whence){
  case SEEK_CUR:
    AGS_SNDFILE(user_data)->current += offset;
    break;
  case SEEK_SET:
    AGS_SNDFILE(user_data)->current = &(AGS_SNDFILE(user_data)->pointer[offset]);
    break;
  case SEEK_END:
    AGS_SNDFILE(user_data)->current = &(AGS_SNDFILE(user_data)->pointer[AGS_SNDFILE(user_data)->length - offset]);
    break;
  }

  return(AGS_SNDFILE(user_data)->current - AGS_SNDFILE(user_data)->pointer);
}

sf_count_t
ags_sndfile_vio_read(void *ptr, sf_count_t count, void *user_data)
{
  guchar *retval;

  retval = memcpy(ptr, AGS_SNDFILE(user_data)->current, count * sizeof(guchar));

  return(retval - AGS_SNDFILE(user_data)->pointer);
}

sf_count_t
ags_sndfile_vio_write(const void *ptr, sf_count_t count, void *user_data)
{
  guchar *retval;

  retval = memcpy(AGS_SNDFILE(user_data)->current, ptr, count * sizeof(guchar));

  return(retval - AGS_SNDFILE(user_data)->pointer);
}

sf_count_t
ags_sndfile_vio_tell(const void *ptr, sf_count_t count, void *user_data)
{
  return(AGS_SNDFILE(user_data)->current - AGS_SNDFILE(user_data)->pointer);
}

/**
 * ags_sndfile_new:
 *
 * Creates an #AgsSndfile.
 *
 * Returns: an empty #AgsSndfile.
 *
 * Since: 1.0.0
 */
AgsSndfile*
ags_sndfile_new()
{
  AgsSndfile *sndfile;

  sndfile = (AgsSndfile *) g_object_new(AGS_TYPE_SNDFILE,
					NULL);

  return(sndfile);
}
