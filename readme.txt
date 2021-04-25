replace c-style calls with fmtlib for logging

finish opengl lighting section 

models being loaded from disk more than once 

get camera movement similar to how it'll be in the game

Add player model and camera follows player
Add floor tiles to test level

get objects loading and rendering from level struct

add object properties, like state timings to level text

add lighting

main loop should run based on level text data
  object state machine based properties
  checking win state of level

animation

terrain creation

add menu screen

pick style guide and stick to it

lay out a simple level

make level editor

wall creation

octree for collisions

multithreaded?

RESOURCED NEEDED
  meshes
  animation
  textures
  sounds

object

world
	holds object locations
	informs objects about collisions
    objects hold a reference to world but world does not hold a reference to objects	

game modes

create a complete build.bat or cmake file to build this project
