VISIT THE SCREENSHOTS FOLDER TO SEE THE ENGINE IN ACTION

/****************************************
*		GOALS
	4 levels
	generated levels
	death conditions: 	y = -20, health = 0, 
	win condition:		touch the final fruit
	add menu screen
	audio
	animation

*****************************************/
FILETYPES:
	.txt	level file
	.cmp	campath file
	.ppm	ppm file with initial cps points
*****************************************/

game
	collision detection and response
	-------------------------------------------------
	camera on a rail system - done
	read a level and its campath file - done
	camera follows campath - done
	camera should follow campath smoothly - done

refactoring
	move header and cpps into same directory
	pull update outside of main.cpp
	separate octree into header and cpp file
	compile lib fmt and glm into a dll and add it to the project
	need a tool to enforce style guide
	-------------------------------------------------
	Fix names: all types and functions InThisCase and all variables in_this_case - done
	pull rendering system outside of main.cpp - done

campath editor
	show coords on hover
	move control points
	delete control points
	add control points
	-------------------------------------------------
	get campath control pts from a ppm - done
	write cam path files - done
	
collision detection
	TODO	
	+let object be in more than one cell at once
	+write method to print the nearest neighbors
	+add bounding box to each object, consider spherical
	+find nearest neighbors, then intersecting bounding boxes, then fine-grain triangle collision
	-------------------------------------------------
	+only draw cells with list.size() > 0  - done
	+use center for now - done
	+verify postion to grid coord is working - done
	+draw object positions to make sure its center of object - done
	+update grid node lists each frame - done

	NOTES
	+keep separate list of stationary and moving objects
	+use distance squared instead of actual distance
	+do a coarse check before doing a granular check
	+does collision checking need to be done every frame?


level generator
	terrain creation : generate levels and campaths

need gravity
entity states

main loop should run based on level text data - done
  object state machine based properties - done

finish opengl lighting section 


RESOURCED NEEDED

game modes
