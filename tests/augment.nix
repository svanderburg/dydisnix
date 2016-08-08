{infrastructure, lib}:

lib.mapAttrs (targetName: target:
  target // {
    properties.augment = "augment";
  }
) infrastructure
