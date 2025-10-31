# Builds the Go wallet library using buildGoModule
# This properly handles Go dependencies with vendoring
{ pkgs, goWalletSdk }:

pkgs.buildGoModule {
  pname = "go-wallet-sdk-lib";
  version = "0.0.1";
  
  src = goWalletSdk;
  
  # This hash needs to be updated when dependencies change
  # Set to an empty string or lib.fakeSha256 first, then update with the hash from the error message
  vendorHash = null;  # Try building with null first to see if vendor directory exists
  
  # Build flags for C shared library
  buildPhase = ''
    runHook preBuild
    
    export CGO_ENABLED=1
    export GOOS=${if pkgs.stdenv.isDarwin then "darwin" else "linux"}
    
    mkdir -p $out/lib
    
    # Build the C shared library
    go build -buildmode=c-shared -o $out/lib/libgowalletsdk${if pkgs.stdenv.isDarwin then ".dylib" else ".so"} ./cshared
    
    runHook postBuild
  '';
  
  # Skip default install phase since we handle it in buildPhase
  installPhase = ''
    # Already installed in buildPhase
    echo "Library installed to $out/lib"
    ls -la $out/lib/
  '';
  
  meta = with pkgs.lib; {
    description = "Go Wallet SDK C shared library";
    platforms = platforms.unix;
  };
}

