{system, pkgs, distribution, invDistribution}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  testService1 = {
    name = "testService1";
    pkg = customPkgs.testService1;
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
      inherit testService1;
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
      inherit testService1 testService2;
    };
    type = "echo";
    requireZone = "Europe";
    requireZones = [ "US" "Europe" ];
    requireMem = 262145;
    requireAccess = [ "private" ];
  };
}
