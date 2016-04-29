#include <qapplication.h>
#include <unistd.h>
#include "Xsupplicant.h"

#include <gui_interface.h>

int main( int argc, char ** argv )
{    

    
    QApplication a( argc, argv );    
    
    Xsupplicant w;
    
    w.main();
    
    a.exec();
}
