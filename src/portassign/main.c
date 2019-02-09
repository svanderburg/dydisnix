#include "portassign.h"
#include <stdio.h>
#include <getopt.h>

static void print_usage(const char *command)
{
    printf("Usage: %s --services services_expr --infrastructure infrastructure_expr --distribution distribution_expr --ports ports_expr [OPTION]\n\n", command);

    puts(
    "Assigns unique port numbers to services based on their distribution to machines\n"
    "in the network.\n\n"

    "Options:\n"
    "  -s, --services=services_expr Services configuration which describes all\n"
    "                               components of the distributed system\n"
    "  -i, --infrastructure=infrastructure_expr\n"
    "                               Infrastructure configuration which captures\n"
    "                               properties of machines in the network\n"
    "  -d, --distribution=distribution_expr\n"
    "                               Distribution configuration which maps services\n"
    "                               to machines in the network\n"
    "  -p, --ports=ports_expr       Ports configuration specifies the current port\n"
    "                               assignments\n"
    "      --service-property=serviceProperty\n"
    "                               Property in service model that specifies the port\n"
    "                               assignment. (Defaults to: portAssign)\n"
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
        {"ports", required_argument, 0, 'p'},
        {"service-property", required_argument, 0, 'S'},
        {"xml", no_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    char *ports = NULL;
    char *service_property = "portAssign";
    int xml = FALSE;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:p:h", long_options, &option_index)) != -1)
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
            case 'p':
                ports = optarg;
                break;
            case 'S':
                service_property = optarg;
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
        fprintf(stderr, "A services model file must be specified!\n");
        return 1;
    }

    if(infrastructure == NULL)
    {
        fprintf(stderr, "An infrastructure model file must be specified!\n");
        return 1;
    }

    if(distribution == NULL)
    {
        fprintf(stderr, "A distribution model file must be specified!\n");
        return 1;
    }

    /* Execute operation */
    portassign(services, infrastructure, distribution, ports, service_property, xml);
    return 0;
}
