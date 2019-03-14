@ECHO OFF
SETLOCAL EnableDelayedExpansion
SETLOCAL EnableExtensions

REM Get the directory location of this script, assume it contains the OpenViBE dist tree. These variables will be used by OpenViBE executables.
SET "OV_PATH_ROOT=%~dp0"
SET "OV_PATH_BIN=%OV_PATH_ROOT%\bin"
SET "OV_PATH_LIB=%OV_PATH_ROOT%\bin"
SET "OV_PATH_DATA=%OV_PATH_ROOT%\share\openvibe"
SET "MENSIA_PATH_DATA=%OV_PATH_ROOT%\share\mensia"

SET OV_PAUSE=
SET OV_RUN_IN_BG=

IF /i "%1"=="--no-pause" (
	SET OV_PAUSE=
	SHIFT
)
IF /i "%1"=="--pause" (
	SET OV_PAUSE=PAUSE
	SHIFT
)
IF /i "%1"=="--run-bg" (
	REM Run in background, disable pause. The first start arg is the app title.
	SET OV_RUN_IN_BG=START "%OV_PATH_ROOT%/bin/openvibe-designer.exe"
	SET OV_PAUSE=
	SHIFT
)

SET LUA_EXE_PATH=
FOR %%F IN (lua.exe) DO (
	SET LUA_EXE_PATH_FULL=%%~$PATH:F
	SET LUA_EXE_PATH=!LUA_EXE_PATH_FULL:lua.exe=!
)

SET PATH=%OV_PATH_LIB%;%PATH%
SET LUA_CPATH=%LUA_CPATH%;%LUA_EXE_PATH%\clibs\?.dll;%OV_PATH_ROOT%\bin\?.dll
SET LUA_PATH=%LUA_PATH%;%LUA_EXE_PATH%\lua\?.lua;%OV_PATH_ROOT%\bin\?.lua

REM get the arguments
REM this allows to build the full argument list (even when # > 9)
REM %* is not modifed by SHIFT
SET "args="
:getArgs
IF "%~1" NEQ "" (
  SET args=%args% %1
  SHIFT
  goto :getArgs
)

%OV_RUN_IN_BG% "%OV_PATH_ROOT%/bin/openvibe-designer.exe"  %args%

%OV_PAUSE%
