camera on a rail system
add campath to level txt
camera should find closest path point to players new position
and smoothly transition there, while looking at a certain direction

generate level using some method

need collision detection
need gravity

entity types need states

need lighting

need level editor

finish opengl lighting section 

entities inherit from entity 

camera on a rail --> camera follows a bezier through the level

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
