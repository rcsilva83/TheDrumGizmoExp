# DrumGizmo Build Notes

DrumGizmo now supports both CMake and the legacy Autotools build system.

## CMake (preferred)

```sh
cmake -S . -B build
cmake --build build -j$(nproc)
cmake --install build --prefix "$PWD/install"
```

Common options:

- `-DDG_WITH_DEBUG=ON`
- `-DDG_ENABLE_CLI=ON`
- `-DDG_NUM_CHANNELS=16`
- `-DDG_SSE_LEVEL=auto`

## Legacy Autotools

```sh
./autogen.sh
./configure --prefix=$PWD/install --with-test --with-debug --enable-lv2
make -j$(nproc)
make install
```

The historical installation guide is still available in `README`.
