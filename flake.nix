{
  description = "Personal QMK config environment";

  inputs = {
    nixpkgs.url = "https://channels.nixos.org/nixpkgs-unstable/nixexprs.tar.xz";
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
          qmk = pkgs.mkShell {
            packages = [
              pkgs.clang-tools
              pkgs.dos2unix
              pkgs.keymap-drawer
              pkgs.qmk
            ];
          };

          nix = pkgs.mkShell {
            inputsFrom = [ qmk ];
            packages = [
              self.formatter.${system}
              pkgs.nil
              pkgs.statix
            ];
          };
        in
        {
          inherit default qmk nix;
        }
      );
    };
}
