#ifndef __DYDISNIX_CHECKOPTIONS_H
#define __DYDISNIX_CHECKOPTIONS_H

#define DYDISNIX_DEFAULT_BATCH FALSE

#define DYDISNIX_FLAG_XML               0x1
#define DYDISNIX_FLAG_OUTPUT_XML        0x2
#define DYDISNIX_FLAG_GROUP_SUBSERVICES 0x4
#define DYDISNIX_FLAG_BATCH             0x8

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
    DYDISNIX_OPTION_OUTPUT_XML = 258,
    DYDISNIX_OPTION_SERVICE_PROPERTY = 259,
    DYDISNIX_OPTION_TARGET_PROPERTY = 260,
    DYDISNIX_OPTION_BATCH = 261,
    DYDISNIX_OPTION_GROUP_SUBSERVICES = 262,
    DYDISNIX_OPTION_GROUP = 263,
    DYDISNIX_OPTION_OUTPUT_DIR = 264,
    DYDISNIX_OPTION_DOCS = 265,
    DYDISNIX_OPTION_INTERFACE = 266,
    DYDISNIX_OPTION_EXTRA_PARAMS = 267,
    DYDISNIX_OPTION_ID_RESOURCES = 268,
    DYDISNIX_OPTION_IDS = 269,
    DYDISNIX_OPTION_OUTPUT = 270
}
DydisnixCommandLineOptions;

int check_services_option(const char *services);

int check_infrastructure_option(const char *infrastructure);

int check_distribution_option(const char *distribution);

int check_service_mapping_property_option(const char *service_property);

int check_target_mapping_property_option(const char *target_property);

#endif
