{ pkgs ? import <nixpkgs> {} }:

with pkgs;

mkShell rec {
  nativeBuildInputs = [
    pkg-config
    gcc
    emacs
  ];
  buildInputs = [
    lua
  ];

  LD_LIBRARY_PATH = lib.makeLibraryPath buildInputs;
}
