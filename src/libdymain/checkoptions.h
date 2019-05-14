#ifndef __DYDISNIX_CHECKOPTIONS_H
#define __DYDISNIX_CHECKOPTIONS_H

#define DYDISNIX_DEFAULT_XML FALSE
#define DYDISNIX_DEFAULT_GROUP_SUBSERVICES FALSE
#define DYDISNIX_DEFAULT_BATCH FALSE

typedef enum
{
    /* Short and long options */
    DYDISNIX_OPTION_HELP = 'h',

    DYDISNIX_OPTION_SERVICES = 's',
    DYDISNIX_OPTION_INFRASTRUCTURE = 'i',
    DYDISNIX_OPTION_DISTRIBUTION = 'd',

    DYDISNIX_OPTION_PORTS = 'p',

    DYDISNIX_OPTION_FORMAT = 'f',

    /* Long-only options */
    DYDISNIX_OPTION_STRATEGY = 256,
    DYDISNIX_OPTION_XML = 257,
    DYDISNIX_OPTION_SERVICE_PROPERTY = 258,
    DYDISNIX_OPTION_TARGET_PROPERTY = 259,
    DYDISNIX_OPTION_BATCH = 260,
    DYDISNIX_OPTION_GROUP_SUBSERVICES = 261,
    DYDISNIX_OPTION_GROUP = 262,
    DYDISNIX_OPTION_OUTPUT_DIR = 263,
    DYDISNIX_OPTION_DOCS = 264,
    DYDISNIX_OPTION_INTERFACE = 265
}
DydisnixCommandLineOptions;

int check_services_option(const char *services);

int check_infrastructure_option(const char *infrastructure);

int check_distribution_option(const char *distribution);

int check_service_mapping_property_option(const char *service_property);

int check_target_mapping_property_option(const char *target_property);

#endif
