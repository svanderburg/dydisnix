#include "graphcol.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>

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
        {"services", required_argument, 0, 's'},
        {"infrastructure", required_argument, 0, 'i'},
        {"xml", no_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    int xml = FALSE;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 's':
                services = optarg;
                break;
            case 'i':
                infrastructure = optarg;
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

    if(services == NULL)
    {
        fprintf(stderr, "A services XML file must be specified!\n");
        return 1;
    }

    if(infrastructure == NULL)
    {
        fprintf(stderr, "An infrastructure XML file must be specified!\n");
        return 1;
    }

    /* Execute operation */
    return graphcol(services, infrastructure, xml);
}
