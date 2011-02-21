{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.order {
  inherit infrastructure;
  distribution = initialDistribution;
  targetProperty = "priority";
}
