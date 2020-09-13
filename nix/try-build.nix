{ servicesFile, infrastructureFile, distributionFile, serviceName, targetName, defaultClientInterface, defaultTargetProperty, extraParams
, nixpkgs ? <nixpkgs>
, disnix
}:

let
  inherit (builtins) getAttr;

  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;
  distributionFun = import distributionFile;

  pkgs = import nixpkgs {};

  wrapArchitecture = import "${disnix}/share/disnix/wrap-architecture.nix" {
    inherit (pkgs) lib;
  };

  normalizeArchitecture = import "${disnix}/share/disnix/normalize-architecture.nix" {
    inherit (pkgs) lib;
  };

  architectureFun = wrapArchitecture.wrapBasicInputsModelsIntoArchitectureFun {
    inherit servicesFun infrastructure distributionFun extraParams;
    packagesFun = null;
  };

  normalizedArchitecture = normalizeArchitecture.generateNormalizedDeploymentArchitecture {
    inherit architectureFun nixpkgs defaultClientInterface defaultTargetProperty;
    defaultDeployState = false;
    extraParams = {};
  };

  target = getAttr targetName infrastructure;
  system = if target ? system then target.system else builtins.currentSystem;
in
normalizedArchitecture.services."${serviceName}".pkgs."${system}"
