{
  description = " Keylogger for Linux. Leaks your keyboard input";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      packages.${system} = {
        default = pkgs.stdenv.mkDerivation {
          name = "keylogger";
          src = builtins.path {
            path = ./.;
            filter = path: type: !(nixpkgs.lib.hasSuffix ".png" path || nixpkgs.lib.hasSuffix ".txt" path);
          };
          nativeBuildInputs = [
            pkgs.gcc
            pkgs.gnumake
          ];
          doCheck = false;
          buildPhase = ''
            make
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp out/keylogger $out/bin/
          '';
        };
      };

      devShells.${system}.default = pkgs.mkShell {
        nativeBuildInputs = [
          pkgs.gcc
          pkgs.gnumake
        ];
        shellHook = ''
          echo "Keylogger requires sudo access to run. To run it without root privilege, visit this link"
          echo "https://github.com/ryhkml/keylogger?tab=readme-ov-file#rootless"
        '';
      };

      apps.${system}.default = {
        type = "app";
        program = "${self.packages.${system}.default}/bin/keylogger";
      };
    };
}
