
# build settings is in file 'Matmakefile'

matmake2 = matmake2/build/matmake2

all: ${matmake2}
	@echo using Matmake buildsystem
	@echo for more options use 'matmake2 -h'
	@${matmake2} -t gcc

${matmake2}: matmake2/src/*.cpp
	make -C matmake2

clean:
	${matmake2} --clean
	
