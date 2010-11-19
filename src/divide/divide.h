#ifndef __DIVIDE_H
#define __DIVIDE_H
#include <glib.h>

typedef enum
{
    STRATEGY_NONE,
    STRATEGY_GREEDY,
    STRATEGY_HIGHEST_BIDDER
}
Strategy;

void divide(Strategy strategy, gchar *service_xml, gchar *infrastructure_xml, gchar *distribution_xml, gchar *service_property, gchar *infrastructure_property);

#endif
