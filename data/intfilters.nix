{lib}:

rec {
  inherit (builtins) listToAttrs attrNames getAttr;
  inherit (lib) filter elem;

  filterDerivations = services:
    listToAttrs (map (serviceName:
      let
        service = getAttr serviceName services;
      in
      { name = serviceName;
        value = listToAttrs(map (propertyName:
	          { name = propertyName; value = getAttr propertyName service; }
		  ) (filter (propertyName: propertyName != "pkg" && propertyName != "dependsOn") (attrNames service)))
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
  
  filterDistributionMapAttributeOnList = {distribution, services, infrastructure, serviceProperty, targetPropertyList}:
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
	      targetPropertyListValue = getAttr targetPropertyList target;
	    in
	      elem servicePropertyValue targetPropertyListValue
          ) targets;
      }
    ) (attrNames distribution))
  ;
  
  filterDistributionMapListOnAttribute = {distribution, services, infrastructure, servicePropertyList, targetProperty}:
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
	      targetPropertyValue = getAttr targetProperty target;
	    in
	      elem targetPropertyValue servicePropertyListValue
	  ) targets;
      }
    ) (attrNames distribution))
  ;
  
  filterDistributionMapAttributes = {distribution, services, infrastructure, serviceProperty, targetProperty}:
    listToAttrs (map (serviceName:
      { name = serviceName;
        value =
	  let
	    servicePropertyValue = getAttr serviceProperty (getAttr serviceName services);
	    targets = getAttr serviceName distribution;
	  in
	  filter (targetName:
	    let
	      targetPropertyValue = getAttr targetProperty (getAttr targetName infrastructure);
	    in
	    servicePropertyValue == targetPropertyValue
	  ) targets;
      }
    ) (attrNames distribution))
  ;
}
