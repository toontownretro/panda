# panda

This is the `panda` tree of the Player, originally derived from the `panda` directory of the main [Panda3D](https://github.com/panda3d/panda3d) repository.

Panda is the main engine component of the Player, containing the code for the scene graph, rendering, audio, networking, etc.

## How to build
Assumes you have already set up the development environment and have built and installed the trees required by Panda.
#### Windows
```
cta panda
cd %PANDA%
ppremake
nmake/jom install OR msbuild panda.sln -t:install
```
#### Unix
```
cta panda
cd $PANDA
ppremake
make install
```
See the [Wiki](https://github.com/toontownretro/documentation/wiki) for instructions on setting up the development environment and the entire project as a whole.

## Differences between main Panda3D
- A rewritten [animation system](https://github.com/toontownretro/panda/tree/master/src/anim) that is more flexible and performant, including support for blend trees, animation events, and minimal inverse kinematics.
- The work-in-progress `shaderpipeline` branch of main Panda3D is merged in, adding support for precompiled SPIR-V shaders.
- A flexible [shader generator system](https://github.com/toontownretro/panda/tree/master/src/shader), utilizing precompiled SPIR-V shader variations.
- A dedicated [level system](https://github.com/toontownretro/panda/tree/master/src/map), along with an offline [compiler](https://github.com/toontownretro/panda/tree/master/src/mapbuilder), including a visibility preprocessor and a GPU-accelerated lighting preprocessor.
- A [post-processing pipeline system](https://github.com/toontownretro/panda/tree/master/src/postprocess).
- A [physics implementation](https://github.com/toontownretro/panda/tree/master/src/pphysics) using NVIDIA PhysX 4.
- Support for more modern OpenGL features that improve performance.
- Various performance optimizations.
- Improved multithreading support.

