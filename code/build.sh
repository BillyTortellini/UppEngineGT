#Directories
dir=~/UppEngineGT
ldir=${dir}/libs
odir=${dir}/build
cdir=${dir}/code

source=game.cpp
flags='-Wall -fPIC -shared' 
output=${odir}/libgame.so
output_cpy=${odir}/libgame_tmp.so

# Compile the library
g++ $flags -o $output $source

# Copy result for dynamic code loading
cp $output $output_cpy

# Set library search path
export LD_LIBRARY_PATH=$odir
