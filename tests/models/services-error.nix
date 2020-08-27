{system, pkgs, distribution, invDistribution}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  testService1B = {
    name = "testService1B";
    pkg = customPkgs.testService1B;
    type = "echo";
    requireZone = "US";
    requireZones = [ "US" "Asia" ];
    requireMem = 262144;
    requireAccess = [ "public" "private" ];
    stateful = true;
  };
  
  testService2 = {
    name = "testService2";
    pkg = customPkgs.testService2;
    dependsOn = {
      testService1 = testService1B;
    };
    type = "echo";
    requireZone = "US";
    requireZones = [ "Europe" "Asia" ];
    requireMem = 262143;
    requireAccess = [ "public" "private" ];
  };
  
  testService3 = {
    name = "testService3";
    pkg = customPkgs.testService3;
    dependsOn = {
      testService1 = testService1B;
      inherit testService2;
    };
    type = "echo";
    requireZone = "Europe";
    requireZones = [ "US" "Europe" ];
    requireMem = 262145;
    requireAccess = [ "private" ];
  };
}
