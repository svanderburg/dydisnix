#include "divide.h"
#include <stdio.h>
#include <getopt.h>
#define _GNU_SOURCE
#include <string.h>

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --strategy strategy --services-xml services.xml --infrastructure-xml infrastructure.xml --distribution-xml distribution.xml --service-property serviceProperty --target-property targetProperty\n", command);
    fprintf(stderr, "%s {-h | --help}\n", command);
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
    divide(strategy, services_xml, infrastructure_xml, distribution_xml, service_property, target_property);
    return 0;
}
