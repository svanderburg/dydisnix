{ servicesFile
, infrastructureFile
, distributionFile ? null
, qosFile ? null
, outputExpr ? true
, nixpkgs ? <nixpkgs>
, coordinatorProfile ? null
}:

let
  # Dependencies
  pkgs = import nixpkgs {};
  filters = import ./filters.nix { inherit pkgs; };
  distributionXSL = if outputExpr then ./distribution_expr.xsl else ./distribution.xsl;
  
  # Import files
  servicesFun = import servicesFile;
  infrastructure = import infrastructureFile;
  qosFun = import qosFile;
  
  # Evaluations
  services = servicesFun {
    distribution = {};
    invDistribution = {};
    system = builtins.currentSystem;
    inherit pkgs;
  };
  
  serviceProperties = filters.filterDerivations services;
  
  initialDistribution = 
    if distributionFile == null then filters.createCartesianProduct { services = serviceProperties; inherit infrastructure; }
    else import distributionFile
  ;
  
  previousDistribution =
    if coordinatorProfile == null then null
    else
      filters.generatePreviousDistribution coordinatorProfile
  ;
  
  qos = if qosFile == null then initialDistribution else qosFun { services = serviceProperties; inherit infrastructure initialDistribution previousDistribution filters; };
in
pkgs.stdenv.mkDerivation {
  name = "distribution.${if outputExpr then "nix" else "xml"}";
  buildInputs = [ pkgs.libxslt ];
  buildCommand =
  ''
    (
    cat <<EOF
    ${builtins.toXML qos}
    EOF
    ) | xsltproc ${distributionXSL} - > $out
  '';
}
