pushd %~dp0
@rem do not call emsdk_env.bat if it has already been done
@if "%EM_CONFIG%"=="" call "..\..\emsdk\emsdk_env.bat"

emrun index.html --serve_root ..
