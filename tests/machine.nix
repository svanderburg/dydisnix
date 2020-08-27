{disnix, dydisnix}:
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
}
