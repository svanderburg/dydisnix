{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.multiwaycut {
  inherit services infrastructure;
  distribution = initialDistribution;
}
