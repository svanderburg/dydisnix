{ nixpkgs ? <nixpkgs> }:

let
  pkgs = import nixpkgs {};
  filters = import ./filters.nix { inherit pkgs; };
in
{
  servicesToXML = {servicesFile}:
    let
      servicesFun = import servicesFile;
      services = servicesFun { distribution = null; system = null; pkgs = null; };
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
        map (target: target.hostname) targets
      ) distribution;
    in
    filters.generateDistributionXML mappings;
}
