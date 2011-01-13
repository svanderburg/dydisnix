#include <stdio.h>
#include <getopt.h>
#define _GNU_SOURCE
#include "multiwaycut.h"

static void print_usage(char *command)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s distribution.xml\n", command);
    fprintf(stderr, "%s {-h | --help}\n", command);
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'h':
		print_usage(argv[0]);
		return 0;
	}
    }
    
    /* Validate options */
    
    if(optind >= argc)
    {
	fprintf(stderr, "ERROR: No distribution XML specified!\n");
	return 1;
    }
    else
    {
	/* Execute operation */
	return multiwaycut(argv[optind]);
    }
}
