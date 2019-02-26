{ nixpkgs ? <nixpkgs>
, systems ? [ "i686-linux" "x86_64-linux" ]
, dydisnix ? {outPath = ./.; rev = 1234;}
, fetchDependenciesFromNixpkgs ? false
, officialRelease ? false
}:

let
  pkgs = import nixpkgs {};

  # Refer either to disnix in the parent folder, or to the one in Nixpkgs
  disnixJobset = if fetchDependenciesFromNixpkgs then {
    build = pkgs.lib.genAttrs systems (system:
      (import nixpkgs { inherit system; }).disnix
    );
  } else import ../disnix/release.nix { inherit nixpkgs systems officialRelease; };

  jobs = rec {
    tarball =
      let
        disnix = builtins.getAttr (builtins.currentSystem) (disnixJobset.build);
      in
      pkgs.releaseTools.sourceTarball {
        name = "dydisnix-tarball";
        version = builtins.readFile ./version;
        src = dydisnix;
        inherit officialRelease;

        CFLAGS = "-Wall";

        buildInputs = [ pkgs.pkgconfig pkgs.getopt pkgs.libxml2 pkgs.glib disnix ]
          ++ pkgs.lib.optional (!pkgs.stdenv.isLinux) pkgs.libiconv
          ++ pkgs.lib.optional (!pkgs.stdenv.isLinux) pkgs.gettext;
      };

    build = pkgs.lib.genAttrs systems (system:
      let
        pkgs = import nixpkgs { inherit system; };

        disnix = builtins.getAttr system (disnixJobset.build);
      in
      pkgs.releaseTools.nixBuild {
        name = "dydisnix";
        src = tarball;

        CFLAGS = "-Wall";

        buildInputs = [ pkgs.pkgconfig pkgs.getopt pkgs.libxml2 pkgs.glib disnix ]
          ++ pkgs.lib.optional (!pkgs.stdenv.isLinux) pkgs.libiconv
          ++ pkgs.lib.optional (!pkgs.stdenv.isLinux) pkgs.gettext;
      });

    tests =
      let
        disnix = builtins.getAttr (builtins.currentSystem) (disnixJobset.build);
        dydisnix = builtins.getAttr (builtins.currentSystem) build;
      in
      import ./tests {
        inherit nixpkgs pkgs disnix dydisnix;
      };
  };
in
jobs
