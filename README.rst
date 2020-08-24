|Travis| |AppVeyor|

.. |Travis| image:: https://travis-ci.com/LUGGPublic/CG_Labs.svg?branch=master
   :target: https://travis-ci.com/LUGGPublic/CG_Labs
.. |AppVeyor| image:: https://ci.appveyor.com/api/projects/status/ga1y28ottvmvgif6/branch/master?svg=true
   :target: https://ci.appveyor.com/project/pierremoreau/cg-labs/branch/master

CG_Labs contains the source code for the various Computer Graphics courses
taught at `Lund University`_ (EDAF80_ and EDAN35_). All assignments of EDAF80
are available, as well as the second assignment of EDAN35.

The public repository is found at https://github.com/LUGGPublic/CG_Labs.

Dependencies
============

CG_Labs uses the following dependencies:

* assimp_ (>= 5.0.0): to load OBJ models;
* CMake_ (>= 3.13): to generate project files and handle dependencies;
* `Dear ImGui`_ (1.78): to display information on screen, such as logs;
* Git_: to clone missing dependencies;
* GLAD_ (0.1.33): to load OpenGL extensions;
* GLFW_ (>= 3.2.0): to create a window and an OpenGL context, as well as handle inputs;
* GLM_ (0.9.9.5): for linear algebra operations;
* stb_: to load image files;
* tinyfiledialogs_: to display dialogue windows to the user, in case of errors
  for example.

The project contains the needed files for GLAD and Dear ImGui, and it will
download assimp, GLM, GLFW, stb and tinyfiledialogs if they are not found on
your computer, so you only need to make sure that CMake and Git are installed
on your computer.

An OpenGL 4.1 context is created by the project; if your hardware or its driver
does not support OpenGL 4.1, you should use the `OpenGL 3.3`_ branch instead
which will create a 3.3 context.

C++14 features are used by this project, so you will need a C++14-capable
compiler; if you are using Visual Studio, that means Visual Studio 2015 or
later.

The different resources used in the assignments, textures, object files, can
be found in `this ZIP file`_, which should be automatically retrieved by CMake.

Building
========

Visual Studio 2017
------------------

We will use the following directory layout::

  * some_random_folder // Wherever you want to store the project on your
  |                    // computer.
  |
  |-> * code           // This is what you downloaded from GitHub.

On the contrary to previous versions, Visual Studio 2017 can use CMake files
directly, so instead of opening a project, choose “Open a folder” and select
the “code” folder. After a few seconds, Visual Studio should start processing
the CMake files and downloading the dependencies: this can take some time.

Once Visual Studio is done with the processing, you should now have a new menu
called “CMake” in the menu bar. In there you can build and clean the project,
edit the CMake configuration, and others.

After building, it is now time to run the assignment. In order to do that, you
need to choose the “Startup Item” by clicking on the combo box preceded by a
green arrow pointing to the right (similar to the “play” or “start” symbols),
and select, for example, “EDAF80_Assignment1.exe”.

Others (also works for Visual Studio 2017)
------------------------------------------

The project uses CMake to automate the building. It is recommended to build in
a separate folder but you can as well build directly within the source
directory. We will use the following directory layout::

  * some_random_folder // Wherever you want to store the project on your
  |                    // computer.
  |
  |-> * code           // This is what you downloaded from GitHub.
  |
  |-> * build          // A folder you created, and which will contain all the
                       // build related files.

First, you need to create the project files, be it for Unix Makefiles, Ninja,
Xcode, or Visual Studio::

  cd build
  cmake -G $generator ../code

where ``$generator`` can be, among others, ``"Visual Studio 14 2015 Win64"``
for Visual Studio 2015 on a 64-bit computer, ``"Visual Studio 15 2017 Win64"``
for Visual Studio 2017 on a 64-bit computer, ``Xcode`` for Xcode,
``"Unix Makefiles"`` for Unix Makefiles. To see the full list of generators
supported by CMake, have a look at `cmake-generators(7)`_.

If you prefer to use CMake GUI, just set the source directory to “code” and the
build directory to “build”. Then, press the “Configure” button, modify some of
the variables if needed, and press the button again. Finally, press the
“Generate” button to create the project files.

You should now have your project files available in the “build” folder. To
build the project, you can either use your IDE’s interface, or run
``cmake --build .`` from the “build” folder. You can pass
``--config Debug|RelWithDebInfo|Release`` to build in a specific configuration.

Issues with retrieving the resource archive
-------------------------------------------

If CMake fails to retrieve the resource archive, you will need to make sure to
extract the zip containing the resources into “code”; this should result in the
creation of a folder “res” under “code”.

Miscellaneous
=============

If you are on a laptop and would like to force the assignments to run on the
discrete GPU, set the option ``GLFW_USE_HYBRID_HPG`` to ``ON`` using CMake
— either from the CMake GUI or using CMake on the command line.

Licence
=======

* Dear ImGui is released under an MIT licence, see `Dear ImGui’s licence`_.
* This project is unlicenced, see `the UNLICENCE file`_.

.. _Lund University: http://www.lu.se/
.. _EDAF80: http://cs.lth.se/edaf80
.. _EDAN35: http://cs.lth.se/edan35
.. _CMake: https://cmake.org/
.. _Git: https://git-scm.com/
.. _GLFW: http://www.glfw.org/
.. _GLAD: https://github.com/Dav1dde/glad
.. _GLM: http://glm.g-truc.net/
.. _Dear ImGui: https://github.com/ocornut/imgui
.. _assimp: https://github.com/assimp/assimp
.. _stb: https://github.com/nothings/stb
.. _tinyfiledialogs: https://sourceforge.net/projects/tinyfiledialogs/
.. _cmake-generators(7): https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html
.. _Dear ImGui’s licence: src/external/Dear ImGui/LICENSE.txt
.. _OpenGL 3.3: https://github.com/LUGGPublic/CG_Labs/tree/OpenGL_3.3
.. _this ZIP file: http://fileadmin.cs.lth.se/cs/Education/EDA221/assignments/EDAF80_resources.zip
.. _the UNLICENCE file: UNLICENCE
