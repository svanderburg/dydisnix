{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

filters.divide {
  strategy = "highest-bidder";

  inherit services infrastructure;
  distribution = initialDistribution;
  
  serviceProperty = "requireMem";
  targetProperty = "mem";
}
