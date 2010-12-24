{ nixpkgs ? /etc/nixos/nixpkgs }:

let
  jobs = rec {
    tarball =
      { dydisnix ? {outPath = ./.; rev = 1234;}
      , officialRelease ? false
      , disnix ? (import ../../disnix/trunk/release.nix {}).build {}
      }:

      with import nixpkgs {};

      releaseTools.sourceTarball {
        name = "dydisnix-tarball";
        version = builtins.readFile ./version;
        src = dydisnix;
        inherit officialRelease;

        buildInputs = [ pkgconfig disnix getopt libxml2 glib ];
      };

    build =
      { tarball ? jobs.tarball {}
      , system ? "x86_64-linux"
      , disnix ? (import ../../disnix/trunk/release.nix {}).build {}
      }:

      with import nixpkgs { inherit system; };

      releaseTools.nixBuild {
        name = "dydisnix";
        src = tarball;

        buildInputs = [ pkgconfig disnix getopt libxml2 glib ];
      };      
  };
in jobs
