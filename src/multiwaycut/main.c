#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>
#include "multiwaycut.h"

static void print_usage(const char *command)
{
    printf("Usage: %s [--infrastructure infrastructure_nix] --distribution distribution_nix\n\n", command);

    puts(
    "Divides all services in the distribution model by using an approximation\n"
    "algorithm for the multiway cut problem.\n\n"

    "This algorithm is useful to generate a distribution in which the amount of\n"
    "network links is minimized.\n\n"

    "Options:\n"
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
    "  -h, --help               Shows the usage of this command to the user\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"infrastructure", required_argument, 0, DYDISNIX_OPTION_INFRASTRUCTURE},
        {"distribution", required_argument, 0, DYDISNIX_OPTION_DISTRIBUTION},
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"output-xml", no_argument, 0, DYDISNIX_OPTION_OUTPUT_XML},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *infrastructure = NULL;
    char *distribution = NULL;
    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "i:d:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
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
                flags |= DYDISNIX_FLAG_OUTPUT_XML;
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
      || !check_distribution_option(distribution))
        return 1;

    /* Execute operation */
    return multiwaycut(distribution, infrastructure, flags);
}
