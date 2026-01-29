test.exe : test.obj engine.obj cache1d.obj a.obj
	wlink debug all f test f engine f cache1d f a system dos4g

test.obj : test.c build.h pragmas.h
	wcc386 test /d2
cache1d.obj : cache1d.c pragmas.h
	wcc386 cache1d /d2
engine.obj : engine.c build.h pragmas.h ves2.h
	wcc386 engine /d2
a.obj : a.c
	wcc386 a /d2
