{ lib
, stdenv
, cmake
, ninja
, pkg-config
, qt6
, go
, logosLiblogosSrc
, logosCppSdkSrc
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
  ];

  buildInputs = qtDeps;

  cmakeBuildType = "Release";

  dontWrapQtApps = true;

  postUnpack = ''
    echo "Replacing vendor dependencies with flake inputs"
    rm -rf source/vendor/logos-liblogos source/vendor/logos-cpp-sdk
    mkdir -p source/vendor
    cp -r ${logosLiblogosSrc} source/vendor/logos-liblogos
    cp -r ${logosCppSdkSrc} source/vendor/logos-cpp-sdk
    chmod -R u+w source/vendor/logos-liblogos source/vendor/logos-cpp-sdk
  '';

  preConfigure = ''
    echo "Building wallet library from go-wallet-sdk"
    bash ./build_wallet_lib.sh
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

