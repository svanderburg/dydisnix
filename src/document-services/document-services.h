#ifndef __DYDISNIX_VISUALIZE_SERVICES_H
#define __DYDISNIX_VISUALIZE_SERVICES_H
#include <glib.h>
#include <checkoptions.h>

int document_services(gchar *services, gchar *group, const unsigned int flags, gchar *docs, gchar *extra_params);

int document_services_batch(gchar *services, const unsigned int flags, gchar *output_dir, gchar *image_format, gchar *docs, gchar *extra_params);

#endif
