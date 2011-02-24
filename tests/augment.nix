{infrastructure, lib}:

lib.mapAttrs (targetName: target:
  target // {
    augment = "augment";
  }
) infrastructure
