include ../rules.inc

SOURCE =	cyclic.c start_app.c
CONTROL = 	cyclic.ctl
LOADIMAGE =	cyclic.l

default:	$(LOADIMAGE)
debug:		default
release:	default

$(LOADIMAGE):	$(OBJFILES)
	clink $(CONTROL)
	$(RM) *.asm *.obj *.map *.err *.dat

clean:
	-$(RM) *.obj *.asm *.map *.dat *.l *.err *.map
