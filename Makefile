CC = g++

LD = g++

OBJ_DIR = ./

OUT_DIR = ./bin
INC_HERE = ./

CPPFLAGS = -g -I$(INC_HERE) -I/usr/include `pkg-config --cflags gtkmm-3.0`
#-I/usr/include/gtkmm-3.0 -I/usr/include/glibmm-2.4


CFLAGS = -std=c++11 -g -Wall $(CPPFLAGS)


#SYSLIBS = -L/usr/lib64 -L/usr/lib
SYSLIBS = `pkg-config --libs gtkmm-3.0`



CFILES = main.cc main-window.cc options-dialog.cc tp-canvas.cc gcode-grid.cc gcode-manager.cc


$(OBJ_DIR)/%.o : %.cc
	$(CC) -o $@ -c $(CFLAGS) $<


#$(OBJ_DIR)/tp-canvas.o  $(OBJ_DIR)/options-dialog.o $(OBJ_DIR)/main-window.o $(OBJ_DIR)/main.o

all: 
	$(CC) $(CFLAGS) -o 2dG $(CFILES) $(LDFLAGS) $(SYSLIBS)


lint :
	lint -px -std1  $(CPPFLAGS) ${CFILES}


clean:
	rm $(OBJ_DIR)/*.o
