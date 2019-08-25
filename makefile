Blocks.exe: main.o hsdlg.o nhsdlg.o statdlg.o aboutdlg.o data.o resource.res
	gcc -Wall -o Blocks.exe -mwindows main.o hsdlg.o nhsdlg.o statdlg.o aboutdlg.o data.o resource.res -lcomctl32 -luxtheme -lversion

main.o: main.c defs.h
	gcc -c main.c

hsdlg.o: hsdlg.c defs.h
	gcc -c hsdlg.c

nhsdlg.o: nhsdlg.c defs.h
	gcc -c nhsdlg.c

statdlg.o: statdlg.c defs.h
	gcc -c statdlg.c

aboutdlg.o: aboutdlg.c defs.h
	gcc -c aboutdlg.c

data.o: data.c defs.h
	gcc -c data.c

resource.res: resource.rc resource.h
	windres -O coff --codepage 65001 resource.rc resource.res

clean:
	del *.o;del *.res;del *.exe;del *.dat

