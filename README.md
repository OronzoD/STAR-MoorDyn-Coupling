# STAR-MoorDyn-Coupling
If you use this coupling please consider cite the related open acsess paper at 
LINK:

https://kwnsfk27.r.eu-west-1.awstrack.me/L0/https:%2F%2Fauthors.elsevier.com%2Fsd%2Farticle%2FS0306-2619(24)01585-X/1/0102019176949299-288f55be-ca99-4e3d-9b4b-779bf5c452de-000000/xkC80JuGDe9FhwSkuviq8abpHfE=388 

DOI:

https://doi.org/10.1016/j.apenergy.2024.124202

Please refer to Section 2.3 of the paper for details on the coupling. Here, I’ll outline the main aspects:

BEFORE compiling the library, you need to compile MoorDyn. The make file looks for the Moordyn files located in "MoorDyn-master/compileSO" for Linux, "MoorDyn-master/compileDLL" for Windows.

The Windows implementation isn’t currently working. I’ll try to fix it over the next couple of days—hopefully!

In this repository, you'll find the files necessary for creating a coupling between Star CCM+ and MoorDyn. In the "Library" folder, there is a makefile: you can compile the library using the command "make makefile comp". In the "Coupling" folder, there are "uclib.h" and "uclib.cpp". You do not need to modify the "uclib.h" file. Additionally, in the "uclib.cpp," there is the coupling. It works by calling MoorDyn only at the end of a time step, and for the other iterations, it uses the moments and forces previously calculated. 

In the "uclib.cpp," you HAVE TO modify these parameters: 

- SCALE: 1 if the simulation is for a complete body, 2 if there is symmetry for the plane y=0. 
- II: The value of Inner Iteration in STAR (I need to check whether the number should match the inner iteration or the iteration of the DFBI solver. I haven’t thought about this before, as I usually use the same number for both.). You need to match the two numbers; otherwise, the coupling will be mathematically incorrect. 
- Xcg, Ycg, Zcg: the coordinates of the CoG. 
- Phi, Theta, Psi: the rotation with respect to the body's rest position. ONLY USE IT FOR FREE DECAY!!!

Additionally, in the "Example" folder, there's an example of a Point Absorber with its mooring system and a precompiled library. There is also a Java Macro which automatically sets up the coupling. The file "test.so" is only to show how the macro works; probably, the precompiled library will not work (you need to compile it on your machine).

In the "Mooring" folder, there is the file "lines.txt," which is the mooring for the PointAbsorber. PLEASE REMEMBER THAT THE CONNECTION TO THE VESSEL IS IN THE CoG COORDINATE SYSTEM.

The "lines.txt" file must be placed in a folder named "Mooring," located in the same directory as the simulation file. Otherwise, you might encounter an error like this: Error: dtC is less than dtM. (0.02 < 0) Error: SIGSEGV: memory access exception. (Thanks to luop21 for pointing that out!)


