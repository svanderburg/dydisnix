#include "idassign.h"
#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>

static void print_usage(const char *command)
{
    printf(
    "Usage: %s --services services_expr --infrastructure infrastructure_expr --distribution distribution_expr --id-resources id_resources_nix [OPTION]\n"
    " or: %s --services services_expr --id-resources id_resources_nix\n\n",
    command, command);

    puts(
    "Assigns unique IDs to services based on their ID requirements and ID\n"
    "resources configuration.\n\n"

    "Options:\n"
    "  -s, --services=services_expr Services configuration which describes all\n"
    "                               components of the distributed system\n"
    "  -i, --infrastructure=infrastructure_expr\n"
    "                               Infrastructure configuration which captures\n"
    "                               properties of machines in the network\n"
    "  -d, --distribution=distribution_expr\n"
    "                               Distribution configuration which maps services\n"
    "                               to machines in the network\n"
    "      --id-resources=id_resources_nix\n"
    "                               ID resources Nix expression defining numeric ID\n"
    "                               resources\n"
    "      --ids=ids_nix            IDs Nix expression mapping services to unique\n"
    "                               IDs per resource\n"
    "      --service-property=serviceProperty\n"
    "                               Property in service model that specifies which\n"
    "                               numeric ID resources a service needs (Defaults\n"
    "                               to: requireUniqueIdsFor)\n"
    "      --output-file            Specifies the file where to write the IDS to.\n"
    "                               If no file was provided, it writes to the\n"
    "                               standard output\n"
    "      --xml                    Specifies that the configurations are in XML not\n"
    "                               the Nix expression language.\n"
    "      --output-xml             Specifies that the output should be in XML not the\n"
    "                               Nix expression language\n"
    "  -h, --help                   Shows the usage of this command to the user\n"
    );
}

static int check_id_resources_option(const char *id_resources)
{
    if(id_resources == NULL)
    {
        fprintf(stderr, "An ID resources expression must be specified!\n");
        return FALSE;
    }
    else
        return TRUE;
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"services", required_argument, 0, DYDISNIX_OPTION_SERVICES},
        {"infrastructure", required_argument, 0, DYDISNIX_OPTION_INFRASTRUCTURE},
        {"distribution", required_argument, 0, DYDISNIX_OPTION_DISTRIBUTION},
        {"id-resources", required_argument, 0, DYDISNIX_OPTION_ID_RESOURCES},
        {"ids", required_argument, 0, DYDISNIX_OPTION_IDS},
        {"service-property", required_argument, 0, DYDISNIX_OPTION_SERVICE_PROPERTY},
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"output-xml", no_argument, 0, DYDISNIX_OPTION_OUTPUT_XML},
        {"output-file", required_argument, 0, DYDISNIX_OPTION_OUTPUT},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    char *id_resources = NULL;
    char *ids = NULL;
    char *service_property = "requiresUniqueIdsFor";
    char *output_file = NULL;

    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:p:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DYDISNIX_OPTION_SERVICES:
                services = optarg;
                break;
            case DYDISNIX_OPTION_INFRASTRUCTURE:
                infrastructure = optarg;
                break;
            case DYDISNIX_OPTION_DISTRIBUTION:
                distribution = optarg;
                break;
            case DYDISNIX_OPTION_ID_RESOURCES:
                id_resources = optarg;
                break;
            case DYDISNIX_OPTION_IDS:
                ids = optarg;
                break;
            case DYDISNIX_OPTION_SERVICE_PROPERTY:
                service_property = optarg;
                break;
            case DYDISNIX_OPTION_XML:
                flags |= DYDISNIX_FLAG_XML;
                break;
            case DYDISNIX_OPTION_OUTPUT_XML:
                flags |= DYDISNIX_FLAG_OUTPUT_XML;
                break;
            case DYDISNIX_OPTION_OUTPUT:
                output_file = optarg;
                break;
            case DYDISNIX_OPTION_HELP:
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    /* Validate options */

    if(!check_services_option(services)
      || !check_id_resources_option(id_resources))
        return 1;

    /* Execute operation */
    return idassign(services, infrastructure, distribution, id_resources, ids, service_property, output_file, flags);
}
