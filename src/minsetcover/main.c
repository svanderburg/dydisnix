#include "minsetcover.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <checkoptions.h>

static void print_usage(const char *command)
{
    printf("Usage: %s --services services --infrastructure infrastructure --distribution distribution --target-property targetProperty\n\n", command);

    puts(
    "Divides services over machines by using an approximation algorithm for the\n"
    "minimum set cover problem.\n\n"

    "This algorithm is useful to generate a cost effective deployment in which the\n"
    "amount of machines is minimized.\n\n"

    "Options:\n"
    "  -s, --services=services_nix\n"
    "                           Services Nix expression which describes all\n"
    "                           components of the distributed system\n"
    "  -i, --infrastructure=infrastructure_nix\n"
    "                           Infrastructure Nix expression which captures\n"
    "                           properties of machines in the network\n"
    "  -d, --distribution=distribution_nix\n"
    "                           Distribution Nix expression which maps services\n"
    "                           to machines in the network\n"
    "      --target-property=targetProperty\n"
    "                           Specifies which target property stores the total\n"
    "                           capacity\n"
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
        {"services", required_argument, 0, 's'},
        {"infrastructure", required_argument, 0, 'i'},
        {"distribution", required_argument, 0, 'd'},
        {"target-property", required_argument, 0, 'T'},
        {"xml", no_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    char *target_property = NULL;
    int xml = FALSE;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 's':
                services = optarg;
                break;
            case 'i':
                infrastructure = optarg;
                break;
            case 'd':
                distribution = optarg;
                break;
            case 'T':
                target_property = optarg;
                break;
            case 'x':
                xml = TRUE;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
        }
    }

    /* Validate options */

    if(!check_services_option(services)
      || !check_infrastructure_option(infrastructure)
      || !check_distribution_option(distribution)
      || !check_target_property_option(target_property))
        return 1;

    /* Execute operation */
    return minsetcover(services, infrastructure, distribution, target_property, xml);
}
