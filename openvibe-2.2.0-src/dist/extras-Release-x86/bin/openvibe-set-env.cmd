@ECHO OFF 
 
SET "dependencies_base=D:\openvibe-2.2.0-src\\dependencies" 
 

SET PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\\bin;%PATH%

SET PATH=%dependencies_base%\gtk\bin;%PATH%
SET OGRE_HOME=%dependencies_base%\ogre
SET PATH=%OGRE_HOME%\bin\release;%OGRE_HOME%\bin\debug;%PATH%
SET PATH=%dependencies_base%\cegui\bin;%PATH%
SET PATH=%dependencies_base%\cegui\dependencies\bin;%PATH%
SET VRPNROOT=%dependencies_base%\vrpn
SET PATH=%VRPNROOT%\bin;%PATH%

SET PATH=%dependencies_base%\vcredist_100\;%PATH%
SET PATH=%dependencies_base%\vcredist_110\;%PATH%
SET PATH=%dependencies_base%\vcredist\;%PATH%

REM Apply user-provided Python2.7 paths if available
IF NOT !PYTHONHOME27!==!EMPTY!  IF NOT !PYTHONPATH27!==!EMPTY! SET REPLACE_PYTHON=true
IF NOT !REPLACE_PYTHON!==!EMPTY! (
	SET "PYTHONHOME=%PYTHONHOME27%"
	SET "PYTHONPATH=%PYTHONPATH27%"
)

