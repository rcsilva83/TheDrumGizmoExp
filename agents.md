# Agent Notes

This file mirrors the high-level guidance in `AGENTS.md` and exists so
automation that expects lowercase filenames can find the same project context.

Key update: CMake is now supported alongside Autotools.

- Preferred build: `cmake -S . -B build && cmake --build build -j$(nproc)`
- Install: `cmake --install build --prefix "$PWD/install"`
- Legacy flow remains available via `./autogen.sh && ./configure && make`
