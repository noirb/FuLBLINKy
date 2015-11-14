@echo off
TITLE F u L B L I N K y

REM ----------------------------------------------
REM  Sets the necessary environment variables to
REM  build the project from Visual Studio.
REM ----------------------------------------------

REM ---------------------
REM Test required paths
REM ---------------------
if not exist %CEGUI_ROOT% goto err_nocegui
if not exist %GLFW_ROOT% goto err_noglfw
if not exist %GLM_ROOT% goto err_noglm
if not exist %GLEW_ROOT% goto err_noglew

REM ---------------------
REM Include directories
REM ---------------------

REM GLEW
set GLEW_INC=%GLEW_ROOT%\include
if not exist %GLEW_INC% goto err_noglewinc

REM GLM
set GLM_INC=%GLM_ROOT%

REM GLFW
set GLFW_INC=%GLFW_ROOT%\include
if not exist %GLFW_ROOT% goto err_noglfwinc

REM CEGUI
set CEGUI_INC=%CEGUI_ROOT%\cegui\include;%CEGUI_ROOT%\binaries\cegui\include
if not exist %CEGUI_INC% goto err_noceguiinc

REM ---------------------
REM Lib directories
REM ---------------------

REM GLEW
set GLEW_LIB=%GLEW_ROOT%\lib\Release\x64
if not exist %GLEW_LIB% goto err_noglewlib

REM GLFW
set GLFW_LIB=%GLFW_ROOT%\lib-vc2015
if not exist %GLFW_LIB% goto err_noglfwlib

REM CEGUI
set CEGUI_LIB=%CEGUI_ROOT%\binaries\lib
if not exist %CEGUI_LIB% goto err_noceguilib

goto success


:err_nocegui
echo   CEGUI Root directory was not found: %CEGUI_ROOT%
goto err

:err_noceguiinc
echo   CEGUI_INC directory could not be found: %CEGUI_INC%
goto err

:err_noceguilib
echo   CEGUI_LIB directory could not be found: %CEGUI_LIB%
goto err

:err_noglew
echo   GLEW Root directory was not found: %GLEW_ROOT%
goto err

:err_noglewinc
echo   GLEW_INC directory could not found: %GLEW_INC%
goto err

:err_noglewlib
echo   GLEW_LIB directory could not be found: %GLEW_LIB%
goto err

:err_noglm
echo   GLM Root directory was not found: %GLM_ROOT%
goto err

:err_noglfw
echo   GLFW Root directory was not found: %GLFW_ROOT%
goto err

:err_noglfwinc
echo   GLFW_INC directory could not be found: %GLFW_INC%
goto err

:err_noglfwlib
echo   GLFW_LIB directory could not be found: %GLFW_LIB%
goto err

:err
exit /B 1

:success
exit /B 0