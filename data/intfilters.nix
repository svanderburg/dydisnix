{pkgs}:

let
  inherit (builtins) listToAttrs attrNames getAttr hasAttr lessThan head tail length;
  inherit (pkgs.lib) filter elem mapAttrs sort;

  translateDependencyReferencesToNames = {services, dependencies}:
    map (dependencyName: (getAttr dependencyName dependencies).name) (attrNames dependencies)
  ;
in
rec {
  # Filter out the pkg attribute (because that implies building), and replace dependency references by names
  filterDerivations = services:
    listToAttrs (map (serviceName:
      let
        service = getAttr serviceName services;
      in
      { name = serviceName;
        value = listToAttrs(map (propertyName:
          { name = propertyName;
            # Replace dependency references by their names
            value = if propertyName == "dependsOn" then translateDependencyReferencesToNames { inherit services; dependencies = service.dependsOn; }
              else if propertyName == "connectsTo" then translateDependencyReferencesToNames { inherit services; dependencies = service.connectsTo; }
              else getAttr propertyName service;
          } ) (filter (propertyName: propertyName != "pkg") (attrNames service)))
        ;
      }
    ) (attrNames services))
  ;
  
  /**
   * Generates a distribution model that is a cartesian product, by mapping each
   * service in the service model to each target in the infrastructure model.
   *
   * Parameters:
   * services: Services model
   * infrastructure: Infrastructure model
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  createCartesianProduct = {services, infrastructure}:
    listToAttrs (map (serviceName:
      { name = serviceName;
        value = map (targetName: targetName) (attrNames infrastructure);
      }
    ) (attrNames services))
  ;
  
  /**
   * Generates a distribution by filtering on mappings of a property of a
   * service onto a property of a target machine (that is a list) in the
   * infrastructure model.
   *
   * Parameters:
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   * services: Services model
   * infrastructure: Infrastructure model
   * serviceProperty: Name of the property of a service
   * targetPropertyList: Name of the property of a target machine that is a list
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  mapAttrOnList = {distribution, services, infrastructure, serviceProperty, targetPropertyList}:
    listToAttrs (map (serviceName:
      { name = serviceName;
        value =
          let
            servicePropertyValue = getAttr serviceProperty (getAttr serviceName services);
            targets = getAttr serviceName distribution;
          in
          filter (targetName:
            let
              target = getAttr targetName infrastructure;
              targetPropertyListValue = getAttr targetPropertyList target.properties;
            in
              elem servicePropertyValue targetPropertyListValue
          ) targets;
      }
    ) (attrNames distribution))
  ;
  
  /**
   * Generates a distribution by filtering on mappings of a property list of a
   * service to a property of a target machine in the infrastructure model.
   *
   * Parameters:
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   * services: Services model
   * infrastructure: Infrastructure model
   * servicePropertyList: Name of the property of a service that is a list
   * targetProperty: Name of the property of a target machine
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  mapListOnAttr = {distribution, services, infrastructure, servicePropertyList, targetProperty}:
    listToAttrs (map (serviceName:
      { name = serviceName;
        value =
          let
            targets = getAttr serviceName distribution;
            service = getAttr serviceName services;
            servicePropertyListValue = getAttr servicePropertyList service;
          in
          filter (targetName:
            let
              target = getAttr targetName infrastructure;
              targetPropertyValue = getAttr targetProperty target.properties;
            in
            elem targetPropertyValue servicePropertyListValue
          ) targets;
      }
    ) (attrNames distribution))
  ;
  
  /**
   * Generates a distribution by filtering on mappings of a service property
   * to a property of a target machine in the infrastructure model.
   *
   * Parameters:
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   * services: Services model
   * infrastructure: Infrastructure model
   * servicePropert: Name of the property of a service
   * targetProperty: Name of the property of a target machine
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  mapAttrOnAttr = {distribution, services, infrastructure, serviceProperty, targetProperty}:
    listToAttrs (map (serviceName:
      { name = serviceName;
        value =
          let
            servicePropertyValue = getAttr serviceProperty (getAttr serviceName services);
            targets = getAttr serviceName distribution;
          in
          filter (targetName:
            let
              targetPropertyValue = getAttr targetProperty (getAttr targetName infrastructure).properties;
            in
            servicePropertyValue == targetPropertyValue
          ) targets;
      }
    ) (attrNames distribution))
  ;
  
  /**
   * Maps each service in the that have been been marked as stateful to its
   * previous targets, or to the given candidates if it has not been
   * distribution previously.
   *
   * Parameters:
   * services: Services model
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   * previousDistribution: A candidate target mapping representing the previous distribution in which each key refers to a service and each value to a list of machine names
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  mapStatefulToPrevious = {services, distribution, previousDistribution}:
    if previousDistribution == null then distribution
    else
    listToAttrs (map (serviceName:
      { name = serviceName;
        value =
          let
            service = getAttr serviceName services;
            targets = getAttr serviceName distribution;
            previousTargets = if hasAttr serviceName previousDistribution then getAttr serviceName previousDistribution
              else targets;
          in
          if service ? stateful && service.stateful then
            filter (targetName: elem targetName previousTargets) targets
          else targets;
      }
    ) (attrNames distribution))
  ;
  
  /**
   * Maps each service that have been distributed to target machines in a
   * previous distributions to its original locations. It maps new services to
   * the given candidates.
   *
   * Parameters:
   * services: Services model
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   * previousDistribution: A candidate target mapping representing the previous distribution in which each key refers to a service and each value to a list of machine names
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  mapBoundServicesToPrevious = {services, distribution, previousDistribution}:
    if previousDistribution == null then distribution
    else
    listToAttrs (map (serviceName:
      { name = serviceName;
        value =
          let
            service = getAttr serviceName services;
            targets = getAttr serviceName distribution;
          in
          if hasAttr serviceName previousDistribution then getAttr serviceName previousDistribution else targets;
      }
    ) (attrNames distribution))
  ;
  
  findSuccessor = {targets, previousTarget, allTargets}:
    if head targets == previousTarget then
      if length targets == 1 then head allTargets
      else head (tail targets)
    else findSuccessor {
      targets = tail targets;
      inherit previousTarget allTargets;
    }
  ;
  
  findNextTarget = {targets, previousTarget}:
    if targets == [] then throw "Cannot find a target machine!"
    else if length targets == 1 then head targets
    else if elem previousTarget targets then findSuccessor { inherit targets previousTarget; allTargets = targets; }
    else head targets
  ;
  
  generateRoundRobinDistribution = {serviceNames, distribution, previousTarget}:
    if serviceNames == [] then []
    else
      let
        serviceName = head serviceNames;
        targets = getAttr serviceName distribution;
        
        target = if previousTarget == null then head targets
          else findNextTarget { inherit targets previousTarget; };
      in
      [ { name = serviceName; value = [ target ]; } ]
      ++ generateRoundRobinDistribution {
        serviceNames = tail serviceNames;
        inherit distribution;
        previousTarget = target;
      }
  ;
  
  /**
   * Maps each service to a target machine in the candidate target list using
   * the round robin strategy. In other words: it distributes services in equal
   * proportions to each candidate target in circular order.
   *
   * Parameters:
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  divideRoundRobin = {distribution}:
    listToAttrs (generateRoundRobinDistribution {
      serviceNames = attrNames distribution;
      inherit distribution;
      previousTarget = null;
    })
  ;
  
  /**
   * Orders the candidate targets in the distribution by priority.
   *
   * Parameters:
   * infrastructure: Infrastructure model
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   * targetProperty: Name of the property of a target in the infrastructure model to sort on
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each value to a list of machine names
   */
  order = {infrastructure, distribution, targetProperty}:
    mapAttrs (serviceName: mapping:
      sort (targetAName: targetBName:
        let
          targetA = getAttr targetAName infrastructure;
          targetB = getAttr targetBName infrastructure;
        in
        lessThan (getAttr targetProperty targetA.properties) (getAttr targetProperty targetB.properties) ) mapping
    ) distribution
  ;
}
