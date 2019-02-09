#include <stdio.h>
#include <getopt.h>
#include "visualize-services.h"

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --services services_expr [--xml] [--group GROUP] [--group-subservices]\n", command);
    fprintf(stderr, "%s {-h | --help}\n", command);
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"services", required_argument, 0, 's'},
        {"xml", no_argument, 0, 'x'},
        {"group-subservices", no_argument, 0, 'G'},
        {"group", required_argument, 0, 'g'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    int xml = FALSE;
    int group_subservices = FALSE;
    char *group = "";

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 's':
                services = optarg;
                break;
            case 'x':
                xml = TRUE;
                break;
            case 'G':
                group_subservices = TRUE;
                break;
            case 'g':
                group = optarg;
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
        fprintf(stderr, "A services model file must be specified!\n");
        return 1;
    }

    /* Execute visualize operation */
    return visualize_services(services, xml, group_subservices, group);
}
