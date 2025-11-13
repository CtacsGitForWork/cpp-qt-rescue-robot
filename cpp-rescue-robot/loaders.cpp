
#include "characters.h"
#include "field_elements.h"
#include "field.h"
#include "level_loader.h"
#include "objects.h"
#include "player.h"
#include <utility/geometry.h>
#include "objects.h"

struct PlayerLoader {
    PlayerLoader() {
        Loader<Player>::AddLoader("player", [](const QJsonObject &json, GameContext& context, Floor*) -> std::shared_ptr<Player> {
            Coordinate pos = LoadData<Coordinate>(json["pos"]);
            Direction dir = LoadData<Direction>(json["dir"]);
            return std::make_shared<Player>(context, pos, dir);
        });
    }
} player_loader;

struct VictimLoader {
    VictimLoader() {
        Loader<Object>::AddLoader("victim", [](const QJsonObject &json, GameContext& context, Floor*) {
            Coordinate pos = LoadData<Coordinate>(json["pos"]);
            Direction dir = LoadData<Direction>(json["dir"]);
            return std::make_shared<Victim>(context, pos, dir);
        });
    }
} victim_loader;


struct StairLoader {
    StairLoader() {
        Loader<Object>::AddLoader("stair", [](const QJsonObject &json, GameContext& context, Floor*) {
            Coordinate pos = LoadData<Coordinate>(json["pos"]);
            Direction dir = LoadData<Direction>(json["dir"]);
            auto down = json["down"].toBool();
            return std::make_shared<Stairs>(context, pos, dir, down);
        });
    }
} stair_loader;


struct GrassLoader {
    GrassLoader() {
        Loader<Tile>::AddLoader("grass", [](const QJsonObject &, GameContext& context, Floor*) {
            return std::make_shared<FloorTile>(context, "grass");
        });

    }
} grass_loader;


struct DoorLoader {
    DoorLoader() {
        Loader<Wall>::AddLoader("door", [](const QJsonObject &json, GameContext& context, Floor*) {
            // Парсим параметры из самого объекта двери (не из вложенного "wall")
            std::optional<std::string> key_name;
            if (json.contains("key")) {
                key_name = json["key"].toString().toStdString();
            }

            std::optional<QColor> color;
            if (json.contains("color")) {
                QString color_str = json["color"].toString();
                color = QColor(color_str);
                if (!color->isValid()) {
                    qWarning() << "Invalid door color:" << color_str;
                    color.reset();
                }
            }

        qDebug() << "Creating door with key:"
                 << (key_name ? QString::fromStdString(*key_name) : "None")
                 << "and color:" << (color ? color->name() : "None");


            return std::make_shared<Door>(context, key_name, color);
        });
    }
} door_loader;


struct MarbleLoader {
    MarbleLoader() {
        Loader<Tile>::AddLoader("marble-black", [](const QJsonObject &, GameContext& context, Floor*) {
            return std::make_shared<FloorTile>(context, "floor4");
        });

    }
} marble_loader;

struct EdgeWallLoader {
    EdgeWallLoader() {
        Loader<Wall>::AddLoader("edge", [](const QJsonObject &, GameContext& context, Floor*) {
            return std::make_shared<EdgeWall>(context);
        });

    }
} edge_wall_loader;

struct EmptyWallLoader {
    EmptyWallLoader() {
        Loader<Wall>::AddLoader("empty", [](const QJsonObject &, GameContext& , Floor*) {
            return std::make_shared<EmptyWall>();
        });

    }
} empty_wall_loader;

struct KeyLoader {
    KeyLoader() {
        Loader<Object>::AddLoader("key", [](const QJsonObject &json, GameContext& context, Floor*) {
            Coordinate pos = LoadData<Coordinate>(json["pos"]);
            auto key_name = json["name"].toString();
            auto key_color = QColor::fromString(json["color"].toString());
            return std::make_shared<Key>(context, pos, key_name.toStdString(), key_color);
        });
    }
} key_loader;

struct HydrantLoader {
    HydrantLoader() {
        Loader<Object>::AddLoader("hydrant", [](const QJsonObject &json, GameContext& context, Floor*) {
            Coordinate pos = LoadData<Coordinate>(json["pos"]);
            return std::make_shared<Hydrant>(context, pos);
        });
    }
} hydrant_loader;

struct FireLoader {
    FireLoader() {
        Loader<Object>::AddLoader("fire", [](const QJsonObject &json, GameContext& context, Floor*) {
            Coordinate pos = LoadData<Coordinate>(json["pos"]);
            return std::make_shared<Fire>(context, pos);
        });
    }
} fire_loader;
