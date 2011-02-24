{infrastructure, augmentFun}:

let pkgs = import (builtins.getEnv "NIXPKGS_ALL") {};
in
augmentFun { inherit infrastructure; inherit (pkgs) lib; }
