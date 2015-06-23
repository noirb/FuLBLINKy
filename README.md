# CFD_Project
A fluid flow visualization program focused on 3D fluid data.

# Build Dependencies
  - GLFW3  (http://www.glfw.org/)
  - GLM    (http://glm.g-truc.net/0.9.6/index.html)
  - CEGUI  (http://cegui.org.uk/)
  - A graphics driver compatible with OpenGL 3.3+

# Build Instructions - Linux
  - Install dependencies for GLFW & CEGUI first (includes GLM, so you don't need to install it separately):

    ```
    sudo apt-get install cmake xorg-dev libglu1-mesa-dev cmake-curses-gui libglew-dev libglm-dev
    ```
  
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
