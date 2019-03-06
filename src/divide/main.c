#include "divide.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>

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
    "  -h, --help               Shows the usage of this command to the user\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"strategy", required_argument, 0, 'A'},
        {"services", required_argument, 0, 's'},
        {"infrastructure", required_argument, 0, 'i'},
        {"distribution", required_argument, 0, 'd'},
        {"service-property", required_argument, 0, 'S'},
        {"target-property", required_argument, 0, 'T'},
        {"xml", no_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    char *service_property = NULL;
    char *target_property = NULL;
    Strategy strategy = STRATEGY_NONE;
    int xml = FALSE;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 'A':
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
            case 's':
                services = optarg;
                break;
            case 'i':
                infrastructure = optarg;
                break;
            case 'd':
                distribution = optarg;
                break;
            case 'S':
                service_property = optarg;
                break;
            case 'T':
                target_property = optarg;
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
        fprintf(stderr, "A services expression must be specified!\n");
        return 1;
    }

    if(infrastructure == NULL)
    {
        fprintf(stderr, "An infrastructure configuration file must be specified!\n");
        return 1;
    }

    if(distribution == NULL)
    {
        fprintf(stderr, "A distribution configuration file must be specified!\n");
        return 1;
    }

    if(service_property == NULL)
    {
        fprintf(stderr, "A service property must be specified!\n");
        return 1;
    }

    if(target_property == NULL)
    {
        fprintf(stderr, "An target property must be specified!\n");
        return 1;
    }

    /* Execute operation */
    return divide(strategy, services, infrastructure, distribution, service_property, target_property, xml);
}
