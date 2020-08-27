{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.mapStatefulToPrevious {
  inherit services previousDistribution;
  distribution = initialDistribution;
}
