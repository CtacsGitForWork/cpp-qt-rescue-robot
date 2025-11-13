//#include "characters.h"
//#include "field_elements.h"
#include "level_loader.h"
#include "mainwindow.h"

//#include "objects.h"
//#include "player.h"

#include <QApplication>
//#include <QLoggingCategory>

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
   // QLoggingCategory::setFilterRules("*.debug=true");
    QApplication a(argc, argv);
    //AssetLoader loader;
    //Game game{ loader, 15, 15 };
    //Game game{ loader, 30, 30 };


    Game game{};
    LevelLoader level_loader;



    if (level_loader.LoadFromFile(game, "://resources/levels/main_level.json")) {
   // if (level_loader.LoadFromFile(game, "C:/Users/CTAC/cpp-qt/problems/sprint5/robot-9-level-loader/submission/resources/levels/main_level.json")) {
        qDebug() << "Level loaded successfully from main_level.json";
    } else {
        qWarning() << "Failed to load level from main_level.json";
    }



    //std::shared_ptr<Player> player = std::make_shared<Player>(game.GetContext(), Coordinate{ 5, 7, 0 }, Direction::kDown);
   /* std::shared_ptr<Player> player = std::make_shared<Player>(game.GetContext(), Coordinate{ 5, 5, 0 }, Direction::kRight);
    game.SetPlayer(player);

    Floor& floor0 = game.AddFloor(0);
    Floor& floor1 = game.AddFloor(1);

    std::shared_ptr<EdgeWall> edge_wall = std::make_shared<EdgeWall>(game.GetContext());
    std::shared_ptr<Victim> victim = std::make_shared<Victim>(game.GetContext(), Coordinate{ 7, 12, 0 }, Direction::kLeft);
    std::shared_ptr<EmptyWall> empty_wall = std::make_shared<EmptyWall>();
    std::shared_ptr<Door> door1 = std::make_shared<Door>();
    std::shared_ptr<Door> door2 = std::make_shared<Door>();

    std::shared_ptr<FloorTile> marble_tile = std::make_shared<FloorTile>(game.GetContext(), "floor4");

    floor0.SetWall({ 4, 4 }, Direction::kUp, edge_wall);
    floor0.SetWall({ 5, 4 }, Direction::kUp, door1);
    floor0.SetWall({ 6, 4 }, Direction::kUp, edge_wall);
    floor0.SetWall({ 4, 3 }, Direction::kUp, edge_wall);
    floor0.SetWall({ 5, 3 }, Direction::kUp, door2);
    floor0.SetWall({ 6, 3 }, Direction::kUp, edge_wall);

    MakeRoom(floor0, edge_wall, marble_tile, 4, 6, 0, 5);
    MakeRoom(floor1, edge_wall, marble_tile, 4, 6, 0, 2);
    MakeRoom(floor0, edge_wall, marble_tile, 0, 10, 6, 14);
    floor0.SetWall({ 5, 5 }, Direction::kDown, empty_wall);

    Stairs stair_up{ game.GetContext(), {5, 1, 0}, Direction::kDown, false };
    Stairs stair_down{ game.GetContext(), {5, 1, 1}, Direction::kUp, true };*/

    Controller controller{ game };

    /*
    Game game{15, 15};
    Player player{game.GetContext(), {5, 5, 0}, Direction::kRight};
    game.SetPlayer(&player);
    Controller controller{game};
    game.AddFloor(0);
    Victim victim{game.GetContext(), {12, 6, 0}, Direction::kRight};
    */


    MainWindow w{ game, controller };
    w.show();
    return a.exec();
}
