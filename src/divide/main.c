#include "divide.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>

static void print_usage(const char *command)
{
    printf("Usage: %s --strategy STRATEGY --services-xml services_xml --infrastructure-xml infrastructure_xml --distribution-xml distribution_xml --service-property serviceProperty --target-property targetProperty\n\n", command);

    puts(
    "Divides services over candidate targets using a one dimensional division strategy.\n\n"

    "Options:\n"
    "      --strategy=STRATEGY  Specifies the division strategy to use. Currently the\n"
    "                           supported strategies are: 'greedy', 'highest-bidder',\n"
    "                           and 'lowest-bidder'\n"
    "      --services-xml=services_xml\n"
    "                           XML representation of a configuration describing the\n"
    "                           properties of the services\n"
    "      --infrastructure-xml=infrastructure_xml\n"
    "                           XML representation of a configuration describing the\n"
    "                           available machines and their properties\n"
    "      --distribution-xml=distribution_xml\n"
    "                           XML representation of a configuration mapping\n"
    "                           services to machines\n"
    "      --service-property=serviceProperty\n"
    "                           Specifies which service property contains a capacity\n"
    "      --target-property=targetProperty\n"
    "                           Specifies which target property stores the total\n"
    "                           capacity\n"
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
        {"services-xml", required_argument, 0, 's'},
        {"infrastructure-xml", required_argument, 0, 'i'},
        {"distribution-xml", required_argument, 0, 'd'},
        {"service-property", required_argument, 0, 'S'},
        {"target-property", required_argument, 0, 'T'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services_xml = NULL;
    char *infrastructure_xml = NULL;
    char *distribution_xml = NULL;
    char *service_property = NULL;
    char *target_property = NULL;
    Strategy strategy = STRATEGY_NONE;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
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
                services_xml = optarg;
                break;
            case 'i':
                infrastructure_xml = optarg;
                break;
            case 'd':
                distribution_xml = optarg;
                break;
            case 'S':
                service_property = optarg;
                break;
            case 'T':
                target_property = optarg;
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

    if(services_xml == NULL)
    {
        fprintf(stderr, "A services XML file must be specified!\n");
        return 1;
    }

    if(infrastructure_xml == NULL)
    {
        fprintf(stderr, "An infrastructure XML file must be specified!\n");
        return 1;
    }

    if(distribution_xml == NULL)
    {
        fprintf(stderr, "A distribution XML file must be specified!\n");
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
    return divide(strategy, services_xml, infrastructure_xml, distribution_xml, service_property, target_property);
}
