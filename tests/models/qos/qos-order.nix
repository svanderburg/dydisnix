{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.order {
  inherit infrastructure;
  distribution = initialDistribution;
  targetProperty = "priority";
}
