set CFLAGS=-march=i386 -O1 -flto -fwhole-program -Wall -Wextra -Wpedantic -Wno-attributes -DRANGECHECK

@set GLOBOBJS=
@set GLOBOBJS=%GLOBOBJS% draw.c
@set GLOBOBJS=%GLOBOBJS% engine.c
@set GLOBOBJS=%GLOBOBJS% engine2.c
@set GLOBOBJS=%GLOBOBJS% filesystem.c
@set GLOBOBJS=%GLOBOBJS% p_setup.c
@set GLOBOBJS=%GLOBOBJS% palette.c
@set GLOBOBJS=%GLOBOBJS% tables.c
@set GLOBOBJS=%GLOBOBJS% test.c
@set GLOBOBJS=%GLOBOBJS% tiles.c
@set GLOBOBJS=%GLOBOBJS% z_zone.c

gcc %GLOBOBJS% %CFLAGS% -o release/dj32test.exe
strip -s release/dj32test.exe
stubedit release/dj32test.exe dpmi=CWSDPR0.EXE
