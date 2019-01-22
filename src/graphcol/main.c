#include "graphcol.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s --services-xml services.xml --infrastructure-xml infrastructure.xml\n", command);
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
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *services_xml = NULL;
    char *infrastructure_xml = NULL;
    
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
    
    /* Execute operation */
    return graphcol(services_xml, infrastructure_xml);
}
