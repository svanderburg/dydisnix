{nixpkgs, pkgs, disnix, dydisnix}:

with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

let
  machine = import ./machine.nix {
    inherit disnix dydisnix;
  };

  models = ./models;
in
simpleTest {
  nodes = {
    inherit machine;
  };
  testScript = ''
    # Execute port assignment test. First, we assign a unique port number
    # to each service using a shared ports pool among all machines.

    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${models}/services-sharedports.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix > ports.nix");

    # We run the same command again with the generated port configuration
    # as extra parameter. The generated ports configuration should be
    # identical.

    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${models}/services-sharedports.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix -p ports.nix > ports2.nix");
    $machine->mustSucceed("[ \"\$(diff ports.nix ports2.nix | wc -l)\" = \"0\" ]");

    # We delete the first service. The first one must be removed, but the
    # remaining port assignments should remain identical.
    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${models}/services-sharedports2.nix -i ${models}/infrastructure.nix -d ${models}/distribution2.nix -p ports2.nix > ports3.nix");

    $machine->mustFail("grep '    testService1 =' ports3.nix");
    $machine->mustSucceed("grep \"\$(grep '    \"testService2\" ='  ports.nix | head -1)\" ports3.nix");
    $machine->mustSucceed("grep \"\$(grep '    \"testService3\" ='  ports.nix | head -1)\" ports3.nix");

    # We add the first service again. It should have received a new port
    # number.
    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${models}/services-sharedports.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix -p ports3.nix > ports4.nix");
    $machine->mustSucceed("[ \"\$(diff ports.nix ports4.nix | wc -l)\" != \"0\" ]");

    # We now remove the portAssign attribute for the first service. It
    # should disappear from the ports configuration.
    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${models}/services-sharedports3.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix -p ports4.nix > ports5.nix");
    $machine->mustFail("grep '    \"testService1\" =' ports5.nix");

    # Map two services with private port assignments to the same
    # machine. They should have different port numbers
    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${models}/services-privateports.nix -i ${models}/infrastructure.nix -d ${models}/distribution2.nix > ports.nix");
    $machine->mustSucceed("grep '\"testService2\" = 8001;' ports.nix");
    $machine->mustSucceed("grep '\"testService3\" = 8002;' ports.nix");

    # Map two services with private port assignments to different
    # machines. They should have the same port number
    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${models}/services-privateports.nix -i ${models}/infrastructure.nix -d ${models}/distribution3.nix > ports.nix");
    $machine->mustSucceed("grep '\"testService2\" = 8001;' ports.nix");
    $machine->mustSucceed("grep '\"testService3\" = 8001;' ports.nix");

    # Make the testService3 reservation private. It should have a
    # different port number, while testService2's port number remains
    # the same.
    $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${models}/services-sharedprivateport.nix -i ${models}/infrastructure.nix -d ${models}/distribution3.nix -p ports.nix > ports2.nix");
    $machine->mustSucceed("grep '\"testService2\" = 8001;' ports2.nix");
    $machine->mustSucceed("grep '\"testService3\" = 3001;' ports2.nix");
  '';
}
