#include <QApplication>
#include "mainwindow.h"
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    std::cout << "╔════════════════════════════════════╗" << std::endl;
    std::cout << "║         CINEBASE (Qt C++)          ║" << std::endl;
    std::cout << "║      Video Library Manager         ║" << std::endl;
    std::cout << "╚════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
