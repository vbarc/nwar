IF NOT EXIST "out\" MKDIR "out"

glslc -fshader-stage=vertex shader-vertex.glsl -o out\shader-vertex.spv || exit /b %errorlevel%
glslc -fshader-stage=fragment shader-fragment-main.glsl -o out\shader-fragment-main.spv || exit /b %errorlevel%
glslc -fshader-stage=fragment shader-fragment-wireframe.glsl -o out\shader-fragment-wireframe.spv || exit /b %errorlevel%

ECHO Compiling shaders... DONE
