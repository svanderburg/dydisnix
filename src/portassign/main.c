#include "portassign.h"
#include <stdio.h>
#include <getopt.h>

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --services services_expr --infrastructure infrastructure_expr --distribution distribution_expr --ports ports_expr --service-property serviceProperty [--xml]\n", command);
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
            case '?':
                print_usage(argv[0]);
                return 0;
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
