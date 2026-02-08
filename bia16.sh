CFLAGS="-march=i286 -mcmodel=medium -li86 -mnewlib-nano-stdio -Ofast -fomit-frame-pointer -mregparmcall -flto -fwhole-program -Wall -Wextra -Wno-maybe-uninitialized -DRANGECHECK -DZONEIDCHECK"

ia16-elf-gcc -c engine.c     -march=i286 -mcmodel=medium -Og    -fomit-frame-pointer -mregparmcall -flto -fwhole-program -Wall -Wextra -Wno-maybe-uninitialized -DRANGECHECK -DZONEIDCHECK
ia16-elf-gcc -c engine2.c    -march=i286 -mcmodel=medium -Og    -fomit-frame-pointer -mregparmcall -flto -fwhole-program -Wall -Wextra -Wno-maybe-uninitialized -DRANGECHECK -DZONEIDCHECK
ia16-elf-gcc -c filesystem.c -march=i286 -mcmodel=medium -O0    -fomit-frame-pointer -mregparmcall -flto -fwhole-program -Wall -Wextra -Wno-maybe-uninitialized -DRANGECHECK -DZONEIDCHECK
ia16-elf-gcc -c p_setup.c    -march=i286 -mcmodel=medium -Ofast -fomit-frame-pointer -mregparmcall                       -Wall -Wextra -Wno-maybe-uninitialized -DRANGECHECK -DZONEIDCHECK

GLOBOBJS=""
GLOBOBJS+=" draw.c"
GLOBOBJS+=" engine.o"
GLOBOBJS+=" engine2.o"
GLOBOBJS+=" filesystem.o"
GLOBOBJS+=" p_setup.o"
GLOBOBJS+=" palette.c"
GLOBOBJS+=" tables.c"
GLOBOBJS+=" test.c"
GLOBOBJS+=" tiles.c"
GLOBOBJS+=" z_zone.c"

ia16-elf-gcc $GLOBOBJS $CFLAGS -o release/ia16test.exe

rm engine.o
rm engine2.o
rm filesystem.o
rm p_setup.o
