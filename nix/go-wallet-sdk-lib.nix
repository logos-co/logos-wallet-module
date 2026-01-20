# Builds the Go WalletSDK library using buildGoModule
# This properly handles Go dependencies with vendoring
{ pkgs, goWalletSdk }:

pkgs.buildGoModule {
  pname = "go-wallet-sdk-lib";
  version = "unstable";
  
  src = goWalletSdk;
  
  # Set this to lib.fakeHash first, run the build, 
  # then replace with the hash Nix gives you
  # vendorHash = pkgs.lib.fakeHash;
  vendorHash = "sha256-sfJ1QW4J7b/K0XU5mkMzdFcJLAKeuZdPD7Tq+KfWw7g=";
  
  # Build static C library from Go source
  buildPhase = ''
    runHook preBuild
    
    export CGO_ENABLED=1
    export GOOS=${if pkgs.stdenv.isDarwin then "darwin" else "linux"}
    
    make static-library
    
    runHook postBuild
  '';
  
  installPhase = ''
    runHook preInstall
    mkdir -p $out/lib
    cp build/libgowalletsdk.* $out/lib/
    runHook postInstall
  '';
  
  doCheck = false;

  meta = with pkgs.lib; {
    description = "Go Wallet SDK C static library";
    platforms = platforms.unix;
  };
}
