# mio

mio is a collection of code that I have added to over several years while working on scientific projects.

##altro
  A set of header files that provide functionality for error handling, random number gerneation, file handling, etc.
  error.h is used extensively throughout my software. Next in line is types.h which contains macros for generating 1 to 4 value classes with built in math and sorting operations.
  An interesting one is freqBuffer.h. The user provides a callback function and continually pushes values onto it's internal queue. It then calls the call back function and feeds it a value from the queue at a user specified frequency. It runs on its own thread that sleeps when there are no values in the queue. I use it most commonly with sliders that are calling a "heavy" function which does not need all the values from the slider.
  
##CMakeModules
  Various cmake find modules for locating libraries and headers
  
##ipc
  shared memory and semaphore helper classes
  
##lcm
  various LCM files for standard types and opencv mat
  
##math
  headers and source files for various math operations. geometric operations, spline fitting, ransac, discrete integration, and more.
  
##qt
  This is where I put my custom widgets such as the advanced slider widget. This is both integer and double sliders that have their own spinboxes for setting the current, min, and max values for the slider. Curently, to my knowledge, I have not seen another double slider implemented.
  
##serialIO
  Simple class for performing serial communications (eg. RS-232)
  
##socket
  TCP and UDP socket helper classes
