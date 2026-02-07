#CFLAGS="-march=i286 -mcmodel=medium -li86 -mnewlib-nano-stdio -Os -fomit-frame-pointer -mregparmcall -DC_ONLY -DRANGECHECK -DZONEIDCHECK -flto -fwhole-program"
CFLAGS="-march=i286 -mcmodel=medium -li86 -mnewlib-nano-stdio -O0 -fomit-frame-pointer -mregparmcall -ffunction-sections -Wall -Wextra -Wno-maybe-uninitialized -DC_ONLY -DRANGECHECK -DZONEIDCHECK"

GLOBOBJS=""
GLOBOBJS+=" draw.c"
GLOBOBJS+=" engine.c"
GLOBOBJS+=" engine2.c"
GLOBOBJS+=" filesystem.c"
GLOBOBJS+=" p_setup.c"
GLOBOBJS+=" palette.c"
GLOBOBJS+=" tables.c"
GLOBOBJS+=" test.c"
GLOBOBJS+=" tiles.c"
GLOBOBJS+=" z_zone.c"

ia16-elf-gcc $GLOBOBJS $CFLAGS -o release/ia16test.exe
