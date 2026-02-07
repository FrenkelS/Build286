set CFLAGS=-2 -os -bcl=dos -mm -k4096 -q -wx -DC_ONLY -DRANGECHECK -DZONEIDCHECK

@set GLOBOBJS=
@set GLOBOBJS=%GLOBOBJS% draw.c
@set GLOBOBJS=%GLOBOBJS% engine.c
@set GLOBOBJS=%GLOBOBJS% engine2.c
@set GLOBOBJS=%GLOBOBJS% filesystem.c
@set GLOBOBJS=%GLOBOBJS% global.c
@set GLOBOBJS=%GLOBOBJS% p_setup.c
@set GLOBOBJS=%GLOBOBJS% palette.c
@set GLOBOBJS=%GLOBOBJS% tables.c
@set GLOBOBJS=%GLOBOBJS% test.c
@set GLOBOBJS=%GLOBOBJS% tiles.c
@set GLOBOBJS=%GLOBOBJS% z_zone.c

wcl %GLOBOBJS% %CFLAGS% -fe=release/wc16test.exe -fm=release/wc16test.map

del *.err
del *.obj
