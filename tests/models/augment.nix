{infrastructure, lib}:

lib.mapAttrs (targetName: target:
  target // {
    properties = target.properties // {
      augment = "augment";
    };
  }
) infrastructure
