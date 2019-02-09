#include "filterbuildable.h"
#include <stdio.h>
#include <getopt.h>

static void print_usage(const char *command)
{
    printf("Usage: %s -s services_nix -i infrastructure_nix -d distribution_nix --distribution-xml distribution_xml\n\n", command);

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
    "      --distribution-xml=distribution_xml\n"
    "                               XML representation of a configuration mapping\n"
    "                               services to machines\n"
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
        {"distribution-xml", required_argument, 0, 'D'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    char *distribution_xml = NULL;

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
            case 'D':
                distribution_xml = optarg;
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

    if(services == NULL)
    {
        fprintf(stderr, "A service expression must be specified!\n");
        return 1;
    }

    if(infrastructure == NULL)
    {
        fprintf(stderr, "An infrastructure expression must be specified!\n");
        return 1;
    }

    if(distribution == NULL)
    {
        fprintf(stderr, "A distribution expression must be specified!\n");
        return 1;
    }

    if(distribution_xml == NULL)
    {
        fprintf(stderr, "A distribution XML file must be specified!\n");
        return 1;
    }

    /* Execute operation */
    return filter_buildable(services, infrastructure, distribution, distribution_xml);
}
