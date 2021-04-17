models being loaded from disk more than once
	** change load_model_from_desk to load a mesh instead of an object 

finish game-ready camera movement

Add player model and camera follows player
Cube positions should be part of level
Add floor tiles to test level

consider vsync instead of pausing

finish opengl tutorial

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
