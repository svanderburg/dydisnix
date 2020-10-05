#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>
#include "visualize-infra.h"

static void print_usage(const char *command)
{
    printf("Usage: %s --infrastructure infrastructure_expr [OPTION]\n\n", command);

    puts(
    "Creates a visualization of the in which machines and hosted containers on the\n"
    "machine visualized. The output is generated in dot format and can be converted\n"
    "to an image by using the `dot' tool.\n\n"

    "Options:\n"
    "  -i, --infrastructure=infrastructure_nix\n"
    "                               Infrastructure Nix expression which captures\n"
    "                               properties of machines in the network\n"
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
        {"infrastructure", required_argument, 0, DYDISNIX_OPTION_INFRASTRUCTURE},
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    char *infrastructure = NULL;
    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "i:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
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

    if(!check_infrastructure_option(infrastructure))
        return 1;

    /* Execute visualize operation */
    return visualize_infra(infrastructure, flags);
}
