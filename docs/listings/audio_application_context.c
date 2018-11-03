#include <glib.h>
#include <glib-object.h>

#include <ags/libags.h>
#include <ags/libags-audio.h>

AgsApplicationContext *application_context;
AgsThread *task_thread;

application_context = (AgsApplicationContext *) ags_audio_application_context_new();
task_thread = ags_concurrency_provider_get_task_thread(AGS_CONCURRENCY_PROVIDER(application_context));
