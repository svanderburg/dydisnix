{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.minsetcover {
  inherit services infrastructure;
  distribution = initialDistribution;
  targetProperty = "cost";
}
