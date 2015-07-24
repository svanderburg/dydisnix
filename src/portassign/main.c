#include "portassign.h"
#include <stdio.h>
#include <getopt.h>

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --services-xml services.xml --infrastructure-xml infrastructure.xml --distribution-xml distribution.xml --ports-xml ports.xml --service-property serviceProperty\n", command);
    fprintf(stderr, "%s {-h | --help}\n", command);
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"services-xml", required_argument, 0, 's'},
        {"infrastructure-xml", required_argument, 0, 'i'},
        {"distribution-xml", required_argument, 0, 'd'},
        {"ports-xml", required_argument, 0, 'p'},
        {"service-property", required_argument, 0, 'S'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    char *services_xml = NULL;
    char *infrastructure_xml = NULL;
    char *distribution_xml = NULL;
    char *ports_xml = NULL;
    char *service_property = "portAssign";
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case 's':
                services_xml = optarg;
                break;
            case 'i':
                infrastructure_xml = optarg;
                break;
            case 'd':
                distribution_xml = optarg;
                break;
            case 'p':
                ports_xml = optarg;
                break;
            case 'S':
                service_property = optarg;
                break;
            case 'h':
            case '?':
                print_usage(argv[0]);
                return 0;
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
    
    /* Execute operation */
    portassign(services_xml, infrastructure_xml, distribution_xml, ports_xml, service_property);
    return 0;
}
