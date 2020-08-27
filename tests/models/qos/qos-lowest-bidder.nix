{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.divide {
  strategy = "lowest-bidder";

  inherit services infrastructure;
  distribution = initialDistribution;
  
  serviceProperty = "requireMem";
  targetProperty = "mem";
}
