# Third-party notices

M0 permits only the dependencies below. CMake verifies each release archive by
SHA-256 before extraction.

| Dependency | Version | Licence | SHA-256 |
| --- | --- | --- | --- |
| GLFW | 3.4 | zlib/libpng | `c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01` |
| GLM | 1.0.3 | MIT | `6775e47231a446fd086d660ecc18bcd076531cfedd912fbd66e576b118607001` |
| Dear ImGui | 1.92.8 | MIT | `fecb33d33930e12ff53a34064e9d3a06c8f7c3e04408f14cd36c80e3faac863b` |
| Catch2 | 3.15.0 | BSL-1.0 | `9650c55e497759cc39b977e45524bc8acb15256061c112080916ab6cb0b1ea66` |
| volk | 1.4.350 | MIT | `e47c6efe5294bb03729a976b385864bba5daf46ec60f0bdea11e1d1446345f9a` |
| Vulkan-Headers | 1.4.350 | Apache-2.0 | `6dd105e5cc7ddab6e7b611ae2c1872740d1727557cc8bf9daf13d6de1e4b3999` |

The engine targets Vulkan 1.3. Newer headers do not authorize use of Vulkan 1.4
features. Full licence texts are included in each dependency's fetched source
archive and build tree.
