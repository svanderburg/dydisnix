{ pkgs
, dydisnix
, disnix
}:

let
  inherit (builtins) toXML;
in
rec {
  nixExpressionToXML = {name, expr, xsl}:
    pkgs.stdenv.mkDerivation {
      name = "${name}.xml";
      buildInputs = [ pkgs.libxslt ];
      exprXML = toXML expr;
      passAsFile = [ "exprXML" ];

      buildCommand = ''
        if [ "$exprXMLPath" != "" ]
        then
            xsltproc ${xsl} $exprXMLPath > $out
        else
        (
        cat << "EOF"
        $exprXML
        EOF
        ) | xsltproc ${xsl} - > $out
        fi
      '';
    };

  generateServicesXML = services:
    nixExpressionToXML {
      name = "services";
      expr = services;
      xsl = ./services.xsl;
    };

  generateInfrastructureXML = infrastructure:
    nixExpressionToXML {
      name = "infrastructure";
      expr = infrastructure;
      xsl = "${disnix}/share/disnix/infrastructure.xsl";
    };

  generateDistributionXML = distribution:
    nixExpressionToXML {
      name = "distribution";
      expr = distribution;
      xsl = ./distribution.xsl;
    };

  generatePortsXML = ports:
    nixExpressionToXML {
      name = "ports";
      expr = ports;
      xsl = ./ports.xsl;
    };

  generateDocsXML = docs:
    nixExpressionToXML {
      name = "docs";
      expr = docs;
      xsl = ./docs.xsl;
    };

  generateIdResourcesXML = idResources:
    nixExpressionToXML {
      name = "idresources";
      expr = idResources;
      xsl = ./idresources.xsl;
    };

  generateIdsXML = ids:
    nixExpressionToXML {
      name = "ids";
      expr = ids;
      xsl = ./ids.xsl;
    };

  generatePreviousDistribution = coordinatorProfile:
    import "${(pkgs.stdenv.mkDerivation {
      name = "distribution.nix";
      buildInputs = [ dydisnix ];
      buildCommand = ''
        dydisnix-previous-distribution ${coordinatorProfile} > $out
      '';
    })}";

  /**
   * Uses a one-dimensional configuration strategy to divide services over
   * candidate target machines.
   *
   * Parameters:
   * strategy: Should be one of the following: 'greedy', 'highest-bidder',
   *   'lowest-bidder'. Greedy distributes services to machines in linear order
   *   and keeps distributing them to a machine until its capacity limit has
   *   been reached. Highest bidder distributes for each selection step a
   *   service to a machine with the most available capacity left. Lowest bidder
   *   distributes for each selection step a service to a machine with the least
   *   available capacity (that is still capable of hosting the service).
   * services: Services model
   * infrastructure: Infrastructure model
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   * serviceProperty: Name of the property of a service. This value is supposed to be numeric and gets deducted from the machine's total capacity
   * targetProperty: Name of the property of a target machine. This value is supposed to be numeric and represents the total amount of capacity a machine provides
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  divide = {strategy, services, infrastructure, distribution, serviceProperty, targetProperty}:
    import "${(pkgs.stdenv.mkDerivation {
      name = "distribution.nix";
      buildInputs = [ dydisnix ];
      buildCommand = 
      ''
        dydisnix-divide \
          --strategy ${strategy} \
          --services ${generateServicesXML services} \
          --infrastructure ${generateInfrastructureXML infrastructure} \
          --distribution ${generateDistributionXML distribution} \
          --service-property ${serviceProperty} \
          --target-property ${targetProperty} \
          --xml \
          > $out
      '';
    })}";
  
  /**
   * Distributes services to candidates using an approximation algorithm for the
   * multiway cut problem.
   *
   * Parameters:
   * services: Services model
   * infrastructure: Infrastructure model
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  multiwaycut = {services, infrastructure, distribution}:
    import "${(pkgs.stdenv.mkDerivation {
      name = "distribution.nix";
      buildInputs = [ dydisnix ];
      buildCommand =
      ''
        dydisnix-multiwaycut \
          --services ${generateServicesXML services} \
          --infrastructure ${generateInfrastructureXML infrastructure} \
          --distribution ${generateDistributionXML distribution} \
          --xml \
          > $out
      '';
    })}";

  /**
   * Distributes services to candidates using an approximation algorithm for the
   * minimum set cover problem.
   *
   * Parameters:
   * services: Services model
   * infrastructure: Infrastructure model
   * distribution: A candidate target mapping in which each key refers to a service and each value to a list of machine names
   * targetProperty: Name of the property of a target machine. This value is supposed to be numeric and represents the total amount of capacity a machine provides
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  minsetcover = {services, infrastructure, distribution, targetProperty}:
    import "${(pkgs.stdenv.mkDerivation {
      name = "distribution.nix";
      buildInputs = [ dydisnix ];
      buildCommand =
      ''
        dydisnix-minsetcover \
          --services ${generateServicesXML services} \
          --infrastructure ${generateInfrastructureXML infrastructure} \
          --distribution ${generateDistributionXML distribution} \
          --target-property ${targetProperty} \
          --xml \
          > $out
      '';
    })}";

  /**
   * Distributes services to candidates using an approximation algorithm for the
   * graph coloring problem.
   *
   * Parameters:
   * services: Services model
   * infrastructure: Infrastructure model
   *
   * Returns:
   * A candidate target mapping in which each key refers to a service and each
   * value to a list of machine names
   */
  graphcol = {services, infrastructure}:
    import "${(pkgs.stdenv.mkDerivation {
      name = "distribution.nix";
      buildInputs = [ dydisnix ];
      buildCommand =
      ''
         dydisnix-graphcol \
          --services ${generateServicesXML services} \
          --infrastructure ${generateInfrastructureXML infrastructure} \
          --xml \
          > $out
      '';
    })}";
}
