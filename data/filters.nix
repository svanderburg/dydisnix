{pkgs}:

(import ./intfilters.nix { inherit (pkgs) lib; }) //
(import ./extfilters.nix { inherit pkgs; })
