{infrastructure, augumentFun}:

let pkgs = import (builtins.getEnv "NIXPKGS_ALL") {};
in
augumentFun { inherit infrastructure pkgs; }
