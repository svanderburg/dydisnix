{ nixpkgs ? <nixpkgs>
, defaultTargetProperty
}:

let
  pkgs = import nixpkgs {};
  filters = import ./filters.nix { inherit pkgs; };

  getTargetProperty = {target, defaultTargetProperty}:
    if target ? targetProperty then target.targetProperty else defaultTargetProperty;
in
{
  servicesToXML = {servicesFile}:
    let
      servicesFun = import servicesFile;
      services = servicesFun {
        distribution = {};
        invDistribution = {};
        system = builtins.currentSystem;
        inherit pkgs;
      };
      serviceProperties = filters.filterDerivations services;
    in
    filters.generateServicesXML serviceProperties;

  infrastructureToXML = {infrastructureFile}:
    let
      infrastructure = import infrastructureFile;
    in
    filters.generateInfrastructureXML infrastructure;


  distributionToXML = {infrastructureFile, distributionFile}:
    let
      infrastructure = import infrastructureFile;
      distribution = import distributionFile { inherit infrastructure; };

      # Attribute set that can be used to map unique target properties to targets
      targetMappings = builtins.listToAttrs (map (targetName:
        let
          target = builtins.getAttr targetName infrastructure;
          targetProperty = getTargetProperty {
            inherit target defaultTargetProperty;
          };
        in
        { name = builtins.getAttr targetProperty target.properties;
          value = targetName;
        }
      ) (builtins.attrNames infrastructure));

      mappings = pkgs.lib.mapAttrs (serviceName: targets:
        if builtins.isList targets then
          map (target:
            let
              targetProperty = getTargetProperty {
                inherit target defaultTargetProperty;
              };
              targetIdentifier = builtins.getAttr targetProperty target.properties;
            in
            { target = builtins.getAttr targetIdentifier targetMappings; }
          ) targets
        else if builtins.isAttrs targets then
          map (mapping:
            let
              targetProperty = getTargetProperty {
                inherit (mapping) target;
                inherit defaultTargetProperty;
              };
              targetIdentifier = builtins.getAttr targetProperty mapping.target.properties;
            in
            { target = builtins.getAttr targetIdentifier targetMappings; }
            // (if mapping ? container then { inherit (mapping) container; } else {})
          ) targets.targets
        else throw "targets has the wrong type!"
      ) distribution;
    in
    filters.generateDistributionXML mappings;

  portsToXML = {portsFile}:
    let
      ports = import portsFile;
    in
    filters.generatePortsXML ports;

  docsToXML = {docsFile}:
    let
      docs = import docsFile;
    in
    filters.generateDocsXML docs;

  idResourcesToXML = {idResourcesFile}:
    let
      idResources = import idResourcesFile;
    in
    filters.generateIdResourcesXML idResources;

  idsToXML = {idsFile}:
    let
      ids = import idsFile;
    in
    filters.generateIdsXML ids;
}
