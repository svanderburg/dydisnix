#include <stdio.h>
#include <getopt.h>
#include "document-services.h"

static void print_usage(const char *command)
{
    printf("Usage: %s --services services_expr [OPTION]\n\n", command);

    puts(
    "Creates an overview of services and displays relevant information about them,\n"
    "such as their description and type.\n\n"

    "Options:\n"
    "  -s, --services=services_expr Services configuration which describes all\n"
    "                               components of the distributed system\n"
    "      --xml                    Specifies that the configurations are in XML not\n"
    "                               the Nix expression language.\n"
    "      --group                  Only displays services and dependencies belonging\n"
    "                               to a group\n"
    "      --group-subservices      Merges all services belonging to a sub group into\n"
    "                               a single node\n"
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
    return document_services(services, group, xml, group_subservices);
}
