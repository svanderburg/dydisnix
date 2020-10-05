#include "visualize-infra.h"
#include <stdio.h>
#include <nixxml-ghashtable-iter.h>
#include <checkoptions.h>
#include <targetstable2.h>

static void visualize_containers(FILE *file, gchar *host_name, GHashTable *containers_table)
{
    fprintf(file, "subgraph cluster_container_%s {\n", host_name);
    fprintf(file, "style=filled;\n");
    fprintf(file, "label=\"containers\"\n");
    fprintf(file, "fillcolor=grey40\n");
    fprintf(file, "node [shape=record,style=filled,color=black,fillcolor=white]\n");

    if(g_hash_table_size(containers_table) == 0)
        fprintf(file, "\"%s:\" [ label = \"(no defined containers)\" ]\n", host_name);
    else
    {
        NixXML_GHashTableOrderedIter iter;
        gchar *container_name;
        gpointer value;

        NixXML_g_hash_table_ordered_iter_init(&iter, containers_table);

        while(NixXML_g_hash_table_ordered_iter_next(&iter, &container_name, &value))
            fprintf(file, "\"%s:%s\" [ label = \"%s\" ]\n", host_name, container_name, container_name);

        NixXML_g_hash_table_ordered_iter_destroy(&iter);
    }

    fprintf(file, "}\n");
}

static void visualize_infrastructure_model(FILE *file, GHashTable *targets_table)
{
    NixXML_GHashTableOrderedIter iter;
    gchar *host_name;
    gpointer value;

    fprintf(file, "digraph G {\n");

    NixXML_g_hash_table_ordered_iter_init(&iter, targets_table);

    while(NixXML_g_hash_table_ordered_iter_next(&iter, &host_name, &value))
    {
        Target *target = (Target*)value;

        fprintf(file, "subgraph cluster_target_%s {\n", host_name);
        fprintf(file, "style=filled;\n");
        fprintf(file, "label=\"%s\"\n", host_name);
        fprintf(file, "fillcolor=grey\n");

        visualize_containers(file, host_name, target->containers_table);

        fprintf(file, "}\n");
    }

    fprintf(file, "}\n");

    NixXML_g_hash_table_ordered_iter_destroy(&iter);
}

int visualize_infra(gchar *infrastructure, const unsigned int flags)
{
    GHashTable *targets_table = create_targets_table2(infrastructure, flags & DYDISNIX_FLAG_XML);

    if(targets_table == NULL)
    {
        g_printerr("Cannot open infrastructure XML file!\n");
        return 1;
    }
    else
    {
        visualize_infrastructure_model(stdout, targets_table);
        delete_targets_table(targets_table);
        return 0;
    }
}
