# Source repository contents

This public source tree contains the KOF 2003 project glue, decryption tool,
tests, packaging scripts and documentation.

It intentionally does **not** contain:

- game ROMs or Neo Geo BIOS files;
- `CMakeLists.txt` or other CMake build files;
- build directories, generated recompiled C, screenshots or distribution
  binaries;
- the upstream `neogeorecomp/` toolkit source.

The MIT-licensed `neogeorecomp` toolkit is a separate dependency.  Obtain it
from <https://github.com/sp00nznet/neogeorecomp> (v0.2.0, commit
`af96b2b4b39b42b392b17bcaa6a74a5f27088202` or later) and place it at
`neogeorecomp/` inside this directory when rebuilding.  The project source
needs the additional generic NEO-PVC, fixed-layer, audio and SDL changes
described in `docs/BUILDING.md` and `THIRD_PARTY_NOTICES.md`.

Ready-to-run Windows and Steam Deck builds, with the required third-party
licence texts, are attached to the GitHub release for this repository.  They
never include ROMs or BIOS data.
