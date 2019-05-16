#ifndef __DYDISNIX_VISUALIZE_SERVICES_H
#define __DYDISNIX_VISUALIZE_SERVICES_H
#include <glib.h>
#include <checkoptions.h>

int visualize_services(gchar *services, const unsigned int flags, gchar *group);

int visualize_services_batch(gchar *services, const unsigned int flags, gchar *output_dir, gchar *image_format);

#endif
