include __crtl00.obj
include __fdiv.obj
include __getregs.obj
include __getreg.obj
include __putreg.obj
include __getsreg.obj
include __putsreg.obj
include __initrtl.obj
include __pe_delay.obj
include __pe_halt.obj
include __sdiv.obj
include __udiv.obj
include intdiv.obj
include string.obj
include Ctype.obj
include errno.obj
include stdlib.obj
include inspector.obj
include clasm\quadinfo.obj
include clasm\semlib.obj
include clasm\control.obj
include clasm\mtclib.obj
include clasm\syslib.obj
include start_app.obj
include block.obj

SHARE:    0-fffff   no_overlay_chk DATA SHARE  {*(share)}
DRAM:     0-fffff   no_overlay_chk DATA DRAM   {*(dram)}
DATA:     0-fffff   no_overlay_chk DATA        {*(data)}
SDRAM:    0-fffff   no_overlay_chk DATA SDRAM  {*(sdram)}
TEXT:     0-7ffff   no_overlay_chk TEXT        {*(text)}
