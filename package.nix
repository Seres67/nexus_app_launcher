{
  cmake,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "nexus_app_launcher";
  version = "0.5.1.0";
  src = ./.;

  nativeBuildInputs = [
    cmake
  ];

  installPhase = ''
    x86_64-w64-mingw32-strip ./*.dll
    mkdir -p $out/lib
    cp ./*.dll $out/lib
  '';
}
