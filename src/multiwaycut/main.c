#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>
#include "multiwaycut.h"

static void print_usage(const char *command)
{
    printf("Usage: %s --services services_expr [--infrastructure infrastructure_nix] --distribution distribution_nix\n\n", command);

    puts(
    "Divides all services in the distribution model by using an approximation\n"
    "algorithm for the multiway cut problem.\n\n"

    "This algorithm is useful to generate a distribution in which the amount of\n"
    "network links between machines is minimized.\n\n"

    "Options:\n"
    "  -s, --services=services_nix\n"
    "                           Services Nix expression which describes all\n"
    "                           components of the distributed system\n"
    "  -i, --infrastructure=infrastructure_nix\n"
    "                           Infrastructure Nix expression which captures\n"
    "                           properties of machines in the network\n"
    "  -d, --distribution=distribution_nix\n"
    "                           Distribution Nix expression which maps services to\n"
    "                           machines in the network\n"
    "      --xml                Specifies that the configurations are in XML not the\n"
    "                           Nix expression language.\n"
    "      --output-xml         Specifies that the output should be in XML not the\n"
    "                           Nix expression language\n"
    "      --output-graph       Specifies that the output should be a representation\n"
    "                           of the problem graph in the DOT language\n"
    "      --output-resolved-graph\n"
    "                           Specifies that the output should be a representation\n"
    "                           of the resolved problem graph in the DOT language\n"
    "  -h, --help               Shows the usage of this command to the user\n"
    );
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
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"output-xml", no_argument, 0, DYDISNIX_OPTION_OUTPUT_XML},
        {"output-graph", no_argument, 0, DYDISNIX_OPTION_OUTPUT_GRAPH},
        {"output-resolved-graph", no_argument, 0, DYDISNIX_OPTION_OUTPUT_RESOLVED_GRAPH},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    unsigned int flags = 0;
    OutputArtifactType artifact_type = ARTIFACT_NIX;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:h", long_options, &option_index)) != -1)
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
            case DYDISNIX_OPTION_XML:
                flags |= DYDISNIX_FLAG_XML;
                break;
            case DYDISNIX_OPTION_OUTPUT_XML:
                artifact_type = ARTIFACT_XML;
                break;
            case DYDISNIX_OPTION_OUTPUT_GRAPH:
                artifact_type = ARTIFACT_GRAPH;
                break;
            case DYDISNIX_OPTION_OUTPUT_RESOLVED_GRAPH:
                artifact_type = ARTIFACT_RESOLVED_GRAPH;
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

    if((!(flags & DYDISNIX_FLAG_XML) && !check_infrastructure_option(infrastructure))
      || !check_services_option(services)
      || !check_distribution_option(distribution))
        return 1;

    /* Execute operation */
    return multiwaycut(services, distribution, infrastructure, flags, artifact_type);
}
