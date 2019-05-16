#include "graphcol.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <checkoptions.h>

#define TRUE 1
#define FALSE 0

static void print_usage(const char *command)
{
    printf("Usage: %s --services services_nix --infrastructure infrastructure_nix\n\n", command);

    puts(
    "Divides services over machines in the network using an approximation\n"
    "algorithm for the graph coloring problem.\n\n"

    "This algorithm is useful to generate deployments in which every inter-dependency\n"
    "is a real network link using a minimum amount of machines.\n\n"

    "Options:\n"
    "  -s, --services=services_nix\n"
    "                           Services Nix expression which describes all\n"
    "                           components of the distributed system\n"
    "  -i, --infrastructure=infrastructure_nix\n"
    "                           Infrastructure Nix expression which captures\n"
    "                           properties of machines in the network\n"
    "      --xml                Specifies that the configurations are in XML not the\n"
    "                           Nix expression language.\n"
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
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DYDISNIX_OPTION_SERVICES:
                services = optarg;
                break;
            case DYDISNIX_OPTION_INFRASTRUCTURE:
                infrastructure = optarg;
                break;
            case DYDISNIX_OPTION_XML:
                flags |= DYDISNIX_FLAG_XML;
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
      || !check_infrastructure_option(infrastructure))
        return 1;

    /* Execute operation */
    return graphcol(services, infrastructure, flags);
}
