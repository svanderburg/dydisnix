{services, infrastructure, initialDistribution, previousDistribution, filters}:

filters.divide {
  strategy = "greedy";

  inherit services infrastructure;
  distribution = initialDistribution;
  
  serviceProperty = "requireMem";
  targetProperty = "mem";
}
