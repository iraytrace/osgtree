#include "MainWindow.h"
#include <QApplication>
#include <QFileInfo>
#include <QSettings>


void appSetup(const QString organizationName)
{
    // set up application name
    QFileInfo applicationFile(QApplication::applicationFilePath());

    // These allow us to simply construct a "QSettings" object without arguments
    qApp->setOrganizationDomain("mil.army.arl");
    qApp->setApplicationName(applicationFile.baseName());
    qApp->setOrganizationName(organizationName);
    qApp->setApplicationVersion(__DATE__ __TIME__);

    // Look up the last directory where the user opened files.
    // set it if it hasn't been set.
    QSettings settings;
    if (!settings.allKeys().contains("currentDirectory"))
        settings.setValue("currentDirectory",
                          applicationFile.path());
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    appSetup("Army Research Laboratory");
    MainWindow w;
    w.show();

    return a.exec();
}
