{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.mapListOnAttr {
  inherit services infrastructure;
  distribution = initialDistribution;
  servicePropertyList = "requireZones";
  targetProperty = "zone";
}
