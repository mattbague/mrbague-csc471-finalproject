Course:     CSC 471
Instructor: Dr. Zoe Wood
Program:    Final Project
Author:     Matt Bague

=======================
* Compiling & Running *
=======================
1. Type "make" to create the executable called "a.out"
2. Run "a.out" to run my final project
3. Controls
    'w':	      Zoom in
    's':	      Zoom out
    'a':       Rotate left
    'd':       Rotate right
    'r':       Move up
    'f':       Move down
    '1':       Default scene
    '2':       Fog scene
    '3':       Rain scene
    '4':       Snow scene
    'z':       Toggles lighthouse's rotating light beam
    'p':       Toggles rain/snow particle system
    'v':       Make lighthouse light the default yellowish color
    'b':       Make lighthouse light blue
    'n':       Make lighthouse light red
    'm':       Make lighthouse light green
    'q':       To quit.

==============
* What Works *
==============


====================
* Design Decisions *
====================
I used the trackball code provided by Dr. Wood since my previous code
wasn't 100% correct. I did not make any changes to it and simply copied and
pasted it into the Assignment 3 base code (and added a few missing variables).

When loading in the mesh, I calculate the normals once based on the initial
values provided by the mesh file which is then saved to the VBO. To deal with
the normals being transformed, I use the VBO normals and apply the ModelMatrix
to them in the shader each time rather than store them in some data structure 
and update them. I assumed this would be faster on the GPU than CPU.

For distance attenuation, I decided to use 1/d^2 for the attenuation/fall off value.
I got this number from reading the article and comments on this site:
http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/

For viewer position, I assumed the viewer just stayed along the z-axis and
zoomed in or out of the object. Like a microscope almost and a slide where the
viewer is the microscope (doesn't move except up and down) and the bunny is
the slide, which moves in multiple directions.

For light position, I put it between the viewer and the object, to the right, and up.
It seemed to match the demo image fairly close. Also, I did not shade the light cube
since I just assumed it was a constant source of light that generated the same amount
in all directions regardless.

For managing multipe shaders, I decided to create a vertex and fragment shader file
for each type of shading and use keyboard commands to load the different shaders in.

For materials, I made one more shiny (identical parameters to the one in the project specs) 
and one matte in order to show both types.

