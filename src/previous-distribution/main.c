#include "generate-previous-distribution.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <checkoptions.h>

static void print_usage(const char *command)
{
    printf("Usage: %s MANIFEST\n\n", command);

    puts(
    "Generates a distribution model from a manifest file.\n\n"

    "Options:\n"
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
        {"output-xml", no_argument, 0, DYDISNIX_OPTION_OUTPUT_XML},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
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
    if(optind >= argc)
    {
        fprintf(stderr, "ERROR: No manifest specified!\n");
        return 1;
    }

    /* Execute operation */
    return generate_previous_distribution(argv[optind], flags);
}
