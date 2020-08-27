{services, infrastructure, initialDistribution, previousDistribution, filters, lib}:

builtins.listToAttrs (map (serviceName:
  let targets = builtins.getAttr serviceName initialDistribution;
  in
  { name = serviceName;
    value = [ (builtins.head targets) ];
  }
) (builtins.attrNames initialDistribution))
