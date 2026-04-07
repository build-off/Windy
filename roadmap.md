# Roadmap
## Core
is the low-level reusable foundation for all the other parts of the 
engine, the main idea is also that it does not have almost no dependencies
on the rest of the engine

- [ ] Platform abstraction ( only unix for now )
- [ ] Math lib - most of the utils from scratch, based on glm
- [ ] Containers | Utilities
- [ ] Hashing, UUID
- [ ] Memory management
- [ ] File system - file/path utils
- [ ] Threading
- [ ] Job,task system
- [ ] Time | Clocks | Timers
- [ ] Logging
- [ ] Assertions | Validation
- [ ] Events | Delegates | Signal
- [ ] Serialization primitives
- [ ] Profiler hooks for tracy

## Application layer
program entry, lifecycle and bootstraping

- [ ] Entry point
- [ ] Command line args, parsing
- [ ] Startup - shutdown flow
- [ ] Editor config loading
- [ ] Window creation, display init
- [ ] Modules boot order
- [ ] Layer System
- [ ] Runtime mode selection
- [ ] Main loop, frame loop, fixed update loop
- [ ] Global service registration
- [ ] Crash safe shutdown handling
- [ ] Headless mode

## Engine runtime
engine orchestration, game/editor execution

- [ ] Engine Context
    - [ ] Engine instance
    - [ ] Runtime state

- [ ] ECS World Management (Scenes)
- [ ] Runtime Update Pipeline
- [ ] Runtime Access Coordination
- [ ] World Lifecycle (Scene)
- [ ] Play/Pause state


## Physics System
we are using [Jolt](https://github.com/jrouwe/JoltPhysics?tab=readme-ov-file),
which is used in games like Horizon Forbidden West or death stranding 2, for all
physics related stuff

- [ ] Jolt integration
- [ ] Physics world wrapper
- [ ] Physics debug draw __important__ 
- [ ] Rigidbodies
- [ ] Colliders
- [ ] Queries
- [ ] Collision Events

## Rendering - Graphics System
abstractions for Vulkan

- Vulkan
    - [ ] Devices
    - [ ] Swapchain
    - [ ] Buffers, Command Buffers
    - [ ] Textures
    - [ ] Pipelines
    - [ ] Descriptors and Syncronization

- Rendering core
    - [ ] Shader system
    - [ ] Material system
    - [ ] Render resources, render passes

- Scene renderer
    - [ ] Mesh rendering
    - [ ] Cameras
    - [ ] Lights
    - [ ] Shadows
    - [ ] Post-proccessing 

- Debug renderer
    - [ ] Gysmos
    - [ ] Bounds

## Tools / UI
all of the engine use ImGUI for the UIs

- Tools UI
    - [ ] ImGUI integration

- Editor
    - [ ] Console
    - [ ] Inspector
    - [ ] Scene/World Viewport
    - [ ] Asset browser

    - [ ] Transform gysmos
    - [ ] Undo/redo __not important__

## Serialization
save state of all the important parts of the engine such as components,
configs and scene states

## Particle System
- [ ] Emitters
- [ ] Particle data
- [ ] Rendering integration, debug preview

## Camera System
handle the viewport with the camera

## Audio System
- [ ] Sound playback
- [ ] Listener
- [ ] Audio assets __maybe__
- [ ] Mixer | Buses

## Input System
keyboard/mouse/controller, actions, axis mapping

## Components ECS
- [ ] Transform
- [ ] Camera
