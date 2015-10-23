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

REM ----------------------------------------------------------
REM 	Fill in the following four paths for your environment
REM ----------------------------------------------------------

set CEGUI_ROOT=<path_to_cegui_dir>
set GLFW_ROOT=<path_to_glfw_dir>
set GLEW_ROOT=<path_to_glew_dir>
set GLM_ROOT=<path_to_glm_dir>



REM ----------------------------------------------------------
REM 			Do not edit below this line
REM =========================================================
REM ----------------------------------------------------------

REM ---------------------
REM Include directories
REM ---------------------

REM GLEW
set GLEW_INC=%GLEW_ROOT%\include

REM GLM
set GLM_INC=%GLM_ROOT%

REM GLFW
set GLFW_INC=%GLFW_ROOT%\include

REM CEGUI
set CEGUI_INC=%CEGUI_ROOT%\cegui\include;%CEGUI_ROOT%\binaries\cegui\include


REM ---------------------
REM Lib directories
REM ---------------------

REM GLEW
set GLEW_LIB=%GLEW_ROOT%\lib\Release\x64

REM GLFW
set GLFW_LIB=%GLFW_ROOT%\binaries.x64\src\Release

REM CEGUI
set CEGUI_LIB=%CEGUI_ROOT%\binaries\lib

REM Open project
start CFD_Project.sln