#include "checkoptions.h"
#include <stdio.h>

#define TRUE 1
#define FALSE 0

dydisnix_bool check_services_option(const char *services)
{
    if(services == NULL)
    {
        fprintf(stderr, "A services expression must be specified!\n");
        return FALSE;
    }
    else
        return TRUE;
}

dydisnix_bool check_infrastructure_option(const char *infrastructure)
{
    if(infrastructure == NULL)
    {
        fprintf(stderr, "An infrastructure configuration file must be specified!\n");
        return FALSE;
    }
    else
        return TRUE;
}

dydisnix_bool check_distribution_option(const char *distribution)
{
    if(distribution == NULL)
    {
        fprintf(stderr, "A distribution configuration file must be specified!\n");
        return FALSE;
    }
    else
        return TRUE;
}

dydisnix_bool check_service_mapping_property_option(const char *service_property)
{
    if(service_property == NULL)
    {
        fprintf(stderr, "A service property must be specified!\n");
        return FALSE;
    }
    else
        return TRUE;
}

dydisnix_bool check_target_mapping_property_option(const char *target_property)
{
    if(target_property == NULL)
    {
        fprintf(stderr, "A target property must be specified!\n");
        return FALSE;
    }
    else
        return TRUE;
}
