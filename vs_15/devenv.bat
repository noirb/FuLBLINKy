@echo off

REM ---------------------------------------------------------------------------
REM startup script to set environment variables for a Windows dev environment.
REM
REM Fill in the paths below, and then run this script to open the project.
REM
REM
REM PLEASE do not check this file in after filling in the paths below!
REM Locations of these dependencies may be completely different for
REM different users.
REM ---------------------------------------------------------------------------

REM ----------------------------------------------------------
REM 	Fill in the following four paths for your environment
REM ----------------------------------------------------------

set CEGUI_ROOT=<path_to_cegui_root>
set GLFW_ROOT=<path_to_glfw_root>
set GLEW_ROOT=<path_to_glew_root>
set GLM_ROOT=<path_to_glm_root>


REM ----------------------------------------------------------
REM 	It should not be necessary to edit below this line
REM ----------------------------------------------------------

REM Set environment variables used by build system
call .\scripts\set_env.bat

if errorlevel 1 goto error

REM Open project
start CFD_Project.sln

if errorlevel 0 goto end

:error
echo ERROR: Paths were not set correctly! Please edit devenv.bat with the correct paths.
exit /B 1

:end
exit /B 0
