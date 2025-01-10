# OpenGL Project: Deux Salles, Deux Ambiances

# compilation

you should use VS with a cmake extension
use gcc and choose the build type: Debug
run *Cmake: Build*
you can now run


# if no VSC, this should work
mkdir build
cd build
cmake ../
cd ..
mkdir bin
make -j4


# Shader:

## Room 1:

- Diffuse maps
- Roughness maps
- Normal maps

- Diffuse lighting
- Specular lighting

- small RGB corner effect

## Room 2:

- voronoi shader based on the distance of 200 lights
- transparance
- mirror


# Implementation

## Software Specifications

- the structure of the code could be better, aroud the all the hpp files (not using cpp files)

## Géometric Specifications

- The room geometry is respected
- There is a skybox

## Modelisation Specification

- There is a collumn everywhere around the map at each 1 meter increment
- Bellow each art exposed in the gallery is a marble footer that allow the art to be higher in the room
- In the room 2, there are a few bench that allow visiors to rest
- there is a plane that move above the room that you can time its stop with scroolwheel up

## Graphic Specifications

- Textures in room 1
- Fluid passage between the 2 rooms
- Mirror in the room 2

## IHM Specifications

- The player have collisions with all the walls

# Bonus

- the player can jump
- the player can crouch (slow the speed too)
- the player can run forward faster by pressig left shift (which change the fov slighly)
- the sun is aligned with the skybox

# Stuff made

- The marble collumns / foot are made in blender with their respectives textures
- the text is made in blender too
- all roughness and normal texture have been genrated by me


# Used 3D models

[boeing 787-8 3D model](https://free3d.com/3d-model/boeing-787-8-thomson-54662.html)

[Statue v1 3D model](https://free3d.com/3d-model/statue-v1--541832.html)

[Elite Dangerous, Farragut Battle Cruiser](https://www.thingiverse.com/thing:3576615)

[Abandoned Cottage House 3D Model](https://free3d.com/3d-model/abandoned-cottage-house-825251.html)

# Keybinds

On a AZERTY keyboard:

[A] quit the program

[ZSQD] move the player

[E] toggle the sun light

[R] switch view to the mirror view

[X] toggle wireframe mode

<!-- [Arrows] rotate the camera -->

[RMB] zoom in

[LMB] interact

[LShift] walk faster

[LCtrl] crouch

[Space] jump

[ScrollDown] switch the shader used for the right room

[ScrollUp] stop the plane