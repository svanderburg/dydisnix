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
      # Execute port assignment test. First, we assign a unique port number
      # to each service using a shared ports pool among all machines.

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix --id-resources ${models}/idresources-global.nix --output-xml --output-file ids.xml");
      my $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService1']/text()\" ids.xml");

      if($result eq "3000\n") {
          print "Port is 3000!\n";
      } else {
          die "Assigned port should be 3000, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");

      if($result eq "3001\n") {
          print "Port is 3001!\n";
      } else {
          die "Assigned port should be 3001, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      if($result eq "3002\n") {
          print "Port is 3002!\n";
      } else {
          die "Assigned port should be 3002, instead it is: $result!";
      }

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix --id-resources ${models}/idresources-global.nix --output-file ids.nix");

      # We run the same command again with the ids configuration
      # as extra parameter. The generated ids configuration should be
      # identical.

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix --id-resources ${models}/idresources-global.nix --ids ids.nix --output-xml --output-file ids.xml");
      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService1']/text()\" ids.xml");

      if($result eq "3000\n") {
          print "Port is 3000!\n";
      } else {
          die "Assigned port should be 3000, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");

      if($result eq "3001\n") {
          print "Port is 3001!\n";
      } else {
          die "Assigned port should be 3001, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      if($result eq "3002\n") {
          print "Port is 3002!\n";
      } else {
          die "Assigned port should be 3002, instead it is: $result!";
      }

      # We delete the first service. The first one must be removed, but the
      # remaining port assignments should remain identical.

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids2.nix -i ${models}/infrastructure.nix -d ${models}/distribution2.nix --id-resources ${models}/idresources-global.nix --ids ids.nix --output-xml --output-file ids.xml");

      $machine->mustFail("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService1']/text()\" ids.xml");
      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");

      if($result eq "3001\n") {
          print "Port is 3001!\n";
      } else {
          die "Assigned port should be 3001, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      if($result eq "3002\n") {
          print "Port is 3002!\n";
      } else {
          die "Assigned port should be 3002, instead it is: $result!";
      }

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids2.nix -i ${models}/infrastructure.nix -d ${models}/distribution2.nix --id-resources ${models}/idresources-global.nix --ids ids.nix --output-file ids.nix");

      # We add the first service again. It should have received a new port
      # number (an higher id than the previously highest assigned id).

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix --id-resources ${models}/idresources-global.nix --ids ids.nix --output-xml --output-file ids.xml");
      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService1']/text()\" ids.xml");

      if($result eq "3003\n") {
          print "Port is 3003!\n";
      } else {
          die "Assigned port should be 3003, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");

      if($result eq "3001\n") {
          print "Port is 3001!\n";
      } else {
          die "Assigned port should be 3001, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      if($result eq "3002\n") {
          print "Port is 3002!\n";
      } else {
          die "Assigned port should be 3002, instead it is: $result!";
      }

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix --id-resources ${models}/idresources-global.nix --ids ids.nix --output-file ids.nix");

      # We now deploy the system with a different resource configuration no longer defining ports.
      # The IDs should disappear from the id configuration.

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -d ${models}/distribution.nix --id-resources ${models}/idresources-differentresource.nix --ids ids.nix --output-xml --output-file ids.xml");

      $machine->mustFail("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService1']/text()\" ids.xml");
      $machine->mustFail("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");
      $machine->mustFail("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      # Map two services with private ID assignments to the same
      # machine. They should have different IDs

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -d ${models}/distribution2.nix --id-resources ${models}/idresources-machine.nix --output-xml --output-file ids.xml ");

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");

      if($result eq "3000\n") {
          print "Port is 3000!\n";
      } else {
          die "Assigned port should be 3000, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      if($result eq "3001\n") {
          print "Port is 3001!\n";
      } else {
          die "Assigned port should be 3001, instead it is: $result!";
      }

      # Map two services with machine level id assignments to different
      # machines. They should have the same id

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix -i ${models}/infrastructure.nix -d ${models}/distribution3.nix --id-resources ${models}/idresources-machine.nix --output-xml --output-file ids.xml ");

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");

      if($result eq "3000\n") {
          print "Port is 3000!\n";
      } else {
          die "Assigned port should be 3000, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      if($result eq "3000\n") {
          print "Port is 3000!\n";
      } else {
          die "Assigned port should be 3000, instead it is: $result!";
      }

      # Execute port assignment test. We assign unique IDs to all services in a service model.

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix --id-resources ${models}/idresources-global.nix --output-xml --output-file ids.xml");
      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService1']/text()\" ids.xml");

      if($result eq "3000\n") {
          print "Port is 3000!\n";
      } else {
          die "Assigned port should be 3000, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");

      if($result eq "3001\n") {
          print "Port is 3001!\n";
      } else {
          die "Assigned port should be 3001, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      if($result eq "3002\n") {
          print "Port is 3002!\n";
      } else {
          die "Assigned port should be 3002, instead it is: $result!";
      }

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix --id-resources ${models}/idresources-global.nix --output-file ids.nix");

      # Reassign port numbers from a resource pool with different boundaries.
      # This invalidates the second and third assignment which should get new and lower port numbers.

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix --id-resources ${models}/idresources-global-different-boundaries.nix --ids ids.nix --output-xml --output-file ids.xml");
      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService1']/text()\" ids.xml");

      if($result eq "3000\n") {
          print "Port is 3000!\n";
      } else {
          die "Assigned port should be 3000, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");

      if($result eq "2000\n") {
          print "Port is 2000!\n";
      } else {
          die "Assigned port should be 2000, instead it is: $result!";
      }

      $result = $machine->mustSucceed("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");

      if($result eq "2001\n") {
          print "Port is 2001!\n";
      } else {
          die "Assigned port should be 2001, instead it is: $result!";
      }

      # We now deploy the system with a different resource configuration no longer defining ports.
      # The IDs should disappear from the id configuration.

      $machine->mustSucceed("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix --id-resources ${models}/idresources-differentresource.nix --ids ids.nix --output-xml --output-file ids.xml");

      $machine->mustFail("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService1']/text()\" ids.xml");
      $machine->mustFail("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService2']/text()\" ids.xml");
      $machine->mustFail("xmllint --xpath \"/ids/ids/resource[\@name='ports']/assignment[\@name='testService3']/text()\" ids.xml");
      $machine->mustFail("xmllint --xpath \"/lastAssignments/assignment[\@name='ports']/text()\" ids.xml");

      # We now attempt to generate IDs for a resource pool that is too small. It should fail.

      $machine->mustFail("${env} dydisnix-id-assign -s ${models}/services-with-ids.nix --id-resources ${models}/idresources-global-small.nix --ids ids.nix --output-xml --output-file ids.xml");
    '';
}
