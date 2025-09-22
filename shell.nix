{
  mkShell,
  cmake,
  clang-tools,
  bintools,
}:
mkShell {
  nativeBuildInputs = [
    cmake
    clang-tools
    bintools
  ];
}
