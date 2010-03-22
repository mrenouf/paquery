#include <stdio.h>
#include <stdlib.h>

#include <glib-2.0/glib.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

static void pa_success_cb(pa_context *c, int success, void *userdata) {
	printf("Set sink %s to base volume\n", (char *) userdata);
}

static void pa_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	pa_cvolume *norm = pa_cvolume_reset((pa_cvolume *)&(i->volume), 2);
	pa_context_set_sink_volume_by_index(c, i->index, norm, pa_success_cb, (void *)i->name);
	printf("Sink: %s\n", i->name);
}

int main(void) {
	printf("Hi!\n");
	pa_glib_mainloop *pa_glib = pa_glib_mainloop_new(NULL);
	pa_mainloop_api *pa_mainloop = pa_glib_mainloop_get_api(pa_glib);
	pa_context *c = pa_context_new(pa_mainloop, "pautil");

	pa_context_get_sink_info_list(c, pa_sink_info_cb, NULL);

	pa_glib_mainloop_free(pa_glib);
	return EXIT_SUCCESS;
}
