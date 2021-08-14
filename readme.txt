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

refactoring
	pull rendering system outside of main.cpp - done
	pull update outside of main.cpp
	need a tool to enforce style guide

campath editor
	get campath control pts from a ppm - done
	write cam path files - done
	show coords on hover
	move control points
	delete control points
	add control points
	
collision detection
	+keep separate list of stationary and moving objects
	+use distance squared instead of actual distance
	+do a coarse check before doing a granular check
	+does collision checking need to be done every frame?
	UNIFORM GRID SPATIAL PARTITIONING
	+update grid node lists each frame
	+use center for now
	+only draw cells with list.size() > 0  // every 10th frame
	+print nearest neighbors
	+add bounding box to each object

level generator
	terrain creation : generate levels and campaths

need gravity
entity states

need level editor

main loop should run based on level text data
  object state machine based properties
  checking win state of level

finish opengl lighting section 
need lighting
animation

add menu screen

RESOURCED NEEDED
  meshes
  animation
  textures
  sounds

object

game modes
