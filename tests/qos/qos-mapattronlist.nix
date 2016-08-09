{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.mapAttrOnList {
  inherit services infrastructure;
  distribution = initialDistribution;
  serviceProperty = "type";
  targetPropertyList = "supportedTypes";
}
