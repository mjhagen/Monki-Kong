@echo off

set name="monki"

set path=%path%;..\bin\

set CC65_HOME=..\

if exist BUILD\%name%.nes del BUILD\%name%.nes

cc65 -Oirs %name%.c --add-source
ca65 crt0_unrom07.s
ca65 %name%.s -g

ld65 -C unrom_07_hor.cfg -o %name%_unrom07.nes crt0_unrom07.o %name%.o nes.lib -Ln labels.txt

del *.o

move /Y labels.txt BUILD\
move /Y %name%.s BUILD\
move /Y %name%_unrom07.nes BUILD\

if exist BUILD\%name%.nes BUILD\%name%_unrom07.nes
