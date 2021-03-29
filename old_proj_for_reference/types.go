package main

import (
	"go-sdl2/sdl"
	"mathlib"
)

type vec3 = [3]float64
type vec4 = [4]float64

type tri struct {
	vert    [3][3]float64
	shade   uint8
	color   sdl.Color
	visible bool
}

type object struct {
	mesh   []tri
	update func(*object)
	draw   func(*sdl.Renderer, *object)
	id     int
}

// construct new objects with this
func makeObject() object {
	objCounter++
	return object{id: objCounter}
}

func (o *object) move(offset vec3) {
	for i := range o.mesh {
		for j := range o.mesh[i].vert {
			o.mesh[i].vert[j] = mathlib.AddVec3(o.mesh[i].vert[j], offset)
		}
	}
}

func (o *object) scale(offset vec3) {
	for i := range o.mesh {
		for j := range o.mesh[i].vert {
			o.mesh[i].vert[j] = mathlib.MultVec3(o.mesh[i].vert[j], offset)
		}
	}
}

func (o *object) makeCopy() object {
	newObj := *o // make a value copy of the object
	objCounter++ // increment global object counter
	newObj.id = objCounter
	// make deep copy of mesh
	newObj.mesh = make([]tri, len(o.mesh))
	copy(newObj.mesh, o.mesh)
	return newObj
}

func (o *object) midpoint() (midpt vec3) {
	numVert := 0
	for i := range o.mesh {
		for j := range o.mesh[i].vert {
			midpt = mathlib.AddVec3(midpt, o.mesh[i].vert[j])
			numVert++
		}
	}
	midpt[0] = midpt[0] / float64(numVert)
	midpt[1] = midpt[1] / float64(numVert)
	midpt[2] = midpt[2] / float64(numVert)
	return
}
