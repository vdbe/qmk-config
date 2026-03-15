{
  description = "Personal QMK config environment";

  inputs = {
    nixpkgs.url = "https://nixpkgs.dev/channel/nixpkgs-unstable";
  };

  outputs =
    { self, nixpkgs, ... }:
    let
      inherit (nixpkgs) lib;
      inherit (lib.attrsets) genAttrs;

      pkgsFor = pkgs: system: import pkgs { inherit system; };

      allSystems = lib.systems.flakeExposed;

      forAllSystems =
        f:
        genAttrs allSystems (
          system:
          f {
            inherit system;
            pkgs = pkgsFor nixpkgs system;
          }
        );
    in
    {
      formatter = forAllSystems ({ pkgs, ... }: pkgs.nixfmt);

      devShells = forAllSystems (
        { pkgs, system, ... }:
        let
          default = qmk;
          qmk = pkgs.mkShellNoCC {
            packages = [
              pkgs.qmk
              pkgs.dos2unix
            ];
          };

          nix = default.overrideAttrs (old: {
            packages = (old.packages or []) ++ [
              self.formatter.${system}
              pkgs.nil
              pkgs.statix
            ];
          });
        in
        {
          inherit default qmk nix;
        }
      );
    };
}
