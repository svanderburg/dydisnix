#ifndef __DYDISNIX_DOCS_CONFIG_H
#define __DYDISNIX_DOCS_CONFIG_H
#include <glib.h>

typedef struct
{
    GHashTable *groups;

    GPtrArray *fields;

    GHashTable *descriptions;
}
DocsConfig;

char *generate_docs_xml_from_expr(char *docs_expr);

DocsConfig *create_docs_config_from_xml(gchar *docs_xml_file);

DocsConfig *create_docs_config_from_nix(gchar *docs_nix);

DocsConfig *create_docs_config(gchar *docs, const int xml);

void delete_docs_config(DocsConfig *docs_config);

gchar *find_group(const DocsConfig *docs_config, gchar *name);

gchar *find_description(const DocsConfig *docs_config, gchar *name);

#endif
