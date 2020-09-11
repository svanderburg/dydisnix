{pkgs, disnix, dydisnix}:

(import ./intfilters.nix { inherit pkgs; }) //
(import ./extfilters.nix { inherit pkgs disnix dydisnix; })
