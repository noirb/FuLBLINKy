# CFD_Project
A fluid flow visualization program focused on 3D fluid data.

# Build Dependencies
  - GLFW3      (http://www.glfw.org/)
  - GLM        (http://glm.g-truc.net/0.9.6/index.html)
  - CEGUI 0.8+ (http://cegui.org.uk/)
  - GTK 3.0+
  - A graphics driver compatible with OpenGL 3.3+

# Build Instructions - Linux
  - Install dependencies for OpenGL development, GTK, GLFW & CEGUI first (this includes GLM, so there's no need to build & install it separately):

    ```
    sudo apt-get install cmake xorg-dev libglu1-mesa-dev cmake-curses-gui libglew-dev libglm-dev libgtk-3-dev
    ```
    NOTE: CEGUI also requires an image codec (like libfreeimage or DevIL) and an XML reader. Many systems will already have both, but if not you should be able to apt-get the library of your choice. e.g.
    ```
    sudo apt-get install libfreeimage-dev libxml2-dev libexpat1-dev
    ```
    
  #### Build GLFW
    GLFW 3 is not available in package form for all distributions. If you can't find it in your package manager (most only have GLFW 2), you'll have to build & install it from source.
  
  - Download & extract the GLFW source from here: http://www.glfw.org/download.html
  - Create a directory to build in:
  
    ```
    mkdir build
    cd build
    ```
  
  - Configure GLFW to build as a Shared Object
  
    ```
    ccmake <path_to_GLFW_source_dir>
      - Press 'c' to configure the project
      - Place cursor on option labeled BUILD_SHARED_LIBS, and press 'enter' to toggle it to ON
      - Press 'g' to generate & exit
    ```
  
  - Build & install GLFW
  
    ```
    make
    sudo make install
    ```

  #### Build CEGUI
    CEGUI 0.8 is not currently available in package form, and needs to be built & installed from source
    
  - Download & extract the CEGUI source from here: http://cegui.org.uk/download
  - Instructions are available in the README.md file in their source package, but I'll summarize them here
  - Create a directory to build in:

    ```
    mkdir build
    cd build
    ```

  - Configure CEGUI. The default options should be fine.

    ```
    cmake <path_to_CEGUI_source_dir>
    ```

  - Build & install CEGUI

    ```
    make
    sudo make install
    ```

  #### Build the Visualizer

    If everything went well with the above steps, you should now have everything you need to build the project. Just make sure your LD_LIBRARY_PATH points to the location the GLFW and CEGUI libs were installed to (/usr/local/lib by default) and  run `make` from the project directory.
    
  ###### Taking the visualizer out of the source tree
    Apart from the final executable, fluid-vis, you also need to copy the `shaders` and `cegui_layout` directories, which contain files necessary for the program to function.
    The `build` directory just contains object and dependency files generated during the build, and can safely be deleted or ignored.
