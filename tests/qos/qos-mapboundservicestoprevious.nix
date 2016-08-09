{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.mapBoundServicesToPrevious {
  inherit services previousDistribution;
  distribution = initialDistribution;
}
