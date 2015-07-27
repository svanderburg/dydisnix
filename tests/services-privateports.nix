{distribution, system, pkgs}:

let
  customPkgs = import ./pkgs { inherit pkgs system; };
in
rec {
  testService2 = {
    name = "testService2";
    pkg = customPkgs.testService2;
    type = "echo";
    portAssign = "private";
  };
  
  testService3 = {
    name = "testService3";
    pkg = customPkgs.testService3;
    dependsOn = {
      inherit testService2;
    };
    type = "echo";
    portAssign = "private";
  };
}
