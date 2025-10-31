# Builds the Go wallet library using buildGoModule
# This properly handles Go dependencies with vendoring
{ pkgs, goWalletSdk }:

pkgs.buildGoModule {
  pname = "go-wallet-sdk-lib";
  version = "0.0.1";
  
  src = goWalletSdk;
  
  # Hash of the Go dependencies
  vendorHash = "sha256-4aiUWRiXRUgLntKvOPCuR8fJpzS+OKKh5oe5B7V74j4=";
  
  # Ignore the existing vendor directory since it's out of sync
  proxyVendor = true;
  
  # Build flags for C shared library
  buildPhase = ''
    runHook preBuild
    
    export CGO_ENABLED=1
    export GOOS=${if pkgs.stdenv.isDarwin then "darwin" else "linux"}
    
    mkdir -p $out/lib
    
    # Build the C shared library using -mod=mod to ignore vendor directory
    go build -mod=mod -buildmode=c-shared -o $out/lib/libgowalletsdk${if pkgs.stdenv.isDarwin then ".dylib" else ".so"} ./cshared
    
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

