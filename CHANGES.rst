Revision history for CG_Labs


v2019.1 YYYY-MM-DD
==================

* Add a “CHANGES.rst” file that will list the different modifications done,
  from now on.
* Ensure Log is destroyed before its clients (GitLab #45)
* Unify the TRS interfaces between the node and the TRSTransform classes, by
  using and exposing a TRSTransform instance inside the node class (GitLab #46)
* Select polygon mode from GUI, and simplify it (GitLab #47)
* Toggle visualisation of light cones in wireframe mode from the GUI
* Switch between shaders from GUI (GitLab #48)
* Edit node::render() to take parent transform
* Rename `WindowManager::CreateWindow()` to
  `WindowManager::CreateGLFWWindow()`, to avoid conflict with Windows API
  macro.
* Replace lodepng with stb, to also support JPEG file loading (GitLab #41)
* Add shader for celestial rings and load it in lab1 (GitLab #51)
* AppVeyor: switch from VS 2019 Preview to VS 2019
* README: Add tinyfiledialogs to dependencies
* (HEAD -> 2019_updates) README: Sort the list of dependencies
