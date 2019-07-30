#include "generate-previous-distribution.h"
#include <glib.h>
#include <manifest.h>
#include <servicemappingarray.h>
#include <checkoptions.h>
#include "candidatetargetmappingtable.h"

static GHashTable *convert_manifest_to_candidate_target_table(Manifest *manifest)
{
    unsigned int i;
    GHashTable *candidate_target_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

    for(i = 0; i < manifest->service_mapping_array->len; i++)
    {
        ServiceMapping *mapping = (ServiceMapping*)g_ptr_array_index(manifest->service_mapping_array, i);
        ManifestService *service = (ManifestService*)g_hash_table_lookup(manifest->services_table, mapping->service);
        GPtrArray *targets = g_hash_table_lookup(candidate_target_table, (gchar*)service->name);
        CandidateTargetMapping *target_mapping = (CandidateTargetMapping*)g_malloc(sizeof(CandidateTargetMapping));

        if(targets == NULL)
        {
            targets = g_ptr_array_new();
            g_hash_table_insert(candidate_target_table, (gchar*)service->name, targets);
        }

        target_mapping->target = mapping->target;
        target_mapping->container = mapping->container;

        g_ptr_array_add(targets, target_mapping);
    }

    return candidate_target_table;
}

int generate_previous_distribution(char *manifest_file, const unsigned int flags)
{
    Manifest *manifest = create_manifest(manifest_file, MANIFEST_ALL_FLAGS, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("The provided manifest cannot be opened!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_manifest(manifest))
        {
            GHashTable *candidate_target_table = convert_manifest_to_candidate_target_table(manifest);

            /* Print the result */
            if(flags & DYDISNIX_FLAG_OUTPUT_XML)
                print_candidate_target_table_xml(candidate_target_table);
            else
                print_candidate_target_table_nix(candidate_target_table);

            g_hash_table_destroy(candidate_target_table);

            exit_status = 0;
        }
        else
            exit_status = 1;

        delete_manifest(manifest);
        return exit_status;
    }
}
