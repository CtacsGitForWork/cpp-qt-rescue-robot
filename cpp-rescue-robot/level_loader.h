#pragma once

#include "utility/geometry.h"
#include "context.h"
#include "field.h"
#include "game.h"
#include "player.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <string>
#include <map>
#include <functional>

template<class T>
T LoadData(const QJsonValue &);

template<>
inline Coordinate LoadData<Coordinate>(const QJsonValue &json) {
    if (json.isObject()) {
        auto obj = json.toObject();
        int x = obj["x"].toInt();
        int y = obj["y"].toInt();
        int z = obj["z"].toInt();
        return Coordinate{x, y, z};
    } else if (json.isArray()) {
        auto obj = json.toArray();
        if (obj.size() == 2) {
            int x = obj[0].toInt();
            int y = obj[1].toInt();
            return Coordinate{x, y};
        } else if (obj.size() == 3 ) {
            int x = obj[0].toInt();
            int y = obj[1].toInt();
            int z = obj[2].toInt();
            return Coordinate{x, y, z};
        }
        throw std::runtime_error("Wrong array length for Coordinate");
    }
    throw std::runtime_error("Expected array or object for Coordinate");
}

template<>
inline Size LoadData<Size>(const QJsonValue &json) {
    Size s;
    if (json.isObject()) {
        auto obj = json.toObject();
        s.height = obj["h"].toInt();
        s.width = obj["w"].toInt();
        return s;
    } else if (json.isArray()) {
        auto obj = json.toArray();
        if (obj.size() != 2) {
            throw std::runtime_error("Wrong array length for size");
        }
        s.height = obj[1].toInt();
        s.width = obj[0].toInt();
        return s;
    }
    throw std::runtime_error("Expected array or object for size");
}

template<>
inline Direction LoadData<Direction>(const QJsonValue &json) {
    auto obj = json.toString();
    if (obj == "u") {
        return Direction::kUp;
    }
    if (obj == "d") {
        return Direction::kDown;
    }
    if (obj == "l") {
        return Direction::kLeft;
    }
    if (obj == "r") {
        return Direction::kRight;
    }
    throw std::runtime_error("Unknown directon " + obj.toStdString());
}

template<class T>
class Loader {
public:
    static std::shared_ptr<T> Load(const QJsonValue &json, GameContext& context, Floor* floor) {
        auto &loaders = GetLoaders();
        if (json.isString()) {
            auto type = json.toString();
            auto loader = loaders.at(type.toStdString());
            return loader(QJsonObject{}, context, floor);
        }
        auto type = json.toObject()["type"].toString();
        auto loader = loaders.at(type.toStdString());
        return loader(json.toObject(), context, floor);
    }

    using LoaderImpl = std::function<std::shared_ptr<T>(const QJsonObject&, GameContext& , Floor*)>;

    static void AddLoader(const std::string& type, LoaderImpl impl) {
        auto& loaders = GetLoaders();
        loaders[type] = impl;
    }

private:
    static std::map<const std::string, LoaderImpl> &GetLoaders() {
        static std::map<const std::string, LoaderImpl> loaders;
        return loaders;
  }
};

class LevelLoader {
public:
    LevelLoader() = default;

    bool LoadFromFile(Game& game, const QString& file_name) {
        QFile file(file_name);

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << QString("Couldn't open saved file.");
            return false;
        }

        QByteArray data = file.readAll();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (doc.isNull()) {
            qWarning() << "JSON parse error:" << error.errorString();
            return false;
        }

        Load(game, doc.object());
        
        return true;
    }

    void Load(Game& game, const QJsonObject &json) {
        game.Reset(LoadData<Size>(json["size"]));
        auto player = Loader<Player>::Load(json["player"], game.GetContext(), nullptr);
        game.SetPlayer(player);

        auto floors_array = json["floors"].toArray();
        for (auto value : floors_array) {
            QJsonObject floor = value.toObject();
            auto& floor_object = game.AddFloor(floor["level"].toInt());

            auto regions = floor["regions"].toArray();
            for (auto regions_value : regions) {
                auto region = regions_value.toObject();
                auto size = LoadData<Size>(region["size"]);
                auto position = LoadData<Coordinate>(region["pos"]);

                for (int i = position.x_pos; i < position.x_pos + size.width; i++) {
                    for (int j = position.y_pos; j < position.y_pos + size.height; j++) {
                        floor_object.SetTile({i, j}, Loader<Tile>::Load(region["tile"], game.GetContext(), &floor_object));
                    }
                }
                SetWalls(game, floor_object, position, size, region);

            }

            if (!floor["walls"].isNull()) {
                auto walls = floor["walls"].toArray();
                for (auto walls_value : walls) {
                    auto wall = walls_value.toObject();

                    AddWalls(game, floor_object, LoadData<Coordinate>(wall["pos"]), wall["length"].toInt(), LoadData<Direction>(wall["dir"]), LoadData<Direction>(wall["dir"]), wall["wall"]);
                }
            }

            if (!floor["objects"].isNull()) {
                auto objects = floor["objects"].toArray();
                for (auto objects_value : objects) {
                    auto object = objects_value.toObject();
                    if (object["type"] == "stair") {
                        auto tmp_arr = object["pos"].toArray();
                        tmp_arr.push_back(floor["level"].toInt());
                        object["pos"] = tmp_arr;
                    }
                        auto obj = Loader<Object>::Load(object, game.GetContext(), nullptr);
                        game.AddObject(obj);
                }
            }

        }
    }

private:
    void AddWalls(Game& game, Floor& floor, Coordinate2D pos, int length, Direction line_dir, Direction wall_dir, const QJsonValue& data) {
        for (auto i = 0; i < length; i++) {
            auto where = pos + Coordinate2D(Coordinate::FromDirection(line_dir)) * i;        

            qDebug() << "Wall data type:" << data.type() << ", content:" << data;
            
            auto wall = Loader<Wall>::Load(data, game.GetContext(), &floor);
            floor.SetWall(where, wall_dir, wall);
        }
    }

    void SetWalls(Game& game, Floor& floor_object, Coordinate2D position, Size size, const QJsonObject& region) {
        foreach (const auto& region_key, region.keys()) {
            if ( std::find(wall_types.begin(), wall_types.end(), region_key.toStdString()) == wall_types.end() ) {
                continue;
            }

            auto wall_dir = ToOrientation(LoadData<Direction>(QJsonValue::fromVariant(region_key.back()))) == Orientation::kHorizontal ? Direction::kLeft : Direction::kUp;
            auto line_dir = ToOrientation(LoadData<Direction>(QJsonValue::fromVariant(region_key.back()))) == Orientation::kHorizontal ? Direction::kDown : Direction::kRight;
            auto offset = Coordinate2D{0, 0};
            auto length = size.height;

            if (region_key.back() == 'r') {
                offset = Coordinate2D{size.width, 0};
            }
            if (region_key.back() == 'd') {
                offset = Coordinate2D{0, size.height};
            }

            if (region_key.back() == 'u') {
                length = size.width;
            }

            AddWalls(game, floor_object, position + offset, length, line_dir, wall_dir, region[region_key]);
        }
    }
    void AddSafeSpace(Game&, Floor&, const QJsonValue& ) {
        return;
    }

    std::vector<std::string> wall_types = {"wall_r", "wall_l", "wall_u", "wall_d"};
};
