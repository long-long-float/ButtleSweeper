SOFTNAME  = mine
EXTENSION = cpp
OBJLIST = Bitmap.obj DebugConsole.obj ArgumentParser.obj

DEFAULTLIB =kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib msimg32.lib

$(SOFTNAME).exe: $(SOFTNAME).obj $(OBJLIST)
	link /SUBSYSTEM:WINDOWS /OUT:$(SOFTNAME).exe $(DEFAULTLIB) $(SOFTNAME).obj $(OBJLIST)

$(EXTENSION).obj: 
	cl /c $<
