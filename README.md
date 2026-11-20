# Evolution Engine

Evolution Engine is a game engine written in C++ and Vulkan 1.4 with an emphasis on rendering. The goal with the engine is to facilitate the development of modern racing games with a fully dynamic renderer, which is to say a renderer that does little to no offline/pre-calculations and does everything on the fly instead. 

## Running the project

The project's build system is [xmake](https://xmake.io), which is required to compile it. Besides that the other requirements are a C++20 compiler and a GPU with Vulkan 1.4 support. The rest is handled by the build system, like dependency/library fetching and compilation. To run the project, run the following commands in a terminal at the project root:

```sh
xmake
xmake run evolution_sandbox
```

Keep in mind that first-time xmake builds take extra time due to having to acquire all the dependencies first. 
To switch between release and debug modes, run the following command:
```sh
xmake f -m debug # or "release" instead of "debug"
```

Code has been tested on Linux and Windows.
