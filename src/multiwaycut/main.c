#include <stdio.h>
#include <getopt.h>
#include "multiwaycut.h"

static void print_usage(const char *command)
{
    printf("Usage: %s distribution_xml\n\n", command);

    puts(
    "Divides all services in the distribution model by using an approximation\n"
    "algorithm for the multiway cut problem.\n\n"

    "This algorithm is useful to generate a distribution in which the amount of\n"
    "network links is minimized.\n\n"

    "Options:\n"
    "  -h, --help               Shows the usage of this command to the user\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
        }
    }

    /* Validate options */

    if(optind >= argc)
    {
        fprintf(stderr, "ERROR: No distribution XML specified!\n");
        return 1;
    }
    else
    {
        /* Execute operation */
        return multiwaycut(argv[optind]);
    }
}
