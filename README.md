# STAR-MoorDyn-Coupling
If you use this coupling please consider cite the related open acsess paper at 
LINK:

https://kwnsfk27.r.eu-west-1.awstrack.me/L0/https:%2F%2Fauthors.elsevier.com%2Fsd%2Farticle%2FS0306-2619(24)01585-X/1/0102019176949299-288f55be-ca99-4e3d-9b4b-779bf5c452de-000000/xkC80JuGDe9FhwSkuviq8abpHfE=388 

DOI:

https://doi.org/10.1016/j.apenergy.2024.124202

Please refer to Section 2.3 of the paper for details on the coupling. Here, I’ll outline the main aspects:

Before compiling the coupling library, compile MoorDyn first. The included
MoorDyn v1.01.02 source is used by both builds:

- Linux: `Library/MoorDyn-1.01.02/compileSO`
- Windows: `Library/MoorDyn-1.01.02/compileDLL`

The Windows implementation uses MinGW-w64 and STAR-CCM+'s `UserFunctions.lib`.
The Windows-specific source and Makefile are now included below.

## Linux compilation

Use a terminal with `g++` and GNU Make available. First compile MoorDyn:

```text
cd Library/MoorDyn-1.01.02/compileSO
make -B -f makefile
```

Then compile the STAR-CCM+ user library:

```text
cd ../..
make -B -f makefile comp
```

The result is `Library/test.so`. Copy it next to the simulation file and keep
the mooring input at `Mooring/lines.txt` beside the `.sim` file.

## Windows compilation

The Windows build uses a 64-bit MinGW-w64 C++ compiler and GNU Make. The
commands can be run from any terminal: Windows Command Prompt, PowerShell,
Anaconda Prompt, MobaXterm, or an MSYS2/MinGW shell. The terminal does not
matter; it only needs to be able to find `g++` and `make`.

### 1. Check the compiler

Open your preferred terminal and run:

```text
g++ --version
make --version
```

If either command is not found, install the MinGW-w64/MSYS2 toolchain and add
its `mingw64\bin` directory to PATH. The compiler must be 64-bit, matching the
Windows STAR-CCM+ installation. Close and reopen the terminal after changing
PATH.

### 2. Compile MoorDyn

Run the following commands from any terminal. The `cd` syntax depends on the
terminal: use `cd /d "D:\..."` in Command Prompt or Anaconda Prompt, use
`Set-Location "D:\..."` in PowerShell, and use the corresponding mounted
POSIX path in MobaXterm. Once inside the directory, the `make` commands are
the same.

```text
cd /d "D:\path\to\STAR-MoorDyn-Coupling\Library\MoorDyn-1.01.02\compileDLL"
make -B -f makefile
```

This compiles the MoorDyn source files and creates the object files used by the
coupling.

### 3. Set the STAR-CCM+ library path

STAR-CCM+ supplies the user-function import library. It is normally located
inside the STAR installation under `star\lib\win64\...\lib`. It can be found
with:

```text
dir "C:\Program Files\Siemens" /s /b | findstr /i "UserFunctions.lib"
```

Open [`makefile_Windows`](Library/makefile_Windows) in a text editor and set
`STAR_USERFUNCTIONS_LIB` to the complete path, including the filename. For
example:

```text
STAR_USERFUNCTIONS_LIB ?= C:/Program Files/Siemens/21.02.008-R8/STAR-CCM+21.02.008-R8/star/lib/win64/clang20.1vc14.2-r8/lib/UserFunctions.lib
```

Use the library belonging to the same STAR-CCM+ version that will run the
simulation.

### 4. Compile the coupling

From the `Library` directory, run:

```text
cd /d "D:\path\to\STAR-MoorDyn-Coupling\Library"
make -B -f makefile_Windows comp
```

The result is `Library\test.dll`. Copy it next to the simulation file. Keep
the mooring input file in `Mooring\lines.txt` beside the `.sim` file. The Java
macro automatically loads `test.dll` on Windows and `test.so` on Linux.

At startup, the coupling automatically changes the process working directory
to the directory containing the loaded user library before calling
`LinesInit()`. Therefore, no operating-system-specific path needs to be edited
in `uclib.cpp`; place the library and the `Mooring` folder as described above.

## Example startup logs

The `Example` folder contains sample startup logs for both platforms:

- [`windows.log`](Example/windows.log)
- [`linux.log`](Example/linux.log)

During a successful initialization, the log should show the user library being
loaded, MoorDyn creating the mooring system, the dynamic-relaxation step, the
`MoorDynStar LOADED SUCCESSFULLY!` message, the resolved directory containing
`Mooring/lines.txt`, and then force and moment values from the first coupling
iterations.

The logs also contain STAR-CCM+ solver messages and monitor warnings from the
example simulation. These messages are not MoorDyn compilation errors. A scan
found no passwords, license keys, access tokens, or other credentials in the
logs. The logs retain the original runtime paths, hostnames, solver port, and
STAR-CCM+ license-server address because these are diagnostic values rather
than credentials.

### Linux error: `cannot find -lstdc++`

If Linux reports:

```text
/usr/bin/ld: cannot find -lstdc++
```

the compiler is being asked to use the static C++ runtime, but the static
development archive is not installed. The included Linux MoorDyn Makefile does
not force static linking, so rebuild from `compileSO` with:

```text
make -B -f makefile
```

This links the normal shared C++ runtime and avoids requiring `libstdc++.a`.

For the Linux coupling itself, run the `comp` target from the `Library`
directory:

```text
make -f makefile comp
```

The `-DLINUX` definition in this Makefile is required. It prevents MoorDyn's
header from selecting its Windows branch and including `Windows.h`. The
command `make makefile comp` is not necessary: Make interprets `makefile` as a
target, prints `Nothing to be done for 'makefile'`, and then proceeds to build
`comp`.

In this repository, you'll find the files necessary for creating a coupling between Star CCM+ and MoorDyn. In the "Library" folder, there is a makefile: compile the Linux library using `make -f makefile comp` or the Windows library using `make -f makefile_Windows comp`. In the "Coupling" folder, there are "uclib.h" and "uclib.cpp". You do not need to modify the "uclib.h" file. Additionally, in the "uclib.cpp," there is the coupling. It works by calling MoorDyn only at the end of a time step, and for the other iterations, it uses the moments and forces previously calculated.

In the "uclib.cpp," you HAVE TO modify these parameters: 

- SCALE: 1 if the simulation is for a complete body, 2 if there is symmetry for the plane y=0. 
- II: The value of Inner Iteration in STAR (I need to check whether the number should match the inner iteration or the iteration of the DFBI solver. I haven’t thought about this before, as I usually use the same number for both.). You need to match the two numbers; otherwise, the coupling will be mathematically incorrect. 
- Xcg, Ycg, Zcg: the coordinates of the CoG. 
- Phi, Theta, Psi: the rotation with respect to the body's rest position. ONLY USE IT FOR FREE DECAY!!!

Additionally, in the "Example" folder, there's an example of a Point Absorber with its mooring system and a precompiled library. There is also a Java Macro which automatically sets up the coupling. The precompiled library is only an example and may not work on your machine; compile the appropriate `test.so` or `test.dll` locally.

In the "Mooring" folder, there is the file "lines.txt," which is the mooring for the PointAbsorber. PLEASE REMEMBER THAT THE CONNECTION TO THE VESSEL IS IN THE CoG COORDINATE SYSTEM.

The "lines.txt" file must be placed in a folder named "Mooring," located in the same directory as the simulation file. Otherwise, you might encounter an error like this: Error: dtC is less than dtM. (0.02 < 0) Error: SIGSEGV: memory access exception. (Thanks to luop21 for pointing that out!)


