{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.minsetcover {
  inherit services infrastructure;
  distribution = initialDistribution;
  targetProperty = "cost";
}
