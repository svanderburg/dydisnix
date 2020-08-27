{nixpkgs, pkgs, disnix, dydisnix}:

with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

let
  machine = import ./machine.nix {
    inherit disnix dydisnix;
  };

  models = ./visualize;
in

simpleTest {
  nodes = {
    inherit machine;
  };
  testScript = ''
    # Generate a documentation catalog
    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-generate-services-docs -s ${models}/services.nix -f svg --output-dir \$TMPDIR/out1");

    # Generate a documentation catalog with extra documentation properties
    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-generate-services-docs -s ${models}/services.nix --docs ${models}/docs.nix -f svg --output-dir \$TMPDIR/out2");
  '';
}
