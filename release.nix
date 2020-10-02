{ nixpkgs ? <nixpkgs>
, systems ? [ "i686-linux" "x86_64-linux" ]
, dydisnix ? { outPath = ./.; rev = 1234; }
, dysnomia ? { outPath = ../dysnomia; rev = 1234; }
, disnix ? { outPath = ../disnix; rev = 1234; }
, officialRelease ? false
}:

let
  pkgs = import nixpkgs {};

  dysnomiaJobset = import "${dysnomia}/release.nix" {
    inherit nixpkgs systems officialRelease dysnomia;
  };

  disnixJobset = import "${disnix}/release.nix" {
    inherit nixpkgs systems officialRelease dysnomia disnix;
  };

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
        dysnomia = builtins.getAttr (builtins.currentSystem) (dysnomiaJobset.build);
        disnix = builtins.getAttr (builtins.currentSystem) (disnixJobset.build);
        dydisnix = builtins.getAttr (builtins.currentSystem) build;
      in
      {
        augmentInfra = import ./tests/augment-infra.nix {
          inherit nixpkgs pkgs dysnomia disnix dydisnix;
        };

        gendist = import ./tests/gendist.nix {
          inherit nixpkgs pkgs dysnomia disnix dydisnix;
        };

        idassign = import ./tests/idassign.nix {
          inherit nixpkgs pkgs dysnomia disnix dydisnix;
        };

        docs = import ./tests/docs.nix {
          inherit nixpkgs pkgs dysnomia disnix dydisnix;
        };

        deployment = import ./tests/deployment.nix {
          inherit nixpkgs pkgs dysnomia disnix dydisnix;
        };
      };

    release = pkgs.releaseTools.aggregate {
      name = "dydisnix-${tarball.version}";
      constituents = [
        tarball
      ]
      ++ map (system: builtins.getAttr system build) systems
      ++ [
        tests.augmentInfra
        tests.gendist
        tests.idassign
        tests.docs
        tests.deployment
      ];
      meta.description = "Release-critical builds";
    };
  };
in
jobs
