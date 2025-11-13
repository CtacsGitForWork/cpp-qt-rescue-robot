#pragma once

#include <QDebug>

#include "context.h"
#include "inventory.h"
#include "object.h"
#include "character.h"

class Stairs : public Object {
public:
    Stairs(GameContext& context, Coordinate position, Direction dir, bool down)
        : Object(context, position)
        , dir_{dir}
        , down_{down} {
		SetSprites(context.asset_loader.LoadWithDirection("objects", down ? "dstair" : "stair"));
    }

public:
    void Interact(Character& character, Direction dir) override {
        if (dir != Invert(dir_)) {
            qInfo() << "Trying to go stairs in the wrong dir";
            return;
        }

        qInfo() << QString("Go stairs %1").arg(QString::fromStdString(ToString(dir)));
        Coordinate target_position	= character.GetPosition()
									+ Coordinate::FromDirection(dir) * 2
									+ Coordinate{0, 0, down_ ? -1 : 1};

        character.SetPosition(target_position);
    }

    void Draw(DrawContext& context) const override {
        if (!sprites_.has_value()) {
            return;
        }

        Asset asset = sprites_.value().Get(dir_);
		double darkness = context.darkener.GetDarkness(CoordinateF(GetPosition()));       
        context.painter.DrawObject(asset, CoordinateF(GetPosition()), darkness);
    }

protected:
    void SetSprites(const AssetInDirections& sprites) {
        sprites_ = sprites;
    }

private:
    Direction dir_;
    bool down_ = false;
    std::optional<AssetInDirections> sprites_;
};

class Key : public Object {
public:
    Key(GameContext& context,
        Coordinate position,
        const std::string& name,
        QColor color
        ) : Object(context, position), name_{name}, color_(color) {
        sprite_ = context.asset_loader.LoadSprite("objects", "key");
        sprite_.ChangeColor(color);
    }

    void Interact(Character& character, Direction) override {
        if (!character.IsPlayer() || !IsVisible()) {
            qWarning() << "Failed to store key in inventory";
            return;
        }

            // Создаем копию имени и цвета перед Disappear()
            std::string key_name = name_;
            QColor key_color = color_;
            
        try {            
            auto inv_key = std::make_shared<InventoryKey>(GetContext(), name_, color_);

            // Проверяем, что ключ создан корректно
            if (!inv_key || !inv_key->GetAsset()) {
                throw std::runtime_error("Failed to create inventory key");
            }

            if (!GetContext().inventory.Store(inv_key)) {
                qWarning() << "Failed to store key in inventory";
            }
            Disappear();
        }
        catch (const std::exception& e) {
            qCritical() << "Key interaction error:" << e.what();
        }
    }

    void Draw(DrawContext& context) const override {
        double darkness = context.darkener.GetDarkness(CoordinateF(GetPosition()));
        context.painter.DrawObject(sprite_, CoordinateF(GetPosition()), darkness);
    }

private:
    std::string name_;
    QColor color_;
    Asset sprite_;
};

class Fire : public Object {
public:
    Fire(GameContext& context, Coordinate position)
        : Object(context, position),
        frame_timer_{context.timer}    {
        sprites_ = context.asset_loader.LoadAnimatedObject("objects", "fire/", 20);

        auto result = frame_timer_.PlanCallback(frame_timer_.Now() + 40, [this]() {
            current_frame_ = (current_frame_ + 1) % sprites_.frames.size();
            auto inner_result = frame_timer_.PlanCallback(frame_timer_.Now() + 40, [this]() {
                current_frame_ = (current_frame_ + 1) % sprites_.frames.size();
            });
        });
    }

    void Interact(Character& character, Direction) override {
        if (character.IsPlayer()) {
            auto water = GetContext().inventory.FindItem([](auto item) {
                return item->IsWater();
            });
            if (water) {
                GetContext().inventory.Remove(water);
                Disappear();
            }
        }
    }

    void Draw(DrawContext& context) const override {
        const Asset& frame = sprites_.GetFrame(current_frame_);
        double darkness = context.darkener.GetDarkness(CoordinateF(GetPosition()));
        context.painter.DrawObject(frame, CoordinateF(GetPosition()), darkness);
    }

private:
    AnimatedAsset sprites_;
    int current_frame_ = 0;
    Timer& frame_timer_;
};

class Hydrant : public Object {
public:
    Hydrant(GameContext& context, Coordinate position)
        : Object(context, position) {
        sprite_ = context.asset_loader.LoadSprite("objects", "hydrant");
    }

    void Interact(Character& character, Direction) override {
        if (character.IsPlayer()) {
            bool has_water = GetContext().inventory.FindItem([](auto item) {
                return item->IsWater();
            });
            if (!has_water) {
                auto water = std::make_shared<InventoryWater>(GetContext());
                GetContext().inventory.Store(water);
            }
        }
    }

    void Draw(DrawContext& context) const override {
        double darkness = context.darkener.GetDarkness(CoordinateF(GetPosition()));
        context.painter.DrawObject(sprite_, CoordinateF(GetPosition()), darkness);
    }

private:
    Asset sprite_;
};

