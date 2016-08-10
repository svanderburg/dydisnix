{ nixpkgs ? <nixpkgs> 
, targetProperty
}:

let
  pkgs = import nixpkgs {};
  filters = import ./filters.nix { inherit pkgs; };
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
      mappings = pkgs.lib.mapAttrs (serviceName: targets: # Substitute targets in infrastructure model by their names
        map (target:
          let
            actualTargetProperty = if target ? targetProperty then target.targetProperty else targetProperty;
          in
          builtins.getAttr actualTargetProperty target.properties) targets
      ) distribution;
    in
    filters.generateDistributionXML mappings;

  portsToXML = {portsFile}:
    let
      ports = import portsFile;
    in
    filters.generatePortsXML ports;
}
