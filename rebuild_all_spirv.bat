@echo off

echo Nuking binaries...

cd build/x64/debug/res/shader_binaries/
del *.spv

cd build/x64/release/res/shader_binaries/
del *.spv

cd ../../../../../

echo Binaries nuked. Building new ones:

echo Debug
for %%f in (./Breakout/shaders/*.vert) do glslangValidator -V -o ./build/debug/res/shader_binaries/%%f.spv ./Breakout/shaders/%%f
for %%f in (./Breakout/shaders/*.frag) do glslangValidator -V -o ./build/debug/res/shader_binaries/%%f.spv ./Breakout/shaders/%%f

echo Release
for %%f in (./Breakout/shaders/*.vert) do glslangValidator -V -o ./build/release/res/shader_binaries/%%f.spv ./Breakout/shaders/%%f
for %%f in (./Breakout/shaders/*.frag) do glslangValidator -V -o ./build/release/res/shader_binaries/%%f.spv ./Breakout/shaders/%%f

pause