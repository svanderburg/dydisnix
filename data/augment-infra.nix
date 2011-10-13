{infrastructure, augmentFun}:

let pkgs = import <nixpkgs> {};
in
augmentFun { inherit infrastructure; inherit (pkgs) lib; }
