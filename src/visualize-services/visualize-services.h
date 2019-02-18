#ifndef __DYDISNIX_VISUALIZE_SERVICES_H
#define __DYDISNIX_VISUALIZE_SERVICES_H
#include <glib.h>

int visualize_services(gchar *services, int xml, int group_subservices, gchar *group);

int visualize_services_batch(gchar *services, int xml, int group_subservices, gchar *output_dir, gchar *image_format);

#endif
