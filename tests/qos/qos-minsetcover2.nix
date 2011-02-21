{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.minsetcover {
  inherit services infrastructure;
  
  distribution = filters.mapListOnAttr {
    inherit services infrastructure;
    distribution = initialDistribution;
    servicePropertyList = "requireAccess";
    targetProperty = "access";
  };
  
  targetProperty = "cost";
}
