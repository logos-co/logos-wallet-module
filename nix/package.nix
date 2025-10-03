{ lib
, stdenv
, cmake
, ninja
, pkg-config
, qt6
, go
, gnumake
, logosLiblogosSrc
, logosCppSdkSrc
, goWalletSdkSrc
}:

let
  qtDeps = [
    qt6.qtbase
    qt6.qtremoteobjects
  ];
  qtPrefixPath = lib.concatStringsSep ";" (map (pkg: "${pkg}") qtDeps);
in
stdenv.mkDerivation {
  pname = "logos-wallet-module";
  version = "0.1.0";

  src = lib.cleanSourceWith {
    src = ../.;
    filter = path: type:
      let
        base = toString ../.;
        pathStr = toString path;
        relPath = lib.removePrefix (base + "/") pathStr;
        # Don't include vendor directories since we'll use built packages
        # But keep go-wallet-sdk vendor directory
        excludeVendor = !(lib.hasPrefix "vendor/logos-" relPath);
      in
        lib.cleanSourceFilter path type && excludeVendor;
  };

  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
    qt6.wrapQtAppsHook
    go
    gnumake
  ];

  buildInputs = qtDeps;

  cmakeBuildType = "Release";

  dontWrapQtApps = true;

  postUnpack = ''
    echo "Replacing vendor dependencies with flake inputs"
    rm -rf source/vendor/logos-liblogos source/vendor/logos-cpp-sdk source/vendor/go-wallet-sdk
    mkdir -p source/vendor
    cp -r ${logosLiblogosSrc} source/vendor/logos-liblogos
    cp -r ${logosCppSdkSrc} source/vendor/logos-cpp-sdk
    cp -r ${goWalletSdkSrc} source/vendor/go-wallet-sdk
    chmod -R u+w source/vendor/logos-liblogos source/vendor/logos-cpp-sdk source/vendor/go-wallet-sdk
  '';

  preConfigure = ''
    echo "Building wallet library from go-wallet-sdk"
    
    # Set up Go build environment
    export HOME=$TMPDIR
    export GOCACHE=$TMPDIR/go-cache
    export GOPATH=$TMPDIR/go
    export CGO_ENABLED=1
    mkdir -p $GOCACHE $GOPATH
    
    cd vendor/go-wallet-sdk
    echo "Go version: $(go version)"
    make build-c-lib
    cd ../..
    
    mkdir -p lib
    cp vendor/go-wallet-sdk/build/libgowalletsdk.* lib/ 2>/dev/null || true
    echo "Wallet C library copied into lib/"
  '';

  cmakeFlags = [
    "-GNinja"
    "-DLOGOS_WALLET_MODULE_USE_VENDOR=ON"
  ];

  CMAKE_PREFIX_PATH = qtPrefixPath;
  QT_DIR = qtPrefixPath;

  meta = with lib; {
    description = "Wallet Logos module plugin";
    platforms = platforms.unix;
  };
}

