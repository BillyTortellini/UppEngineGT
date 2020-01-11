#Directories
dir=~/UppEngineGT
ldir=${dir}/libs
odir=${dir}/build
cdir=${dir}/code/linux-xwindow

#Compiler arguments
source="${cdir}/main.cpp ${ldir}/SDL2-2.0.10/build/libSDL2-2.0.so"
output=${odir}/main
flags=-Wall
includes="-I ${ldir}/SDL2-2.0.10/include -I ${cdir}/.."
libs="-l dl"

#Command
g++ $flags -o $output $source $libs $includes
