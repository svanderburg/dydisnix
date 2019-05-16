#include "portassign.h"
#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>

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
    "      --output-xml             Specifies that the output should be in XML not the\n"
    "                               Nix expression language\n"
    "  -h, --help                   Shows the usage of this command to the user\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"services", required_argument, 0, DYDISNIX_OPTION_SERVICES},
        {"infrastructure", required_argument, 0, DYDISNIX_OPTION_INFRASTRUCTURE},
        {"distribution", required_argument, 0, DYDISNIX_OPTION_DISTRIBUTION},
        {"ports", required_argument, 0, DYDISNIX_OPTION_PORTS},
        {"service-property", required_argument, 0, DYDISNIX_OPTION_SERVICE_PROPERTY},
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"output-xml", no_argument, 0, DYDISNIX_OPTION_OUTPUT_XML},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    char *ports = NULL;
    char *service_property = "portAssign";
    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:p:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DYDISNIX_OPTION_SERVICES:
                services = optarg;
                break;
            case DYDISNIX_OPTION_INFRASTRUCTURE:
                infrastructure = optarg;
                break;
            case DYDISNIX_OPTION_DISTRIBUTION:
                distribution = optarg;
                break;
            case DYDISNIX_OPTION_PORTS:
                ports = optarg;
                break;
            case DYDISNIX_OPTION_SERVICE_PROPERTY:
                service_property = optarg;
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

    if(!check_services_option(services)
      || !check_infrastructure_option(infrastructure)
      || !check_distribution_option(distribution))
        return 1;

    /* Execute operation */
    portassign(services, infrastructure, distribution, ports, service_property, flags);
    return 0;
}
