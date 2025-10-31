{
  mkShell,
  cmake,
  clang-tools,
  bintools,
  xxd,
}:
mkShell {
  nativeBuildInputs = [
    cmake
    clang-tools
    bintools
    xxd
  ];
}
