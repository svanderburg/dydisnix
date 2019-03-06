#ifndef __DYDISNIX_DIVIDE_H
#define __DYDISNIX_DIVIDE_H
#include <glib.h>

typedef enum
{
    STRATEGY_NONE,
    STRATEGY_GREEDY,
    STRATEGY_HIGHEST_BIDDER,
    STRATEGY_LOWEST_BIDDER,
}
Strategy;

int divide(Strategy strategy, gchar *services, gchar *infrastructure, gchar *distribution, gchar *service_property, gchar *target_property, int xml);

#endif
