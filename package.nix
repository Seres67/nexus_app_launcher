{
  cmake,
  xxd,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "nexus_app_launcher";
  version = "0.5.1.0";
  src = ./.;

  nativeBuildInputs = [
    cmake
    xxd
  ];

  installPhase = ''
    mkdir -p $out/lib
    cp ./*.dll $out/lib
    md5sum $out/lib/libnexus_app_launcher.dll | awk '{print $1}' | xxd -r -p > $out/libnexus_app_launcher.dll.md5
  '';
}
