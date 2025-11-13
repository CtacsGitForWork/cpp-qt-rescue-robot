#include "level_loader.h"
#include "mainwindow.h"


#include <QApplication>

void MakeRoom(Floor& floor, std::shared_ptr<Wall> wall, std::shared_ptr<Tile> tile, int x1, int x2, int y1, int y2) {
    for (int i = x1; i <= x2; ++i) {
        for (int j = y1; j <= y2; ++j) {
            floor.SetTile({ i, j }, tile);
            if (i == x1) {
                floor.SetWall({ i, j }, Direction::kLeft, wall);
            }
            if (i == x2) {
                floor.SetWall({ i, j }, Direction::kRight, wall);
            }
            if (j == y1) {
                floor.SetWall({ i, j }, Direction::kUp, wall);
            }
            if (j == y2) {
                floor.SetWall({ i, j }, Direction::kDown, wall);
            }
        }
    }
}

int main(int argc, char* argv[])
{  
    QApplication a(argc, argv);
    Game game{};
    LevelLoader level_loader;

    if (level_loader.LoadFromFile(game, "://resources/levels/main_level.json")) {  
        qDebug() << "Level loaded successfully from main_level.json";
    } else {
        qWarning() << "Failed to load level from main_level.json";
    }

    Controller controller{ game };
    MainWindow w{ game, controller };
    w.show();
    return a.exec();
}

