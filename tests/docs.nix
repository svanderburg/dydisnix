{nixpkgs, pkgs, dysnomia, disnix, dydisnix}:

with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

let
  machine = import ./machine.nix {
    inherit dysnomia disnix dydisnix;
  };

  models = ./visualize;
in

simpleTest {
  nodes = {
    inherit machine;
  };
  testScript =
    let
      env = "NIX_PATH='nixpkgs=${nixpkgs}'";
    in
    ''
      # Generate a documentation catalog
      machine.succeed(
          "${env} dydisnix-generate-services-docs -s ${models}/services.nix -f svg --output-dir $TMPDIR/out1"
      )

      # Generate a documentation catalog with extra documentation properties
      machine.succeed(
          "${env} dydisnix-generate-services-docs -s ${models}/services.nix --docs ${models}/docs.nix -f svg --output-dir $TMPDIR/out2"
      )
    '';
}
