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
    # Execute filter buildable. In this situation no build exceptions
    # occur, so all machines in the network are valid candidate hosts.
    # This test should succeed.

    my $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist --filter-buildable -s ${models}/services.nix -i ${models}/infrastructure.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute filter buildable. In this situation a build exception is
    # thrown for testService1B rendering it undeployable.
    # This test should succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist --filter-buildable -s ${models}/services-error.nix -i ${models}/infrastructure.nix --output-xml");

    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1B']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1B']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute the mapAttrOnAttr method to map requireZone onto zone.
    # testService1 should be assigned to testtarget1. testService2 and
    # testService3 should be assigned to testtarget2. This test should
    # succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-mapattronattr.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute the mapAttrOnList method to map types onto supportedTypes.
    # All services must be assigned to both testtarget1 and testtarget2.
    # This test should succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-mapattronlist.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute the mapListOnAttr method to map requiredZones onto zones.
    # testService1 must be assigned to testtarget1. testService2 must be
    # assigned to testtarget2. testService3 must be assigned to both
    # machines. This test should succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-maplistonattr.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute order. The targets are order by looking to the priority
    # attribute. The order of the targets should be reversed.
    # This test should succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-order.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping[1]/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping[2]/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping[1]/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping[2]/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping[1]/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping[2]/target[text() = 'testtarget1']\" result");

    # Execute the greedy division method. testService1 and testService2
    # should be assigned to testtarget2. testService3 should be assigned
    # to testtarget1. This test should succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-greedy.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute the same division method again. It should fail because the machines cannot provide its required capacity.
    $machine->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure-undercapacity.nix -q ${models}/qos/qos-greedy.nix");

    # Execute the highest bidder method. testService1 should be
    # assigned to testtarget2. testService2 should be assigned to
    # targettarget1. testService3 should be assigned to testtarget2.
    # This test should succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-highest-bidder.nix --output-xml");

    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute the same division method again. It should fail because the machines cannot provide its required capacity.
    $machine->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure-undercapacity.nix -q ${models}/qos/qos-highest-bidder.nix");

    # Execute the lowest bidder method. testService1 and testService2
    # should be assigned to testtarget1. testService3 should be assinged
    # to testtarget2. This test should succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-lowest-bidder.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute the same division method again. It should fail because the machines cannot provide its required capacity.
    $machine->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure-undercapacity.nix -q ${models}/qos/qos-lowest-bidder.nix");

    # Execute round robin division method.
    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-roundrobin.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute minimum set cover approximation method, by looking to the
    # cost attribute in the infrastructure model. All services should
    # be distributed to testtarget1.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-minsetcover.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute minimum set cover approximation method, by looking to the
    # cost attribute in the infrastructure model. testService1 and
    # testService2 should be distributed to testtarget1. testService3
    # should be distributed to testtarget2.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-minsetcover2.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute multiway cut approximation method.
    # In this case all services should be mapped to testtarget1.
    # This test should succeed.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-multiwaycut.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute graph coloring test. Each service should be mapped to a different machine.

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure-3.nix -q ${models}/qos/qos-graphcol.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget3']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget3']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget3']\" result");

    # Execute map stateful to previous test. First, all services are
    # mapped to testtarget1. Then an upgrade is performed in which
    # services are mapped to all targets. testService1 which is marked
    # as stateful is only mapped to testtarget1. This test should
    # succeed.

    my $firstTargets = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-firsttargets.nix");
    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${models}/services.nix -i ${models}/infrastructure.nix -d $firstTargets");
    $machine->mustSucceed("mkdir /nix/var/nix/profiles/per-user/root/disnix-coordinator");
    $machine->mustSucceed("nix-env -p /nix/var/nix/profiles/per-user/root/disnix-coordinator/default --set $result");

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-mapstatefultoprevious.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute map bind service to previous test. We first deploy all
    # services to test1. Then we deploy again and check whether all
    # services will be bound to it again.

    $firstTargets = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-firsttargets.nix");
    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${models}/services.nix -i ${models}/infrastructure.nix -d $firstTargets");
    $machine->mustSucceed("nix-env -p /nix/var/nix/profiles/per-user/root/disnix-coordinator/default --set $result");

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -q ${models}/qos/qos-mapboundservicestoprevious.nix --output-xml");

    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
    $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
    $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

    # Execute a dummy filter in a custom filters expression that erases all targets

    $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${models}/services.nix -i ${models}/infrastructure.nix -F ${models}/qos/customfilters.nix -q ${models}/qos/qos-clear.nix");
  '';
}
