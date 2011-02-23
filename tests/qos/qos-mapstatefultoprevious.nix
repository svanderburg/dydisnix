{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.mapStatefulToPrevious {
  inherit services previousDistribution;
  distribution = initialDistribution;
}
