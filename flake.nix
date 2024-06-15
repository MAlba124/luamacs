{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
          nativeBuildInputs = with pkgs; [
            pkg-config
            gcc
            emacs
          ];
          buildInputs = with pkgs; [
            lua
          ];
        in
          with pkgs;
          {
            devShells.default = mkShell {
              inherit buildInputs nativeBuildInputs;
              LD_LIBRARY_PATH = lib.makeLibraryPath buildInputs;
            };
          }
      );
}
