# mio

mio is a collection of code that I have added to over several years while working on scientific projects.

altro
  A set of header files that provide functionality for error handling, random number gerneation, file handling, etc.
  error.h is used extensively throughout my software. Next in line is types.h which contains macros for generating 1 to 4 value classes with built in math and sorting operations.
  
CMakeModules
  Various cmake find modules for locating libraries and headers
  
ipc
  shared memory and semaphore helper classes
  
lcm
  various LCM files for standard types and opencv mat
  
math
  headers and source files for various math operations. geometric operations, spline fitting, ransac, discrete integration, and more.
  
qt
  This is where I put my custom widgets such as the advanced slider widget. This is both integer and double sliders that have their own spinboxes for setting the current, min, and max values for the slider. Curently, to my knowledge, I have not seen another double slider implemented.
  
serialIO
  Simple class for performing serial communications (eg. RS-232)
  
socket
  TCP and UDP socket helper classes
