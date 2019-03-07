#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>
#include "document-services.h"

static void print_usage(const char *command)
{
    printf("Usage: %s --services services_expr [OPTION]\n", command);
    printf(" or: %s --batch --services services_expr --output-dir [OPTION]\n\n", command);

    puts(
    "Creates an overview of services and displays relevant information about them,\n"
    "such as their description and type.\n\n"

    "Options:\n"
    "  -s, --services=services_expr Services configuration which describes all\n"
    "                               components of the distributed system\n"
    "      --docs=docs_expr         Documentation configuration that specifies\n"
    "                               generation settings\n"
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
        {"docs", required_argument, 0, DYDISNIX_OPTION_DOCS},
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"group-subservices", no_argument, 0, DYDISNIX_OPTION_GROUP_SUBSERVICES},
        {"group", required_argument, 0, DYDISNIX_OPTION_GROUP},
        {"output-dir", required_argument, 0, DYDISNIX_OPTION_OUTPUT_DIR},
        {"format", required_argument, 0, DYDISNIX_OPTION_FORMAT},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *docs = NULL;
    int xml = DYDISNIX_DEFAULT_XML;
    int group_subservices = DYDISNIX_DEFAULT_GROUP_SUBSERVICES;
    char *group = "";
    int batch = DYDISNIX_DEFAULT_BATCH;
    char *output_dir = ".";
    char *image_format = NULL;

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
            case DYDISNIX_OPTION_DOCS:
                docs = optarg;
                break;
            case DYDISNIX_OPTION_XML:
                xml = TRUE;
                break;
            case DYDISNIX_OPTION_GROUP_SUBSERVICES:
                group_subservices = TRUE;
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
        return document_services_batch(services, xml, group_subservices, output_dir, image_format, docs);
    else
        return document_services(services, group, xml, group_subservices, docs);
}
