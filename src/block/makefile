include ../rules.inc

SOURCE =	block.c start_app.c
CONTROL = 	block.ctl
LOADIMAGE =	block.l

default:	$(LOADIMAGE)
debug:		default
release:	default

$(LOADIMAGE):	$(OBJFILES)
	clink $(CONTROL)
	$(RM) *.asm *.obj *.map *.err *.dat

clean:
	-$(RM) *.obj *.asm *.map *.dat *.l *.err *.map
