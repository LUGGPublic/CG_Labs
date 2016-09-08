LU_Computer_Graphics_Labs contains the source code for the various Computer
Graphics courses taught at `Lund University`_ (EDA221_ and EDAN35_). For now,
only Assignment 1 of EDA221 is available.

The public repository is found at https://github.com/LUGGPublic/CG_Labs.

Dependencies
============

LU_Computer_Graphics_Labs uses the following dependencies:

* CMake_: to generate project files and handle dependencies;
* GLFW_: to create a window and an OpenGL context, as well as handle inputs;
* GLAD_: to load OpenGL extensions;
* GLM_: for linear algebra operations;
* `Dear ImGui`_: to display information on screen, such as logs;
* assimp_: to load OBJ models;
* lodepng_: to load PNG files;

The project contains the needed files for GLAD and lodepng, and it will
download assimp, Dear ImGui, GLM and GLFW if they are not found on your
computer, so you only need to make sure that CMake is installed on your
computer.

An OpenGL 4.1 context is created by the project. You could possibly lower that
to an OpenGL 3.3 context, but it is not supported.

The different resources used in the assignments, textures, object files, can
be found in `this ZIP file`_.

Building
========

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

First, you need to create the project files, be it for Makefile, Ninja, Xcode,
Visual Studio::

  cd build
  cmake ../code

If you prefer to use CMake GUI, just set the source directory to ``code`` and
the build directory to ``build``. Then, press the ``Configure`` button, modify
some of the variables if needed, and press the button again. Finally, press the
``Generate`` button to create the project files.

You should now have your project files available in the ``build`` folder. To
build the project, you can either use your IDE’s interface, or run
``cmake --build .`` from the ``build`` folder. You can pass
``--config Debug|RelWithDebInfo|Release`` to build in a specific configuration.

Before running the project, make sure to extract the zip containing the
resources into ``code``; this should result in the creation of a folder
``res`` under ``code``.

Licence
=======

* Dear ImGui is released under an MIT licence, see `Dear ImGui’s licence`_.
* lodepng’s licence can be found at the top of every lodepng file.

.. _Lund University: http://www.lu.se/
.. _EDA221: http://cs.lth.se/eda221
.. _EDAN35: http://cs.lth.se/edan35
.. _CMake: https://cmake.org/
.. _GLFW: http://www.glfw.org/
.. _GLAD: https://github.com/Dav1dde/glad
.. _GLM: http://glm.g-truc.net/
.. _Dear ImGui: https://github.com/ocornut/imgui
.. _assimp: https://github.com/assimp/assimp
.. _lodepng: http://lodev.org/lodepng/
.. _Dear ImGui’s licence: Dear_ImGui_LICENCE
.. _this ZIP file: http://fileadmin.cs.lth.se/cs/Education/EDA221/assignments/EDA221_resources.zip
