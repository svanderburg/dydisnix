{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.divide {
  strategy = "highest-bidder";

  inherit services infrastructure;
  distribution = initialDistribution;
  
  serviceProperty = "requireMem";
  targetProperty = "mem";
}
