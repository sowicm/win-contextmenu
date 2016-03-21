
all: uninstall kill-explorer file install start-explorer

file:
	b2

kill-explorer:
	-taskkill /f /im explorer.exe
	#wmic process where name="explorer.exe" call terminate

start-explorer:
	explorer

install:
	copy /y dist\\MyShellExtension.dll C:\\Windows\\System32
	regsvr32 C:\\Windows\\System32\\MyShellExtension.dll

uninstall:
	-regsvr32 /u C:\\Windows\\System32\\MyShellExtension.dll

clean:
	rd /s /q bin

distclean: clean

.PHONY: all install uninstall clean distclean
