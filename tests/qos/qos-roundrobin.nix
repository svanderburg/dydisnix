{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.divideRoundRobin {
  distribution = initialDistribution;
}
