#include "PetrelTT.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    tPetrelTT window;
    window.show();
    return app.exec();
}
