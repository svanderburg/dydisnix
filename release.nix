{ nixpkgs ? /etc/nixos/nixpkgs }:

let
  jobs = rec {
    tarball =
      { dydisnix ? {outPath = ./.; rev = 1234;}
      , officialRelease ? false
      , disnix ? (import ../../disnix/trunk/release.nix {}).build {}
      }:

      with import nixpkgs {};

      releaseTools.sourceTarball {
        name = "dydisnix-tarball";
        version = builtins.readFile ./version;
        src = dydisnix;
        inherit officialRelease;

        buildInputs = [ pkgconfig getopt libxml2 glib disnix ];
      };

    build =
      { tarball ? jobs.tarball {}
      , system ? "x86_64-linux"
      , disnix ? (import ../../disnix/trunk/release.nix {}).build {}
      }:

      with import nixpkgs { inherit system; };

      releaseTools.nixBuild {
        name = "dydisnix";
        src = tarball;

        buildInputs = [ pkgconfig getopt libxml2 glib disnix ];	
      };

    tests = 
      { nixos ? /etc/nixos/nixos }:
      
      let
        dydisnix = build { system = "x86_64-linux"; };
	tests = ./tests;
      in
      with import "${nixos}/lib/testing.nix" { inherit nixpkgs; system = "x86_64-linux"; services = null; };
      
      {
        install = simpleTest {
	  nodes = {
	    machine =
	      {config, pkgs, ...}:    
	      
	      {
	        virtualisation.writableStore = true;
	        environment.systemPackages = [ dydisnix pkgs.stdenv ];
	      };
	  };
	  testScript = ''
	    # Execute the mapAttrOnAttr method to map requireZone onto zone.
	    # testService1 should be assigned to testtarget1. testService2 and
	    # testService3 should be assigned to testtarget2. This test should
	    # succeed.
	    
	    my $result = $machine->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapattronattr.nix");
	    my @distribution = split('\n', $machine->mustSucceed("cat $result"));
	    
	    if(@distribution[7] =~ /testtarget1/) {
	        print "line 7 contains testtarget1!\n";
	    } else {
		die "line 7 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[12] =~ /testtarget1/) {
	        print "line 12 contains testtarget1!\n";
	    } else {
		die "line 12 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[17] =~ /testtarget2/) {
	        print "line 17 contains testtarget2!\n";
	    } else {
		die "line 17 should contain testtarget2!\n";
	    }
	  
	    # Execute the mapAttrOnList method to map types onto supportedTypes.
	    # All services must be assigned to both testtarget1 and testtarget2.
	    # This test should succeed.
	    
	    my $result = $machine->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-mapattronlist.nix");
	    my @distribution = split('\n', $machine->mustSucceed("cat $result"));
	    
	    if(@distribution[7] =~ /testtarget1/) {
	        print "line 7 contains testtarget1!\n";
	    } else {
		die "line 7 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[8] =~ /testtarget2/) {
	        print "line 8 contains testtarget2!\n";
	    } else {
		die "line 8 should contain testtarget2!\n";
	    }
	    
	    if(@distribution[13] =~ /testtarget1/) {
	        print "line 13 contains testtarget1!\n";
	    } else {
		die "line 13 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[14] =~ /testtarget2/) {
	        print "line 14 contains testtarget1!\n";
	    } else {
		die "line 14 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[19] =~ /testtarget1/) {
	        print "line 19 contains testtarget1!\n";
	    } else {
		die "line 19 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[20] =~ /testtarget2/) {
	        print "line 20 contains testtarget2!\n";
	    } else {
		die "line 20 should contain testtarget2!\n";
	    }
	  
	    # Execute the greedy division method. testService1 and testService2
	    # should be assigned to testtarget2. testService3 should be assigned
	    # to testtarget1. This test should succeed.
	    
	    my $result = $machine->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-greedy.nix");
	    my @distribution = split('\n', $machine->mustSucceed("cat $result"));
	    
	    if(@distribution[7] =~ /testtarget1/) {
	        print "line 7 contains testtarget1!\n";
	    } else {
		die "line 7 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[12] =~ /testtarget1/) {
	        print "line 12 contains testtarget1!\n";
	    } else {
		die "line 12 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[17] =~ /testtarget2/) {
	        print "line 17 contains testtarget2!\n";
	    } else {
		die "line 17 should contain testtarget2!\n";
	    }
	    
	    # Execute the highest bidder method. testService1 should be
	    # assigned to testtarget2. testService2 should be assigned to
	    # targettarget1. testService3 should be assigned to testtarget2.
	    # This test should succeed.
	    
	    my $result = $machine->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-highest-bidder.nix");
	    my @distribution = split('\n', $machine->mustSucceed("cat $result"));
	    
	    if(@distribution[7] =~ /testtarget2/) {
	        print "line 7 contains testtarget2!\n";
	    } else {
	        print "line 7 should contain testtarget2!\n";
	    }
	    
	    if(@distribution[12] =~ /testtarget1/) {
	        print "line 12 contains testtarget1!\n";
	    } else {
		die "line 12 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[17] =~ /testtarget2/) {
	        print "line 17 contains testtarget2!\n";
	    } else {
		die "line 17 should contain testtarget2!\n";
	    }
	    
	    # Execute the lowest bidder method. testService1 and testService2
	    # should be assigned to testtarget1. testService3 should be assinged
	    # to testtarget2. This test should succeed.
	    
	    my $result = $machine->mustSucceed("NIXPKGS_ALL=${nixpkgs}/pkgs/top-level/all-packages.nix dydisnix-gendist -s ${tests}/services.nix -i ${tests}/infrastructure.nix -q ${tests}/qos/qos-lowest-bidder.nix");
	    my @distribution = split('\n', $machine->mustSucceed("cat $result"));
	    
	    if(@distribution[7] =~ /testtarget1/) {
	        print "line 7 contains testtarget1!\n";
	    } else {
	        print "line 7 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[12] =~ /testtarget1/) {
	        print "line 12 contains testtarget1!\n";
	    } else {
		die "line 12 should contain testtarget1!\n";
	    }
	    
	    if(@distribution[17] =~ /testtarget2/) {
	        print "line 17 contains testtarget2!\n";
	    } else {
		die "line 17 should contain testtarget2!\n";
	    }
	  '';
	};
      };
  };
in jobs
