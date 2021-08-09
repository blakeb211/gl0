VISIT THE SCREENSHOTS FOLDER TO SEE THE ENGINE IN ACTION

/****************************************
*		GOALS
	4 levels
	generated levels
	death conditions: 	y = -20, health = 0, 
	win condition:		touch the final fruit
	audio
	animation
*****************************************/
FILETYPES:
	.txt	level file
	.cmp	campath file
	.ppm	ppm file with initial cps points
*****************************************/

camera on a rail system - done

game
	read a level and its campath file - done
	camera follows campath - done
	camera should follow campath smoothly - done


campath editor
	get campath control pts from a ppm - done
	write cam path files - done
	show coords on hover
	move control points
	delete control points
	add control points
	
level generator
	generate a level

need collision detection

need gravity

entity states
add object properties, like state timings to level text
terrain creation : generate levels and campaths

need level editor


entities inherit from entity 

camera on a rail --> camera follows a bezier through the level

main loop should run based on level text data
  object state machine based properties
  checking win state of level


finish opengl lighting section 
need lighting
animation


add menu screen

pick style guide and stick to it

lay out a simple level

make level generator

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
