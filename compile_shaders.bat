IF NOT EXIST "out\" MKDIR "out"

glslc -fshader-stage=vertex vertex.glsl -o out\vertex.spv || exit /b %errorlevel%
glslc -fshader-stage=fragment fragment.glsl -o out\fragment.spv || exit /b %errorlevel%

ECHO Compiling shaders... DONE
