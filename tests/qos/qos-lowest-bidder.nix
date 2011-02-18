{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.divide {
  strategy = "lowest-bidder";

  inherit services infrastructure;
  distribution = initialDistribution;
  
  serviceProperty = "requireMem";
  targetProperty = "mem";
}
