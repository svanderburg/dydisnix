{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.mapBoundServicesToPrevious {
  inherit services previousDistribution;
  distribution = initialDistribution;
}
