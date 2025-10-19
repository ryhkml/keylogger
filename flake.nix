{
  description = "Keylogger for Linux. Leaks your keyboard input";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    flake-utils.lib.eachSystem [ "x86_64-linux" ] (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        mkPackage = pkgs.stdenv.mkDerivation {
          name = "keylogger";
          src = pkgs.lib.cleanSourceWith {
            src = ./.;
            filter =
              path: type:
              !(
                pkgs.lib.hasSuffix ".mp4" path || pkgs.lib.hasSuffix ".png" path || pkgs.lib.hasSuffix ".txt" path
              );
          };
          nativeBuildInputs = [
            pkgs.gcc
            pkgs.libwebsockets.dev
            pkgs.openssl.dev
          ];
          buildPhase = ''
            gcc -o nob nob.c
            ./nob
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp out/keylogger $out/bin/
          '';
        };
      in
      {
        packages = {
          default = mkPackage;
        };
        apps = {
          default = {
            type = "app";
            program =
              let
                wrapper = pkgs.writeShellScriptBin "keylogger" ''
                  ${self.packages.${system}.default}/bin/keylogger "$@"
                '';
              in
              "${wrapper}/bin/keylogger";
          };
        };
        devShells.default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.gcc
            pkgs.libwebsockets.dev
            pkgs.openssl.dev
          ];
          shellHook = ''
            echo "Keylogger development shell"
          '';
        };
      }
    );
}
