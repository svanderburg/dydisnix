{ servicesFile
, infrastructureFile
, distributionFile ? null
, qosFile ? null
, filtersFile ? null
, outputExpr ? true
, nixpkgs ? <nixpkgs>
, coordinatorProfile ? null
, disnix
, dydisnix
}:

let
  # Dependencies
  pkgs = import nixpkgs {};
  referenceFilters = import ./filters.nix {
    inherit pkgs disnix dydisnix;
  };

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

  serviceProperties = referenceFilters.filterDerivations services;

  initialDistribution =
    if distributionFile == null then
      referenceFilters.createCartesianProduct {
        services = serviceProperties;
        inherit infrastructure;
      }
    else
      import distributionFile;

  previousDistribution =
    if coordinatorProfile == null then null
    else
      referenceFilters.generatePreviousDistribution coordinatorProfile;

  filters = if filtersFile == null then referenceFilters
    else import filtersFile {
      inherit pkgs referenceFilters;
    };

  qos = if qosFile == null then
    initialDistribution
  else qosFun {
    services = serviceProperties;
    inherit infrastructure initialDistribution previousDistribution filters;
    inherit (pkgs) lib;
  };
in
pkgs.stdenv.mkDerivation {
  name = "distribution.${if outputExpr then "nix" else "xml"}";
  buildInputs = [ pkgs.libxslt ];
  qosXML = builtins.toXML qos;
  passAsFile = [ "qosXML" ];

  buildCommand = ''
    if [ "$qosXMLPath" != "" ]
    then
        xsltproc ${distributionXSL} $qosXMLPath > $out
    else
    (
    cat <<EOF
    $qosXML
    EOF
    ) | xsltproc ${distributionXSL} - > $out
    fi
  '';
}
