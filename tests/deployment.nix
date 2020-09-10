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
    coordinator = machine;
    testtarget1 = machine;
    testtarget2 = machine;
  };
  testScript = ''
    startAll;

    # Initialise ssh stuff by creating a key pair for communication
    my $key=`${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""`;

    $testtarget1->mustSucceed("mkdir -m 700 /root/.ssh");
    $testtarget1->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");

    $testtarget2->mustSucceed("mkdir -m 700 /root/.ssh");
    $testtarget2->copyFileFromHost("key.pub", "/root/.ssh/authorized_keys");

    $coordinator->mustSucceed("mkdir -m 700 /root/.ssh");
    $coordinator->copyFileFromHost("key", "/root/.ssh/id_dsa");
    $coordinator->mustSucceed("chmod 600 /root/.ssh/id_dsa");

    # Test dydisnix-env by deploying a system with an augmented infrastructure model, dynamically generated distribution, and dynamically assigned IDs

    $coordinator->mustSucceed("SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' DISNIX_REMOTE_CLIENT='disnix-run-activity' NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-env -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -a ${models}/augment.nix -q ${models}/qos/qos-roundrobin.nix --id-resources ${models}/idresources-global.nix --ids ids.nix");
  '';
}
