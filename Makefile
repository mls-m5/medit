
# build settings is in file 'Matmakefile'

matmake = matmake/matmake

all: ${matmake}
	@echo using Matmake buildsystem
	@echo for more options use 'matmake -h'
	@${matmake}

${matmake}:
	make -C &{matmake}

clean:
	${matmake} clean
	
