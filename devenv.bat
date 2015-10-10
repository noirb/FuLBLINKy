@echo off

REM ----------------------------------------------
REM startup script to set environment variables
REM for a Windows development environment.
REM
REM Fill in the paths below, and then run this
REM script to open the project.
REM
REM PLEASE do not check this file in after
REM filling in the paths below! Locations of
REM these dependencies may be completely different
REM for different users.
REM ----------------------------------------------

REM ---------------------
REM Include directories
REM ---------------------

REM GLEW
set GLEW_INC=<path_to_glew/include>

REM GLM
set GLM_INC=<path_to_glm>

REM GLFW
set GLFW_INC=<path_to_glfw.bin.<platform>/include>

REM CEGUI
set CEGUI_INC=<path_to_cegui/include>;<path_to_cegui/binaries/include>


REM ---------------------
REM Lib directories
REM ---------------------

REM GLEW
set GLEW_LIB=<path_to_glew/lib/Release/<platform>>

REM GLFW
set GLFW_LIB=<path_to_glfw/binaries.<platform>/src/Release>

REM CEGUI
set CEGUI_LIB=<path_to_cegui/binaries/lib>

REM Open project
start CFD_Project.sln