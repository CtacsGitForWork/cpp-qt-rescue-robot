#pragma once

#include <memory>

#include "utility/random.h"
#include "utility/assets.h"
#include "utility/timer.h"


#include "context.h"
#include "field.h"
#include "player.h"
#include "inventory.h"


class Game {
public:
    Game() {}
    Game(int w, int h) : field_(w, h) {}
    Game(AssetLoader asset_loader, int w, int h) : asset_loader_(asset_loader), field_(w, h) {

    }

    GameContext& GetContext() {
        return context_;
    }

    Inventory& GetInventory() { return inventory_; }

    Floor& AddFloor(int level) {
        field_.AddFloor(level);
        return field_.GetFloor(level);
    }

    void SetPlayer(std::shared_ptr<Player> player) {
        player_ = player;
    }

    Field& GetField() {
        return field_;
    }

    Player& GetPlayer() {
        return *player_;
    }

    Floor& GetCurrentFloor() {
        Coordinate current = player_->GetPosition();
        return field_.GetFloor(current.z_pos);
    }

    void DrawFrame(DrawContext& context) {
        int level = player_->GetPosition().z_pos;
        Floor& floor = field_.GetFloor(level);

        floor.DrawFloor(context);

        for (int j = 0; j <= field_.GetHeight(); ++j) {
            floor.DrawHWalls(context, j);
        }

        for (int i = 0; i <= field_.GetWidth(); ++i) {
            for (int j = 0; j <= field_.GetHeight(); ++j) {
                Coordinate current_pos{i, j, level};
                floor.DrawVWall(context, current_pos);
                const auto& objects = object_map_.Get(current_pos);

                for (const auto& obj : objects) {
                    if (!obj || !obj->IsVisible()) {
                        continue;
                    }

                    const auto pos = obj->GetPosition();
                    if (pos.x_pos < 0 || pos.y_pos < 0 || pos.z_pos < 0) {
                        qWarning() << "Skipping object with invalid position";
                        continue;
                    }

                    obj->Draw(context);
                }
            }
        }
    }


    void AddObject(const std::shared_ptr<Object> object) {
        if (!object) {
            qWarning() << "Tried to add nullptr object";
            return;
        }
        qDebug() << "Adding object at" << object->GetPosition().x_pos << object->GetPosition().y_pos
                 << "type:" << typeid(*object).name();
        objects_.push_back(object);
        object_map_.Place(object->GetPosition(), object.get());
    }

    void Reset(Size size) {
        field_ = Field(size.width, size.height);
    }
private:
    ObjectMap<Object> object_map_;
    AssetLoader asset_loader_{};
    Field field_; 
    RandomGen random_;
    Timer timer_;
    Inventory inventory_;  // Добавляем инвентарь
    GameContext context_{.object_map = object_map_, .field = field_, .random = random_, .asset_loader = asset_loader_, .timer = timer_, .inventory = inventory_};
    std::shared_ptr<Player> player_ = nullptr;
    std::vector<std::shared_ptr<Object>> objects_{};
};

