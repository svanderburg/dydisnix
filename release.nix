{ nixpkgs ? <nixpkgs>
, systems ? [ "i686-linux" "x86_64-linux" ]
, disnixJobset ? import ../disnix/release.nix { inherit nixpkgs systems officialRelease; }
, dydisnix ? {outPath = ./.; rev = 1234;}
, officialRelease ? false
}:

let
  pkgs = import nixpkgs {};
  
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
        
        buildInputs = [ pkgs.pkgconfig pkgs.getopt pkgs.libxml2 pkgs.glib disnix ]
          ++ pkgs.lib.optional (!pkgs.stdenv.isLinux) pkgs.libiconv
          ++ pkgs.lib.optional (!pkgs.stdenv.isLinux) pkgs.gettext;
      });

    tests = 
      let
        disnix = builtins.getAttr (builtins.currentSystem) (disnixJobset.build);
        dydisnix = builtins.getAttr (builtins.currentSystem) build;
        tests = ./tests;
      in
      with import "${nixpkgs}/nixos/lib/testing.nix" { system = builtins.currentSystem; };
      
      {
        install = simpleTest {
          nodes = {
            machine =
              {config, pkgs, ...}:
              
              {
                virtualisation.writableStore = true;
                environment.systemPackages = [
                  disnix dydisnix pkgs.stdenv
                  pkgs.busybox pkgs.paxctl pkgs.gnumake pkgs.patchelf pkgs.gcc pkgs.perlPackages.ArchiveCpio # Required to build something in the VM
                ];
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
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist --filter-buildable -s ${tests}/services.nix -i ${tests}/infrastructure.nix");
            my @distribution = split('\n', $machine->mustSucceed("cat $result"));
          
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
          
            if($distribution[8] =~ /testtarget2/) {
                print "line 8 contains testtarget2!\n";
            } else {
                die "line 8 should contain testtarget2!\n";
            }
            
            if($distribution[13] =~ /testtarget1/) {
                print "line 13 contains testtarget1!\n";
            } else {
                die "line 13 should contain testtarget1!\n";
            }
          
            if($distribution[14] =~ /testtarget2/) {
                print "line 14 contains testtarget2!\n";
            } else {
                die "line 14 should contain testtarget2!\n";
            }
            
            if($distribution[19] =~ /testtarget1/) {
                print "line 19 contains testtarget1!\n";
            } else {
                die "line 19 should contain testtarget1!\n";
            }
          
            if($distribution[20] =~ /testtarget2/) {
                print "line 20 contains testtarget2!\n";
            } else {
                die "line 20 should contain testtarget2!\n";
            }
          
            # Execute filter buildable. In this situation a build exception is
            # thrown for testService1B rendering it undeployable.
            # This test should succeed.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist --filter-buildable -s ${tests}/services-error.nix -i ${tests}/infrastructure.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
          
            if($distribution[7] =~ /testtarget1/) {
                die "line 7 contains testtarget1!\n";
            } else {
                print "line 7 should contain testtarget1!\n";
            }
          
            # Execute the mapAttrOnAttr method to map requireZone onto zone.
            # testService1 should be assigned to testtarget1. testService2 and
            # testService3 should be assigned to testtarget2. This test should
            # succeed.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapattronattr.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[12] =~ /testtarget1/) {
                print "line 12 contains testtarget1!\n";
            } else {
                die "line 12 should contain testtarget1!\n";
            }
            
            if($distribution[17] =~ /testtarget2/) {
                print "line 17 contains testtarget2!\n";
            } else {
                die "line 17 should contain testtarget2!\n";
            }
          
            # Execute the mapAttrOnList method to map types onto supportedTypes.
            # All services must be assigned to both testtarget1 and testtarget2.
            # This test should succeed.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapattronlist.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[8] =~ /testtarget2/) {
                print "line 8 contains testtarget2!\n";
            } else {
                die "line 8 should contain testtarget2!\n";
            }
            
            if($distribution[13] =~ /testtarget1/) {
                print "line 13 contains testtarget1!\n";
            } else {
                die "line 13 should contain testtarget1!\n";
            }
            
            if($distribution[14] =~ /testtarget2/) {
                print "line 14 contains testtarget1!\n";
            } else {
                die "line 14 should contain testtarget1!\n";
            }
            
            if($distribution[19] =~ /testtarget1/) {
                print "line 19 contains testtarget1!\n";
            } else {
                die "line 19 should contain testtarget1!\n";
            }
            
            if($distribution[20] =~ /testtarget2/) {
                print "line 20 contains testtarget2!\n";
            } else {
                die "line 20 should contain testtarget2!\n";
            }
          
            # Execute the mapListOnAttr method to map requiredZones onto zones.
            # testService1 must be assigned to testtarget1. testService2 must be
            # assigned to testtarget2. testService3 must be assigned to both
            # machines. This test should succeed.
          
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-maplistonattr.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[12] =~ /testtarget2/) {
                print "line 12 contains testtarget2!\n";
            } else {
                die "line 12 should contain testtarget2!\n";
            }
          
            if($distribution[17] =~ /testtarget1/) {
                print "line 17 contains testtarget1!\n";
            } else {
                die "line 17 should contain testtarget1!\n";
            }
            
            if($distribution[18] =~ /testtarget2/) {
                print "line 18 contains testtarget2!\n";
            } else {
                die "line 18 should contain testtarget2!\n";
            }
          
            # Execute the greedy division method. testService1 and testService2
            # should be assigned to testtarget2. testService3 should be assigned
            # to testtarget1. This test should succeed.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-greedy.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[12] =~ /testtarget1/) {
                print "line 12 contains testtarget1!\n";
            } else {
                die "line 12 should contain testtarget1!\n";
            }
            
            if($distribution[17] =~ /testtarget2/) {
                print "line 17 contains testtarget2!\n";
            } else {
                die "line 17 should contain testtarget2!\n";
            }
            
            # Execute order. The targets are order by looking to the priority
            # attribute. The order of the targets should be reversed.
            # This test should succeed.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-order.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget2/) {
                print "line 7 contains testtarget2!\n";
            } else {
                die "line 7 should contain testtarget2!\n";
            }
            
            if($distribution[8] =~ /testtarget1/) {
                print "line 8 contains testtarget1!\n";
            } else {
                die "line 8 should contain testtarget1!\n";
            }
            # Execute the highest bidder method. testService1 should be
            # assigned to testtarget2. testService2 should be assigned to
            # targettarget1. testService3 should be assigned to testtarget2.
            # This test should succeed.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-highest-bidder.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget2/) {
                print "line 7 contains testtarget2!\n";
            } else {
                die "line 7 should contain testtarget2!\n";
            }
            
            if($distribution[12] =~ /testtarget1/) {
                print "line 12 contains testtarget1!\n";
            } else {
                die "line 12 should contain testtarget1!\n";
            }
            
            if($distribution[17] =~ /testtarget2/) {
                print "line 17 contains testtarget2!\n";
            } else {
                die "line 17 should contain testtarget2!\n";
            }
            
            # Execute the lowest bidder method. testService1 and testService2
            # should be assigned to testtarget1. testService3 should be assinged
            # to testtarget2. This test should succeed.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-lowest-bidder.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[12] =~ /testtarget1/) {
                print "line 12 contains testtarget1!\n";
            } else {
                die "line 12 should contain testtarget1!\n";
            }
            
            if($distribution[17] =~ /testtarget2/) {
                print "line 17 contains testtarget2!\n";
            } else {
                die "line 17 should contain testtarget2!\n";
            }
            
            # Execute minimum set cover approximation method, by looking to the
            # cost attribute in the infrastructure model. All services should
            # be distributed to testtarget1.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-minsetcover.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[12] =~ /testtarget1/) {
                print "line 12 contains testtarget1!\n";
            } else {
                die "line 12 should contain testtarget1!\n";
            }
            
            if($distribution[17] =~ /testtarget1/) {
                print "line 17 contains testtarget1!\n";
            } else {
                die "line 17 should contain testtarget1!\n";
            }
            
            # Execute minimum set cover approximation method, by looking to the
            # cost attribute in the infrastructure model. testService1 and
            # testService2 should be distributed to testtarget1. testService3
            # should be distributed to testtarget2.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-minsetcover2.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[12] =~ /testtarget1/) {
                print "line 12 contains testtarget1!\n";
            } else {
                die "line 12 should contain testtarget1!\n";
            }
            
            if($distribution[17] =~ /testtarget2/) {
                print "line 17 contains testtarget2!\n";
            } else {
                die "line 17 should contain testtarget2!\n";
            }
            
            # Execute multiway cut approximation method.
            # In this case all services should be mapped to testtarget1.
            # This test should succeed.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-multiwaycut.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[12] =~ /testtarget1/) {
                print "line 12 contains testtarget1!\n";
            } else {
                die "line 12 should contain testtarget1!\n";
            }
            
            if($distribution[17] =~ /testtarget1/) {
                print "line 17 contains testtarget1!\n";
            } else {
                die "line 17 should contain testtarget1!\n";
            }
            
            # Execute graph coloring test. Each service should be mapped to a different machine.
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure-3.nix -q ${tests}/qos/qos-graphcol.nix");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[7] =~ /testtarget1/) {
                print "line 7 contains testtarget1!\n";
            } else {
                die "line 7 should contain testtarget1!\n";
            }
            
            if($distribution[12] =~ /testtarget2/) {
                print "line 12 contains testtarget2!\n";
            } else {
                die "line 12 should contain testtarget2!\n";
            }
            
            if($distribution[17] =~ /testtarget3/) {
                print "line 17 contains testtarget3!\n";
            } else {
                die "line 17 should contain testtarget3!\n";
            }
            
            # Execute map stateful to previous test. First, all services are
            # mapped to testtarget1. Then an upgrade is performed in which
            # services are mapped to all targets. testService1 which is marked
            # as stateful is only mapped to testtarget1. This test should
            # succeed. (BROKEN since manifest file format has been changed)
            
            my $firstTargets = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-firsttargets.nix");
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' disnix-manifest -s ${tests}/services.nix -i ${tests}/infrastructure.nix -d $firstTargets");
            $machine->mustSucceed("mkdir /nix/var/nix/profiles/per-user/root/disnix-coordinator");
            $machine->mustSucceed("nix-env -p /nix/var/nix/profiles/per-user/root/disnix-coordinator/default --set $result");
            
            $result = $machine->mustSucceed("NIX_PATH='nixpkgs=${nixpkgs}' dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapstatefultoprevious.nix");
            $machine->mustSucceed("(cat $result) >&2");
            @distribution = split('\n', $machine->mustSucceed("cat $result"));
            
            if($distribution[11] =~ /testtarget1/) {
                print "line 11 contains testtarget1!\n";
            } else {
                die "line 11 should contain testtarget1! Instead: $distribution[10]\n";
            }
            
            if($distribution[12] =~ /testtarget2/) {
                die "line 12 contains testtarget2!\n";
            } else {
                print "line 12 does not contain testtarget2!\n";
            }
          '';
        };
      };
  };
in jobs
