include ../rules.inc

SOURCE =	lu_decomp.c 
CONTROL = 	lu_decomp.ctl
LOADIMAGE =	lu_decomp.l

default:	$(LOADIMAGE)
debug:		default
release:	default

$(LOADIMAGE):	$(OBJFILES)
	clink $(CONTROL)
	$(RM) *.asm *.obj *.map *.err *.dat

clean:
	-$(RM) *.obj *.asm *.map *.l *.err *.dat
