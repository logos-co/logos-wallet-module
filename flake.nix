{
  description = "Logos Wallet Module - A wallet plugin for Logos";

  inputs = {
    logos-module-builder.url = "github:logos-co/logos-module-builder";
    nix-bundle-lgx.url = "github:logos-co/nix-bundle-lgx";
    go-wallet-sdk = {
      url = "github:status-im/go-wallet-sdk/f6c0924394c5bdf361bd16b739a80432e68291f5";
      flake = false;
    };
  };

  outputs = inputs@{ logos-module-builder, ... }:
    logos-module-builder.lib.mkLogosModule {
      src = ./.;
      configFile = ./metadata.json;
      flakeInputs = inputs;
      externalLibInputs = {
        gowalletsdk = inputs.go-wallet-sdk;
      };
      tests = {
        dir = ./tests;
        mockCLibs = ["gowalletsdk"];
      };
    };
}
