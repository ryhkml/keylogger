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
        mkPackage =
          websockets:
          pkgs.stdenv.mkDerivation {
            name = "keylogger";
            src = pkgs.lib.cleanSourceWith {
              src = ./.;
              filter = path: type: !(pkgs.lib.hasSuffix ".png" path || pkgs.lib.hasSuffix ".txt" path);
            };

            nativeBuildInputs =
              [
                pkgs.gcc
                pkgs.gnumake
              ]
              ++ (pkgs.lib.optional websockets [
                pkgs.libwebsockets.dev
                pkgs.openssl.dev
              ]);

            buildPhase = ''
              make ${if websockets then "USE_LIBWEBSOCKETS=1" else ""}
            '';

            installPhase = ''
              mkdir -p $out/bin
              cp out/keylogger $out/bin/
            '';
          };
      in
      {
        packages = {
          default = mkPackage false;
          with-websockets = mkPackage true;
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
          with-websockets = {
            type = "app";
            program =
              let
                wrapper = pkgs.writeShellScriptBin "keylogger" ''
                  ${self.packages.${system}.with-websockets}/bin/keylogger "$@"
                '';
              in
              "${wrapper}/bin/keylogger";
          };
        };
        devShells.default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.gcc
            pkgs.gnumake
            pkgs.libwebsockets.dev
            pkgs.openssl.dev
          ];

          shellHook = ''
            echo "Keylogger development shell"
            echo "Build options:"
            echo "1. Default build: nix build"
            echo "2. With websockets: nix build .#with-websockets"
          '';
        };
      }
    );
}
