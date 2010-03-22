#include <stdio.h>
#include <glib.h>
#include <pulse/pulseaudio.h>

int verbose = 0;
const char *APP_NAME = "paquery";

void check_and_display(const char* name, pa_proplist *proplist, GList *properties) {
  if (match(proplist, properties)) {
    // result to stdout
    if (verbose) {
      printf("[%s]\n", name);
      printf("%s\n", pa_proplist_to_string(proplist));
    }
    else {
      printf("%s\n", name);
    }
  }
}

static void pa_card_info_cb(pa_context *c, const pa_card_info*i, int eol, void *userdata) {
  if (!eol)
    check_and_display(i->name, i->proplist, (GList*) userdata);
}

static void pa_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
  if (!eol)
    check_and_display(i->name, i->proplist, (GList*) userdata);
}

static void pa_source_info_cb(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
  if (!eol)
    check_and_display(i->name, i->proplist, (GList*) userdata);
}

static void pa_client_info_cb(pa_context *c, const pa_client_info*i, int eol, void *userdata) {
  if (!eol)
    check_and_display(i->name, i->proplist, (GList*) userdata);
}


typedef struct {
  const char *name;
  const char *value;
} NameValuePair;

int match(pa_proplist *proplist, GList *properties) {
  if (g_list_length(properties) == 0)
    return FALSE;

  while (properties = g_list_next(properties)) {
    NameValuePair *p = (NameValuePair *) properties->data;
    const char *value = pa_proplist_gets(proplist, p->name);
    if (!value || !g_str_equal(value, p->value)) {
      return FALSE;
    }
  }
  return TRUE;
}

enum QueryType {
  QUERY_TYPE_CARD,
  QUERY_TYPE_SINK,
  QUERY_TYPE_SOURCE,
  QUERY_TYPE_CLIENT
};

void usage() {
  fprintf(stderr, "\nUsage: %s [options] [query_type] [<propery=value> ...]\n\n", APP_NAME);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "    -v       Verbose: list all properties for each match\n\n");
  fprintf(stderr, "Query Type (exactly one required):\n");
  fprintf(stderr, "    card     List name of matching cards\n");
  fprintf(stderr, "    sink     List name of matching sinks\n");
  fprintf(stderr, "    source   List name of matching sources\n");
  fprintf(stderr, "    client   List name of matching clients\n\n");
  fprintf(stderr, "The remainder of the arguments are taken as name-value pairs\n");
  fprintf(stderr, "which must be separated by an equals sign. If the value contains,\n");
  fprintf(stderr, "spaces, it must be quoted to be considered a single value.\n\n");

}
// Listable things:
//   modules
//   sinks
//   sources
//   clients
//   sink-inputs
//   source-outputs
//   cards


int main(int argc, char **argv) {
  pa_mainloop *pa_ml;
  pa_mainloop_api *pa_ml_api;
  pa_context *c;
  pa_operation *op;
  GList *properties;
  int type = -1;
  int i;

  if (argc < 2) {
    usage(argv[0]);
    return 100;
  }

  // Options end _after_ the first non-option arg (which is type)
  // and the remaining arguments are interpreted as (name=value)
  // pairs for properties to match

  i = 1;
  while (i < argc && type == -1) {
    char *s = argv[i];
    if (s[0] == '-') {
      if (s[1] == 'v') {
        verbose = 1;
      } else {
        fprintf(stderr, "Error: unknown option (%s)\n", s);
        usage();
        return EXIT_FAILURE;
      }
    } else {
      // The first non-option arg is taken as the query type
      if (g_str_equal(s, "card")) {
        type = QUERY_TYPE_CARD;
      } else if (g_str_equal(s, "sink")) {
        type = QUERY_TYPE_SINK;
      } else if (g_str_equal(s, "source")) {
        type = QUERY_TYPE_SOURCE;
      } else if (g_str_equal(s, "client")) {
        type = QUERY_TYPE_CLIENT;
      } else {
        fprintf(stderr, "Error! Invalid query type (%s), must be one of [card, sink, source]\n", s);
        usage();
        return EXIT_FAILURE;
      }
    }
    i++;
  }

  properties = g_list_alloc();

  // Collect properties from remaining command line args (if any)
  for (; i < argc; i++) {
    const char *name = argv[i];
    char *value = g_strrstr(name, "=");
    if (value) {
      // split at equals
      *value = '\0';
      value++;
      // assign to the list
      NameValuePair *prop = (NameValuePair *) g_malloc(sizeof(NameValuePair));
      prop->name = name;
      prop->value = value;
      properties = g_list_append(properties, prop);
    }
  }

  pa_ml = pa_mainloop_new();
  pa_ml_api = pa_mainloop_get_api(pa_ml);
  c = pa_context_new(pa_ml_api, APP_NAME);
  pa_context_connect(c, NULL, PA_CONTEXT_NOFLAGS, NULL);

  // Wait for connection
  while (pa_context_get_state(c) < PA_CONTEXT_READY) {
    pa_mainloop_iterate(pa_ml, 1, NULL);
  }

  // Check for failed connection
  if (pa_context_get_state(c) != PA_CONTEXT_READY) {
    return EXIT_FAILURE;
  }

  // do the operation
  switch (type) {
  case QUERY_TYPE_CARD:
    op = pa_context_get_card_info_list(c, pa_card_info_cb, properties);
    break;
  case QUERY_TYPE_SINK:
    op = pa_context_get_sink_info_list(c, pa_sink_info_cb, properties);
    break;
  case QUERY_TYPE_SOURCE:
    op = pa_context_get_source_info_list(c, pa_source_info_cb, properties);
    break;
  case QUERY_TYPE_CLIENT:
    op = pa_context_get_client_info_list(c, pa_client_info_cb, properties);
    break;
  }

  // step through events
  while (pa_context_get_state(c) == PA_CONTEXT_READY && pa_operation_get_state(op)
          != PA_OPERATION_DONE) {
    pa_mainloop_iterate(pa_ml, 1, NULL);
  }

  // All done with this (should also free structs, but oh well)
  g_list_free(properties);

  pa_context_disconnect(c);

  // cleanup
  pa_context_unref(c);
  pa_mainloop_free(pa_ml);

  return EXIT_SUCCESS;
}
