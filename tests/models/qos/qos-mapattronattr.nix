{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.mapAttrOnAttr {
  inherit services infrastructure;
  distribution = initialDistribution;
  serviceProperty = "requireZone";
  targetProperty = "zone";
}
