#include <stdio.h>
#include <getopt.h>
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
    "  -h, --help               Shows the usage of this command to the user\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"infrastructure", required_argument, 0, 'i'},
        {"distribution", required_argument, 0, 'd'},
        {"xml", no_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *infrastructure = NULL;
    char *distribution = NULL;
    int xml = FALSE;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "i:d:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
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

    if(!xml && infrastructure == NULL) /* A distribution Nix expression requires an infrastructure Nix expression. XML representation of a distribution mapping is self-contained */
    {
        fprintf(stderr, "An infrastructure configuration file must be specified!\n");
        return 1;
    }

    if(distribution == NULL)
    {
        fprintf(stderr, "A distribution configuration file must be specified!\n");
        return 1;
    }

    /* Execute operation */
    return multiwaycut(distribution, infrastructure, xml);
}
