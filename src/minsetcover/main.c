#include "minsetcover.h"
#include <stdio.h>
#include <getopt.h>
#define _GNU_SOURCE
#include <string.h>

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --services-xml services.xml --infrastructure-xml infrastructure.xml --distribution-xml distribution.xml --target-property targetProperty\n", command);
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
	{"target-property", required_argument, 0, 'T'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *services_xml = NULL;
    char *infrastructure_xml = NULL;
    char *distribution_xml = NULL;
    char *target_property = NULL;
    
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
    
    if(target_property == NULL)
    {
	fprintf(stderr, "An target property must be specified!\n");
	return 1;
    }
    
    /* Execute operation */
    minsetcover(services_xml, infrastructure_xml, distribution_xml, target_property);
    return 0;
}
