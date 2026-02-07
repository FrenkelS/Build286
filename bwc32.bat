set CFLAGS=-3r -ot -bcl=dos4g -q -wx -DRANGECHECK

@set GLOBOBJS=
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

wcl386 %GLOBOBJS% %CFLAGS% -fe=release/wc32test.exe -fm=release/wc32test.map

del *.err
del *.obj
