{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.mapAttrOnAttr {
  inherit services infrastructure;
  distribution = initialDistribution;
  serviceProperty = "requireZone";
  targetProperty = "zone";
}
