{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.mapAttrOnList {
  inherit services infrastructure;
  distribution = initialDistribution;
  serviceProperty = "type";
  targetPropertyList = "supportedTypes";
}
