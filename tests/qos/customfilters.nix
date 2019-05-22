{pkgs, referenceFilters}:

{
  clear = {distribution}:
    pkgs.lib.mapAttrs (serviceName: target: []);
}
