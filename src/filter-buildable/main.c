#include "filterbuildable.h"
#include <stdio.h>
#include <getopt.h>
#define _GNU_SOURCE

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --services services.nix --infrastructure infrastructure.nix --distribution distribution.nix --distribution-xml distribution.xml\n", command);
    fprintf(stderr, "%s {-h | --help}\n", command);
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
	{"distribution-xml", required_argument, 0, 'D'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    char *distribution_xml = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:h", long_options, &option_index)) != -1)
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
	    case 'D':
		distribution_xml = optarg;
		break;
	    case 'h':
		print_usage(argv[0]);
		return 0;
	}
    }
    
    /* Validate options */
    
    if(services == NULL)
    {
	fprintf(stderr, "A service expression must be specified!\n");
	return 1;
    }

    if(infrastructure == NULL)
    {
	fprintf(stderr, "An infrastructure expression must be specified!\n");
	return 1;
    }
    
    if(distribution == NULL)
    {
	fprintf(stderr, "A distribution expression must be specified!\n");
	return 1;
    }
    
    if(distribution_xml == NULL)
    {
	fprintf(stderr, "A distribution XML file must be specified!\n");
	return 1;
    }

    /* Execute operation */
    filter_buildable(services, infrastructure, distribution, distribution_xml);
    return 0;
}
