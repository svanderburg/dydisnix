{ servicesFile
, infrastructureFile
, distributionFile ? null
, qosFile ? null
, outputExpr ? true
, nixpkgs ? builtins.getEnv "NIXPKGS_ALL"
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
  services = servicesFun { distribution = null; system = null; };
  
  serviceProperties = filters.filterDerivations services;
  
  initialDistribution = 
    if distributionFile == null then filters.createCartesianProduct { services = serviceProperties; inherit infrastructure; }
    else import distributionFile
  ;
  
  qos = if qosFile == null then initialDistribution else qosFun { services = serviceProperties; inherit infrastructure initialDistribution filters; };
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
