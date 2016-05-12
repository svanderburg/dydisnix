{lib}:

rec {
  inherit (builtins) listToAttrs attrNames getAttr hasAttr lessThan;
  inherit (lib) filter elem;

  filterDerivations = services:
    listToAttrs (map (serviceName:
      let
        service = getAttr serviceName services;
      in
      { name = serviceName;
        value = listToAttrs(map (propertyName:
                  { name = propertyName;
                    value = if propertyName == "dependsOn"
                      then map (dependencyName: (getAttr dependencyName (service.dependsOn)).name) (attrNames (service.dependsOn))
                      else getAttr propertyName service;
                  } ) (filter (propertyName: propertyName != "pkg") (attrNames service)))
                ;
      }
    ) (attrNames services))
  ;
  
  createCartesianProduct = {services, infrastructure}:
    listToAttrs (map (serviceName:
      { name = serviceName;
        value = map (targetName: targetName) (attrNames infrastructure);
      }
    ) (attrNames services))
  ;
  
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
  
  order = {infrastructure, distribution, targetProperty}:
    lib.mapAttrs (serviceName: mapping:
      lib.sort (targetAName: targetBName:
        let
          targetA = getAttr targetAName infrastructure;
          targetB = getAttr targetBName infrastructure;
        in
        lessThan (getAttr targetProperty targetA.properties) (getAttr targetProperty targetB.properties) ) mapping
    ) distribution
  ;
}
