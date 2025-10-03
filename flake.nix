{
  description = "Build the Logos wallet module plugin with Nix";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  inputs.logos-liblogos = {
    url = "github:logos-co/logos-liblogos/49e9dfaaf793d3a60124abf3d9f830e5c63a0c3e";
    flake = false;
  };
  inputs.logos-cpp-sdk = {
    url = "github:logos-co/logos-cpp-sdk/65aa4a1b24692802e618b3ef0d7c4e320f5f0d99";
    flake = false;
  };
  inputs.go-wallet-sdk = {
    url = "github:status-im/go-wallet-sdk/de483fec457ebec76d4f6ad09f1104f0839ce47d";
    flake = false;
  };

  outputs = { self, nixpkgs, logos-liblogos, logos-cpp-sdk, go-wallet-sdk }:
    let
      lib = nixpkgs.lib;
      systems = [
        "aarch64-darwin"
        "x86_64-darwin"
        "aarch64-linux"
        "x86_64-linux"
      ];
      forSystems = f: lib.genAttrs systems (system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
          f pkgs
      );
    in
    {
      packages = forSystems (pkgs:
        let
          drv = pkgs.callPackage ./nix/package.nix {
            logosLiblogosSrc = logos-liblogos;
            logosCppSdkSrc = logos-cpp-sdk;
            goWalletSdkSrc = go-wallet-sdk;
          };
        in
        {
          default = drv;
          wallet-module = drv;
        }
      );

      devShells = forSystems (pkgs: {
        default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.ninja
            pkgs.pkg-config
            pkgs.go
          ];
          buildInputs = [
            pkgs.qt6.qtbase
            pkgs.qt6.qtremoteobjects
          ];
        };
      });

      checks = forSystems (pkgs: {
        wallet-module = pkgs.callPackage ./nix/package.nix {
          logosLiblogosSrc = logos-liblogos;
          logosCppSdkSrc = logos-cpp-sdk;
          goWalletSdkSrc = go-wallet-sdk;
        };
      });
    };
}

