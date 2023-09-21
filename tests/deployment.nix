{nixpkgs, pkgs, dysnomia, disnix, dydisnix}:

with import "${nixpkgs}/nixos/lib/testing-python.nix" { system = builtins.currentSystem; };

let
  machine = import ./machine.nix {
    inherit dysnomia disnix dydisnix;
  };

  models = ./models;
in
simpleTest {
  name = "deployment";
  nodes = {
    coordinator = machine;
    testtarget1 = machine;
    testtarget2 = machine;
  };
  testScript =
    let
      env = "SSH_OPTS='-o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' DISNIX_REMOTE_CLIENT='disnix-run-activity' NIX_PATH='nixpkgs=${nixpkgs}'";
    in
    ''
      import subprocess

      start_all()

      # Initialise ssh stuff by creating a key pair for communication
      key = subprocess.check_output(
          '${pkgs.openssh}/bin/ssh-keygen -t ecdsa -f key -N ""',
          shell=True,
      )

      testtarget1.succeed("mkdir -m 700 /root/.ssh")
      testtarget1.copy_from_host("key.pub", "/root/.ssh/authorized_keys")

      testtarget2.succeed("mkdir -m 700 /root/.ssh")
      testtarget2.copy_from_host("key.pub", "/root/.ssh/authorized_keys")

      coordinator.succeed("mkdir -m 700 /root/.ssh")
      coordinator.copy_from_host("key", "/root/.ssh/id_dsa")
      coordinator.succeed("chmod 600 /root/.ssh/id_dsa")

      # Test dydisnix-env by deploying a system with an augmented infrastructure model, dynamically generated distribution, and dynamically assigned IDs

      coordinator.succeed(
          "${env} dydisnix-env -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -a ${models}/augment.nix -q ${models}/qos/qos-roundrobin.nix --id-resources ${models}/idresources-global.nix --ids ids.nix"
      )
    '';
}
