#include "divide.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <checkoptions.h>

static void print_usage(const char *command)
{
    printf("Usage: %s --strategy STRATEGY --services services_expr --infrastructure infrastructure_expr --distribution distribution_expr --service-property serviceProperty --target-property targetProperty\n\n", command);

    puts(
    "Divides services over candidate targets using a one dimensional division strategy.\n\n"

    "Options:\n"
    "      --strategy=STRATEGY  Specifies the division strategy to use. Currently the\n"
    "                           supported strategies are: 'greedy', 'highest-bidder',\n"
    "                           and 'lowest-bidder'\n"
    "  -s, --services=services_nix\n"
    "                           Services Nix expression which describes all\n"
    "                           components of the distributed system\n"
    "  -i, --infrastructure=infrastructure_nix\n"
    "                           Infrastructure Nix expression which captures\n"
    "                           properties of machines in the network\n"
    "  -d, --distribution=distribution_nix\n"
    "                           Distribution Nix expression which maps services to\n"
    "                           machines in the network\n"
    "      --service-property=serviceProperty\n"
    "                           Specifies which service property contains a capacity\n"
    "      --target-property=targetProperty\n"
    "                           Specifies which target property stores the total\n"
    "                           capacity\n"
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
        {"strategy", required_argument, 0, DYDISNIX_OPTION_STRATEGY},
        {"services", required_argument, 0, DYDISNIX_OPTION_SERVICES},
        {"infrastructure", required_argument, 0, DYDISNIX_OPTION_INFRASTRUCTURE},
        {"distribution", required_argument, 0, DYDISNIX_OPTION_DISTRIBUTION},
        {"service-property", required_argument, 0, DYDISNIX_OPTION_SERVICE_PROPERTY},
        {"target-property", required_argument, 0, DYDISNIX_OPTION_TARGET_PROPERTY},
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"output-xml", no_argument, 0, DYDISNIX_OPTION_OUTPUT_XML},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    char *service_property = NULL;
    char *target_property = NULL;
    Strategy strategy = STRATEGY_NONE;
    unsigned int flags = 0;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DYDISNIX_OPTION_STRATEGY:
                if(strcmp(optarg, "greedy") == 0)
                    strategy = STRATEGY_GREEDY;
                else if(strcmp(optarg, "highest-bidder") == 0)
                    strategy = STRATEGY_HIGHEST_BIDDER;
                else if(strcmp(optarg, "lowest-bidder") == 0)
                    strategy = STRATEGY_LOWEST_BIDDER;
                else
                {
                    fprintf(stderr, "Unknown strategy: %s\n", optarg);
                    return 1;
                }
                break;
            case DYDISNIX_OPTION_SERVICES:
                services = optarg;
                break;
            case DYDISNIX_OPTION_INFRASTRUCTURE:
                infrastructure = optarg;
                break;
            case DYDISNIX_OPTION_DISTRIBUTION:
                distribution = optarg;
                break;
            case DYDISNIX_OPTION_SERVICE_PROPERTY:
                service_property = optarg;
                break;
            case DYDISNIX_OPTION_TARGET_PROPERTY:
                target_property = optarg;
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
      || !check_distribution_option(distribution)
      || !check_service_mapping_property_option(service_property)
      || !check_target_mapping_property_option(target_property))
        return 1;

    /* Execute operation */
    return divide(strategy, services, infrastructure, distribution, service_property, target_property, flags);
}
