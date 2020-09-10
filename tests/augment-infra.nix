{nixpkgs, pkgs, dysnomia, disnix, dydisnix}:

with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

let
  machine = import ./machine.nix {
    inherit dysnomia disnix dydisnix;
  };

  models = ./models;
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
      # Test augment infra. For each target in the infrastructure model
      # we add the attribute: augment = "augment". This test should
      # succeed.

      my $result = $machine->mustSucceed("${env} dydisnix-augment-infra -i ${models}/infrastructure.nix -a ${models}/augment.nix");
      $machine->mustSucceed("[ \"\$((${env} nix-instantiate --eval-only --xml --strict $result) | grep 'augment')\" != \"\" ]");
    '';
}
