{
  description = "Logos Wallet Module - A wallet plugin for Logos";

  inputs = {
    # Follow the same nixpkgs as logos-liblogos to ensure compatibility
    nixpkgs.follows = "logos-liblogos/nixpkgs";
    logos-cpp-sdk.url = "github:logos-co/logos-cpp-sdk";
    logos-liblogos.url = "github:logos-co/logos-liblogos";
    go-wallet-sdk = {
      url = "github:status-im/go-wallet-sdk/de483fec457ebec76d4f6ad09f1104f0839ce47d";
      flake = false;
    };
  };

  outputs = { self, nixpkgs, logos-cpp-sdk, logos-liblogos, go-wallet-sdk }:
    let
      systems = [ "aarch64-darwin" "x86_64-darwin" "aarch64-linux" "x86_64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f {
        pkgs = import nixpkgs { inherit system; };
        logosSdk = logos-cpp-sdk.packages.${system}.default;
        logosLiblogos = logos-liblogos.packages.${system}.default;
        goWalletSdk = go-wallet-sdk;
      });
    in
    {
      packages = forAllSystems ({ pkgs, logosSdk, logosLiblogos, goWalletSdk }: 
        let
          # Common configuration
          common = import ./nix/default.nix { 
            inherit pkgs logosSdk logosLiblogos; 
          };
          src = ./.;
          
          # Library package
          lib = import ./nix/lib.nix { 
            inherit pkgs common src goWalletSdk logosSdk; 
          };

          # Include package (generated headers from plugin)
          include = import ./nix/include.nix {
            inherit pkgs common src lib logosSdk;
          };

          # Combined package
          combined = pkgs.symlinkJoin {
            name = "logos-wallet-module";
            paths = [ lib include ];
          };
        in
        {
          # Individual outputs
          logos-wallet-module-lib = lib;
          logos-wallet-module-include = include;
          
          # Default package (combined)
          default = combined;
        }
      );

      devShells = forAllSystems ({ pkgs, logosSdk, logosLiblogos, goWalletSdk }: {
        default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.cmake
            pkgs.ninja
            pkgs.pkg-config
            pkgs.go
            pkgs.gnumake
          ];
          buildInputs = [
            pkgs.qt6.qtbase
            pkgs.qt6.qtremoteobjects
          ];
          
          shellHook = ''
            export LOGOS_CPP_SDK_ROOT="${logosSdk}"
            export LOGOS_LIBLOGOS_ROOT="${logosLiblogos}"
            echo "Logos Wallet Module development environment"
            echo "LOGOS_CPP_SDK_ROOT: $LOGOS_CPP_SDK_ROOT"
            echo "LOGOS_LIBLOGOS_ROOT: $LOGOS_LIBLOGOS_ROOT"
          '';
        };
      });
    };
}

