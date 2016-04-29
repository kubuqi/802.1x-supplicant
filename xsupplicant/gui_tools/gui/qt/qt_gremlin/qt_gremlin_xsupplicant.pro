unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
!exists( gui_interface.cpp )
{
    # We create a symlink to the .c file as a .cpp file to make 
    # qmake happy...
    system(ln -s ../../../common/gui_interface.c gui_interface.cpp)
}
!exists( gui_interface.h)
{
    system(ln -s ../../../common/gui_interface.h gui_interface.h)
}
























































































































































































































































































































































































































































































TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release


HEADERS	+= gui_interface.h
SOURCES	+= main.cpp \
	gui_interface.cpp
FORMS	= Xsupplicant.ui

