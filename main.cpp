#include "application.h"

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    if (app.isRunning())
        return 0;
    return app.exec();
}
