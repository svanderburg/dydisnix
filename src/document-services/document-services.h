#ifndef __DYDISNIX_VISUALIZE_SERVICES_H
#define __DYDISNIX_VISUALIZE_SERVICES_H
#include <glib.h>

int document_services(gchar *services, gchar *group, int xml, int group_subservices, gchar *docs);

int document_services_batch(gchar *services, int xml, int group_subservices, gchar *output_dir, gchar *image_format, gchar *docs);

#endif