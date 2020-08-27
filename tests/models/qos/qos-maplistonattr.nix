{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.mapListOnAttr {
  inherit services infrastructure;
  distribution = initialDistribution;
  servicePropertyList = "requireZones";
  targetProperty = "zone";
}
