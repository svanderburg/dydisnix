#include "filterbuildable.h"
#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>

#define TRUE 1
#define FALSE 0

static void print_usage(const char *command)
{
    printf("Usage: %s -s services_nix -i infrastructure_nix -d distribution_nix\n\n", command);

    puts(
    "Evaluates all derivations in the provided Disnix configuration files and filters\n"
    "out the services that are not buildable.\n\n"

    "Options:\n"
    "  -s, --services=services_nix  Services Nix expression which describes all\n"
    "                               components of the distributed system\n"
    "  -i, --infrastructure=infrastructure_nix\n"
    "                               Infrastructure Nix expression which captures\n"
    "                               properties of machines in the network\n"
    "  -d, --distribution=distribution_nix\n"
    "                               Distribution Nix expression which maps services\n"
    "                               to machines in the network\n"
    "      --xml                    Specifies that the configurations are in XML not\n"
    "                               the Nix expression language.\n"
    "  -h, --help                   Shows the usage of this command to the user\n"
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
        {"xml", no_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
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
      || !check_distribution_option(distribution))
        return 1;

    /* Execute operation */
    return filter_buildable(services, infrastructure, distribution, xml);
}
