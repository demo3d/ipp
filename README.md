IPP - Interactive Presentation Platform
=======================================

Rendering library designed for content/animation presentation, integrated with Blender for content
creation and asset processing (resource pipeline built on top of blender).

Some ideas I would like to try out with this library
----------------------------------------------------

### Apply [Redux](https://github.com/reactjs/redux)-like architecture to a rendering engine

  * state is grouped in to a "World" object which is a collection of entities + an event loop ~ redux store
  * state is isolated inside of POD components, components must be trivially serializable and cannot
    hold pointers - references to other entities are held by ID
  * systems update state in response to messages they receive trough the event loop ~ redux reducers with defined order of execution
  * systems updates should be "pure" - same messages + state should result in the same state after a system update
  * communication by messages passing makes implementing a client API a matter of serializing a simple data structure and dispatching it to the event loop ~ redux dispatch
    things like IPC become transparent and interactive debugging/state logging becomes simpler
    API separation in to isolated systems should simplify testing (notoriously bad in graphcis/gamedev)

### Automatically generated binding APIs from C++ source

Using libclang + python [it should be possible](http://szelei.me/code-generator/) to create a
templated binding generator (based on one of the many Python templating libraries like say Jinja).

The generator will analyze C++ code based on conventions and metadata and generate a target
language API that will communicate with C++ trough C FFI and message passing.

Providing raw state (entity/component data and system state) access should also be trivial by
automatically generating C access code on the C++ side and client code to consume it.

### Leverage this design to create a transparent IPC system that will allow real time sync with Blender

Redux pattern + serializable POD messages = transparent IPC
API generator = easy to create up-to-date python API

Those two features will allow tight integration with Blender, for eg. Blender editor can simply
stream state updates over IPC channel to an active IPP player application and the user has
real-time content preview - superior to rolling your own content creation tools or doing the
old export-reload cycle :D

### Optimize for development/productivity over performance

By creating a flexible and transparent architecture based on functional programming patterns
things like real-time Blender state integration should be possible which should speed up content
creation significantly.

Having a well documented and conceptually simple core design should make extending the library
and consuming services much simpler than APIs/systems optimized for performance.

This is not an attempt at creating the next AAA game engine :)

### Generate tiny binary code that runs everywhere

Modern game engines used for this purpose are huge libraries with features that aren't usefully
in scenarios like "display this simple 3D scene" or "render this 3D model" - they generate >10MB
of JS code - even with [WebAssemly](http://webassembly.github.io/) the binaries will be in
megabytes and take 10s of seconds to compile on mobile.

A goal of this library is to keep "advanced features" to a minimum and be as lean as possible,
after async loading system is implemented and C++ IO dependency is removed compiled JS size should
be <1MB uncompressed which should allow fast download and loading on mobile.

Long term aim is to create a library that you can just "plug-in" to any platform where you need
simple 3D stuff rendered (simple as in - 100s of objects scenes, no AAA special effects, etc.).
Similar pure JS solutions exist but can only run in the browser/JS environments and are nasty to
work with (even compared to C++ for this use case) or integrate with other languages

Current status
--------------

Stuff that's implemented :

  * event loop, entity/component system  
  * resource manager/scene loader
  * general purpose animation system
  * user controlled camera system
  * C++11 OpenGL ES 2 wrapper + basic render system on top of it
  * blender scene exporter that supports :
    * meshes with UV/vertex colors/skinning/custom vertex groups & weights
    * armatures
    * linked file export and shared source resource loading
    * keyframe animations for position/rotation/scale on all object, "baking" skeletal animation

Barebones functionality - enough to get a non-trivial blender scene on the screen,
but it's still very from being usable by a 3rd party, some of the crucial missing features :

  * render pipeline doesn't exist - it's just a simple loop for a single directional light -
    enough to get stuff rendering - not enough for anything serious, **WIP**
  * command/event system in the event loop is built in an ad-hoc way - not enough to support
    advanced use cases and writing bindings to the API is tedious and error prone,
    using libclang and analyzing sources based on conventions + some way (?) to hook in metadata
    it should be possible to analyze C++ source and generate bindings for multiple languages.
    Once this is done IPP will (hopefully) be a generally useful library
  * command/event system can be used to expose the entire state of the application - this will
    make features like async resource loading or state synchronization with content editor (blender)
    in real time straightforward to implement (since commands/events are serializable PODs they
    can be transparently marshalled trough IPC for eg.)
  * resource pipeline was also built in a semi ad-hoc way and isn't well documented
    processing/conversion features should be separated in to a library and the build pipeline should
    consume it - this is already done partially but it needs to be completed and documented  
  * The library compiles and runs on clang/emscripten, other compilers are WIP (code compiles
    but doesn't run correctly) and library should work on windows msvc along with linux/emscripten

After the infrastructure stuff is done "real" features such as 3D picking, parallel animation
playback, shadows, VR integration etc. will be added.


License
-------

Everything in this folder is copyrighted by Rafael Munitić and licensed under GPL v3 or later.
When I get the library to a "sort of useful" state I will probably relicense to a permissive license.
