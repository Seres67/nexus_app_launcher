{
  description = "A Nexus addon to display a modular keyboard overlay.";
  inputs = {
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = {
    flake-utils,
    nixpkgs,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
        crossSystem = {
          config = "x86_64-w64-mingw32";
        };
      };
    in {
      packages.default = pkgs.callPackage ./package.nix {};
      devShells.default = pkgs.callPackage ./shell.nix {};
    });
}
