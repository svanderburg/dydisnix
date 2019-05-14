#include "filterbuildable.h"
#include <stdio.h>
#include <getopt.h>
#include <checkoptions.h>
#include <defaultoptions.h>

#define TRUE 1
#define FALSE 0

static void print_usage(const char *command)
{
    printf("Usage: %s -s services_nix -i infrastructure_nix -d distribution_nix\n\n", command);

    puts(
    "Evaluates all derivations in the provided Disnix configuration files and filters\n"
    "out the services that are not buildable.\n\n"

    "Options:\n"
    "  -s, --services=services_nix  Services Nix expression which describes all\n"
    "                               components of the distributed system\n"
    "  -i, --infrastructure=infrastructure_nix\n"
    "                               Infrastructure Nix expression which captures\n"
    "                               properties of machines in the network\n"
    "  -d, --distribution=distribution_nix\n"
    "                               Distribution Nix expression which maps services\n"
    "                               to machines in the network\n"
    "      --xml                    Specifies that the configurations are in XML not\n"
    "                               the Nix expression language.\n"
    "      --target-property=PROP   The target property of an infrastructure model,\n"
    "                               that specifies how to connect to the remote Disnix\n"
    "                               interface. (Defaults to hostname)\n"
    "      --interface=INTERFACE    Path to executable that communicates with a Disnix\n"
    "                               interface. Defaults to: disnix-ssh-client\n"
    "  -h, --help                   Shows the usage of this command to the user\n"

    "\nEnvironment:\n"
    "  DISNIX_CLIENT_INTERFACE    Sets the client interface (defaults to:\n"
    "                             disnix-ssh-client)\n"
    "  DISNIX_TARGET_PROPERTY     Sets the target property of an infrastructure\n"
    "                             model, that specifies how to connect to the remote\n"
    "                             Disnix interface. (Defaults to: hostname)\n"
    );
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
        {"services", required_argument, 0, DYDISNIX_OPTION_SERVICES},
        {"infrastructure", required_argument, 0, DYDISNIX_OPTION_INFRASTRUCTURE},
        {"distribution", required_argument, 0, DYDISNIX_OPTION_DISTRIBUTION},
        {"interface", required_argument, 0, DYDISNIX_OPTION_INTERFACE},
        {"target-property", required_argument, 0, DYDISNIX_OPTION_TARGET_PROPERTY},
        {"xml", no_argument, 0, DYDISNIX_OPTION_XML},
        {"help", no_argument, 0, DYDISNIX_OPTION_HELP},
        {0, 0, 0, 0}
    };
    char *services = NULL;
    char *infrastructure = NULL;
    char *distribution = NULL;
    int xml = DYDISNIX_DEFAULT_XML;
    char *interface = NULL;
    char *target_property = NULL;

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "s:i:d:h", long_options, &option_index)) != -1)
    {
        switch(c)
        {
            case DYDISNIX_OPTION_SERVICES:
                services = optarg;
                break;
            case DYDISNIX_OPTION_INFRASTRUCTURE:
                infrastructure = optarg;
                break;
            case DYDISNIX_OPTION_DISTRIBUTION:
                distribution = optarg;
                break;
            case DYDISNIX_OPTION_XML:
                xml = TRUE;
                break;
            case DYDISNIX_OPTION_INTERFACE:
                interface = optarg;
                break;
            case DYDISNIX_OPTION_TARGET_PROPERTY:
                target_property = optarg;
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

    interface = check_interface_option(interface);
    target_property = check_target_property_option(target_property);

    if(!check_services_option(services)
      || !check_infrastructure_option(infrastructure)
      || !check_distribution_option(distribution))
        return 1;

    /* Execute operation */
    return filter_buildable(services, infrastructure, distribution, xml, interface, target_property);
}
