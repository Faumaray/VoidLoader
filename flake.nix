{
  description = "A project using CMake and X11";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "minecraft-jvmti-shell";
          nativeBuildInputs = with pkgs; [
            gcc
            jdk
            cmake
            clang-tools
            gradle
          ];

          buildInputs = with pkgs; [
            xorg.libX11
            xorg.libXrandr
            xorg.libXcursor
            xorg.libX11
            jdk
          ];
          packages = with pkgs; [
            gcc
            conan
            xorg.libX11
            xorg.libXrandr
            xorg.libXcursor
            xorg.libX11
            jdk
          ];
          #conan install . --output-folder=build --build=missing
          #source build/build/Release/generators/conanbuild.sh
          #cmake --preset conan-release
          #cmake --build build/build/Release
          shellHook = ''

          '';
        };
      }
    );
}
