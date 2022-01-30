where /q cmake
if ERRORLEVEL 1 (
    echo CMAKE is missing. Ensure it is placed in your PATH.
    exit /B
)

pushd %~dp0
@rem do not call emsdk_env.bat if it has already been done
@if "%EM_CONFIG%"=="" call "..\..\emsdk\emsdk_env.bat"

set MAKE="make"

call emcmake cmake -DCMAKE_BUILD_TYPE=Release -D"CMAKE_MAKE_PROGRAM:PATH=%MAKE%" .. -G "MinGW Makefiles"
call emmake %MAKE% -j2

copy /b index.html +,,

@rem Create a server with %EMSDK_PYTHON% -m http.server or emrun index.html --serve_root ..
