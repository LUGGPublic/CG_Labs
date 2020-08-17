Revision history for CG_Labs


v2019.1 2019-09-06
==================

New features
------------

* Select polygon mode from GUI, and simplify it
* Toggle visualisation of light cones in wireframe mode from the GUI
* Switch between shaders from GUI
* Replace lodepng with stb, to also support JPEG file loading
* Add shader for celestial rings and load it in lab1

Improvements
------------

* Add a “CHANGES.rst” file that will list the different modifications done,
  from now on.
* Unify the TRS interfaces between the node and the `TRSTransform` classes, by
  using and exposing a `TRSTransform` instance inside the node class
* Edit node::render() to take parent transform
* AppVeyor: switch from VS 2019 Preview to VS 2019
* README: Add tinyfiledialogs to dependencies
* README: Sort the list of dependencies

Fixes
-----

* Ensure Log is destroyed before its clients
* Rename `WindowManager::CreateWindow()` to
  `WindowManager::CreateGLFWWindow()`, to avoid conflict with Windows API
  macro.
