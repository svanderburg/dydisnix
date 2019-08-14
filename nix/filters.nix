{pkgs}:

(import ./intfilters.nix { inherit pkgs; }) //
(import ./extfilters.nix { inherit pkgs; })
