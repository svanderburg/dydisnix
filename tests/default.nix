{nixpkgs, pkgs, disnix, dydisnix}:

with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };

let
  tests = ./.;
in
{
  install = simpleTest {
    nodes = {
      machine =
        {config, pkgs, ...}:

        {
          virtualisation.writableStore = true;
          virtualisation.pathsInNixDB = [ pkgs.stdenv pkgs.perlPackages.ArchiveCpio pkgs.busybox ];

          # We can't download any substitutes in a test environment. To make tests
          # faster, we disable substitutes so that Nix does not waste any time by
          # attempting to download them.
          nix.extraOptions = ''
            substitute = false
          '';

          environment.systemPackages = [ disnix dydisnix pkgs.stdenv pkgs.graphviz ] ++ pkgs.libxml2.all ++ pkgs.libxslt.all;
        };
      };
      testScript = ''
        # Test augment infra. For each target in the infrastructure model
        # we add the attribute: augment = "augment". This test should
        # succeed.

        my $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-augment-infra -i ${tests}/infrastructure.nix -a ${tests}/augment.nix");
        $machine->mustSucceed("[ \"\$((NIX_PATH='nixpkgs=${nixpkgs}' nix-instantiate --eval-only --xml --strict $result) | grep 'augment')\" != \"\" ]");

        # Execute filter buildable. In this situation no build exceptions
        # occur, so all machines in the network are valid candidate hosts.
        # This test should succeed.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist --filter-buildable -s ${tests}/services.nix -i ${tests}/infrastructure.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute filter buildable. In this situation a build exception is
        # thrown for testService1B rendering it undeployable.
        # This test should succeed.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist --filter-buildable -s ${tests}/services-error.nix -i ${tests}/infrastructure.nix --output-xml");

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

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapattronattr.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute the mapAttrOnList method to map types onto supportedTypes.
        # All services must be assigned to both testtarget1 and testtarget2.
        # This test should succeed.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapattronlist.nix --output-xml");

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

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-maplistonattr.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute order. The targets are order by looking to the priority
        # attribute. The order of the targets should be reversed.
        # This test should succeed.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-order.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping[1]/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping[2]/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping[1]/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping[2]/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping[1]/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping[2]/target[text() = 'testtarget1']\" result");

        # Execute the greedy division method. testService1 and testService2
        # should be assigned to testtarget2. testService3 should be assigned
        # to testtarget1. This test should succeed.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-greedy.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute the same division method again. It should fail because the machines cannot provide its required capacity.
        $machine->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure-undercapacity.nix -q ${tests}/qos/qos-greedy.nix");

        # Execute the highest bidder method. testService1 should be
        # assigned to testtarget2. testService2 should be assigned to
        # targettarget1. testService3 should be assigned to testtarget2.
        # This test should succeed.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-highest-bidder.nix --output-xml");

        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute the same division method again. It should fail because the machines cannot provide its required capacity.
        $machine->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure-undercapacity.nix -q ${tests}/qos/qos-highest-bidder.nix");

        # Execute the lowest bidder method. testService1 and testService2
        # should be assigned to testtarget1. testService3 should be assinged
        # to testtarget2. This test should succeed.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-lowest-bidder.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute the same division method again. It should fail because the machines cannot provide its required capacity.
        $machine->mustFail("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure-undercapacity.nix -q ${tests}/qos/qos-lowest-bidder.nix");

        # Execute round robin division method.
        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-roundrobin.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute minimum set cover approximation method, by looking to the
        # cost attribute in the infrastructure model. All services should
        # be distributed to testtarget1.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-minsetcover.nix --output-xml");

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

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-minsetcover2.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute multiway cut approximation method.
        # In this case all services should be mapped to testtarget1.
        # This test should succeed.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-multiwaycut.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute graph coloring test. Each service should be mapped to a different machine.

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure-3.nix -q ${tests}/qos/qos-graphcol.nix --output-xml");

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

        my $firstTargets = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-firsttargets.nix");
        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${tests}/services.nix -i ${tests}/infrastructure.nix -d $firstTargets");
        $machine->mustSucceed("mkdir /nix/var/nix/profiles/per-user/root/disnix-coordinator");
        $machine->mustSucceed("nix-env -p /nix/var/nix/profiles/per-user/root/disnix-coordinator/default --set $result");

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapstatefultoprevious.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute map bind service to previous test. We first deploy all
        # services to test1. Then we deploy again and check whether all
        # services will be bound to it again.

        $firstTargets = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-firsttargets.nix");
        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${tests}/services.nix -i ${tests}/infrastructure.nix -d $firstTargets");
        $machine->mustSucceed("nix-env -p /nix/var/nix/profiles/per-user/root/disnix-coordinator/default --set $result");

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapboundservicestoprevious.nix --output-xml");

        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService1']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService2']/mapping/target[text() = 'testtarget2']\" result");
        $machine->mustSucceed("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget1']\" result");
        $machine->mustFail("xmllint --xpath \"/distribution/service[\@name='testService3']/mapping/target[text() = 'testtarget2']\" result");

        # Execute a dummy filter in a custom filters expression that erases all targets

        $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -F ${tests}/qos/customfilters.nix -q ${tests}/qos/qos-clear.nix");

        # Execute port assignment test. First, we assign a unique port number
        # to each service using a shared ports pool among all machines.

        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${tests}/services-sharedports.nix -i ${tests}/infrastructure.nix -d ${tests}/distribution.nix > ports.nix");

        # We run the same command again with the generated port configuration
        # as extra parameter. The generated ports configuration should be
        # identical.

        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${tests}/services-sharedports.nix -i ${tests}/infrastructure.nix -d ${tests}/distribution.nix -p ports.nix > ports2.nix");
        $machine->mustSucceed("[ \"\$(diff ports.nix ports2.nix | wc -l)\" = \"0\" ]");

        # We delete the first service. The first one must be removed, but the
        # remaining port assignments should remain identical.
        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${tests}/services-sharedports2.nix -i ${tests}/infrastructure.nix -d ${tests}/distribution2.nix -p ports2.nix > ports3.nix");

        $machine->mustFail("grep '    testService1 =' ports3.nix");
        $machine->mustSucceed("grep \"\$(grep '    \"testService2\" ='  ports.nix | head -1)\" ports3.nix");
        $machine->mustSucceed("grep \"\$(grep '    \"testService3\" ='  ports.nix | head -1)\" ports3.nix");

        # We add the first service again. It should have received a new port
        # number.
        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${tests}/services-sharedports.nix -i ${tests}/infrastructure.nix -d ${tests}/distribution.nix -p ports3.nix > ports4.nix");
        $machine->mustSucceed("[ \"\$(diff ports.nix ports4.nix | wc -l)\" != \"0\" ]");

        # We now remove the portAssign attribute for the first service. It
        # should disappear from the ports configuration.
        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${tests}/services-sharedports3.nix -i ${tests}/infrastructure.nix -d ${tests}/distribution.nix -p ports4.nix > ports5.nix");
        $machine->mustFail("grep '    \"testService1\" =' ports5.nix");

        # Map two services with private port assignments to the same
        # machine. They should have different port numbers
        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${tests}/services-privateports.nix -i ${tests}/infrastructure.nix -d ${tests}/distribution2.nix > ports.nix");
        $machine->mustSucceed("grep '\"testService2\" = 8001;' ports.nix");
        $machine->mustSucceed("grep '\"testService3\" = 8002;' ports.nix");

        # Map two services with private port assignments to different
        # machines. They should have the same port number
        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${tests}/services-privateports.nix -i ${tests}/infrastructure.nix -d ${tests}/distribution3.nix > ports.nix");
        $machine->mustSucceed("grep '\"testService2\" = 8001;' ports.nix");
        $machine->mustSucceed("grep '\"testService3\" = 8001;' ports.nix");

        # Make the testService3 reservation private. It should have a
        # different port number, while testService2's port number remains
        # the same.
        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-port-assign -s ${tests}/services-sharedprivateport.nix -i ${tests}/infrastructure.nix -d ${tests}/distribution3.nix -p ports.nix > ports2.nix");
        $machine->mustSucceed("grep '\"testService2\" = 8001;' ports2.nix");
        $machine->mustSucceed("grep '\"testService3\" = 3001;' ports2.nix");

        # Generate a documentation catalog
        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-generate-services-docs -s ${tests}/visualize/services.nix -f svg --output-dir \$TMPDIR/out1");

        # Generate a documentation catalog with extra documentation properties
        $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-generate-services-docs -s ${tests}/visualize/services.nix --docs ${tests}/visualize/docs.nix -f svg --output-dir \$TMPDIR/out2");
      '';
    };
}
