#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>
#include "visualize-services.h"

static void print_usage(const char *command)
{
    printf("Usage: %s --services services_expr [OPTION]\n", command);
    printf(" or: %s --batch --services services_expr --output-dir [OPTION]\n\n", command);

    puts(
    "Creates a visualization of the services model, in which services are generated\n"
    "as nodes and inter-dependencies as arrows. The output is generated in dot format\n"
    "and can be converted to an image by using the `dot' tool.\n\n"

    "Options:\n"
    "  -s, --services=services_expr Services configuration which describes all\n"
    "                               components of the distributed system\n"
    "      --xml                    Specifies that the configurations are in XML not\n"
    "                               the Nix expression language.\n"
    "      --group                  Only displays services and dependencies belonging\n"
    "                               to a group\n"
    "      --group-subservices      Merges all services belonging to a sub group into\n"
    "                               a single node\n"
    "      --output-dir             Specifies directory in which the batch mode\n"
    "                               outputs are stored (default to current directory)\n"
    "  -f, --format=FORMAT          Image format to use for the outputs (e.g. svg or\n"
    "                               png)\n"
    "      --extra-params           A string with an attribute set in the Nix\n"
    "                               expression language propagating extra parameters\n"
    "                               to the input models\n"
    "  -h, --help                   Shows the usage of this command to the user\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"batch", no_argument, 0, DYDISNIX_OPTION_BATCH},
        {"services", required_argument, 0, DYDISNIX_OPTION_SERVICES},
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"group-subservices", no_argument, 0, DYDISNIX_OPTION_GROUP_SUBSERVICES},
        {"group", required_argument, 0, DYDISNIX_OPTION_GROUP},
        {"output-dir", required_argument, 0, DYDISNIX_OPTION_OUTPUT_DIR},
        {"format", required_argument, 0, DYDISNIX_OPTION_FORMAT},
        {"extra-params", required_argument, 0, DYDISNIX_OPTION_EXTRA_PARAMS},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    unsigned int flags = 0;
    char *group = "";
    int batch = DYDISNIX_DEFAULT_BATCH;
    char *output_dir = ".";
    char *image_format = NULL;
    char *extra_params = "{}";

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "f:s:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DYDISNIX_OPTION_BATCH:
                batch = TRUE;
                break;
            case DYDISNIX_OPTION_SERVICES:
                services = optarg;
                break;
            case DYDISNIX_OPTION_XML:
                flags |= DYDISNIX_FLAG_XML;
                break;
            case DYDISNIX_OPTION_GROUP_SUBSERVICES:
                flags |= DYDISNIX_FLAG_GROUP_SUBSERVICES;
                break;
            case DYDISNIX_OPTION_GROUP:
                group = optarg;
                break;
            case DYDISNIX_OPTION_OUTPUT_DIR:
                output_dir = optarg;
                break;
            case DYDISNIX_OPTION_FORMAT:
                image_format = optarg;
                break;
            case DYDISNIX_OPTION_EXTRA_PARAMS:
                extra_params = optarg;
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

    if(!check_services_option(services))
        return 1;

    /* Execute visualize operation */
    if(batch)
        return visualize_services_batch(services, flags, output_dir, image_format, extra_params);
    else
        return visualize_services(services, flags, group, extra_params);
}
