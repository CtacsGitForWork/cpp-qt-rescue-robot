#pragma once

#include <QDebug>
#include <optional>

#include "context.h"
#include "inventory.h"
#include "utility/random.h"
#include "darkener.h"

#include "field.h"
#include "character.h"

class FloorTile : public Tile {
public:
    using Tile::Tile;
	FloorTile(GameContext& context, const std::string& asset_name) {      
        Asset asset = context.asset_loader.LoadTile("floors", asset_name);        
        int right_angels = context.random.GetRandomElem<int>({0, 1, 2, 3});
        asset.Rotate(right_angels);		
		SetAsset(asset);
    }
};

class EmptyTile : public Tile {
public:
    using Tile::Tile;
};

class Door : public Wall {
public:
    Door(GameContext& context,
         std::optional<std::string> key,
         std::optional<QColor> color = std::nullopt)
        : key_(key), color_(color) {

            Asset asset = context.asset_loader.LoadTile("walls", "wall-white");
        if (color) {
            asset.ChangeColor(*color);
            qDebug() << "Door created with color:" << color->name();
        } else {
            // Стандартный цвет двери - серый
            asset.ChangeColor(QColor(100, 100, 100));
            qDebug() << "Door created with default color";
        }
            SetAsset(asset);
        qDebug() << "=== Door created ===";
        qDebug() << "Position: [5, 10] (hardcoded for debug)";
        qDebug() << "Key:" << (key ? QString::fromStdString(*key) : "None");
        qDebug() << "Color:" << (color ? color->name() : "None");
    }

public:
    void Draw(DrawContext& context, Coordinate pos, Orientation dir) const override {
		if (is_opened_) {
			return;
		}
        // Используем цвет из JSON, если он задан, иначе стандартный серый
        QColor door_color = color_ ? *color_ : QColor(100, 100, 100, 200);       
        QColor edge_color = QColor(50, 50, 100, 128);
        const int edge_width = 2;
        double darkness = context.darkener.GetDarkness(CoordinateF(pos));

        // Определяем точки для прямоугольника в зависимости от направления
        CoordinateF p1 = pos;
        CoordinateF p2 = p1 + CoordinateF{0, 0, 1}; // Верхняя точка двери

        CoordinateF p3;
        if (dir == Orientation::kHorizontal) {
            p3 = p2 + CoordinateF{1, 0, 0}; // Горизонтальная дверь
        } else {
            p3 = p2 + CoordinateF{0, 1, 0}; // Вертикальная дверь
        }

        context.painter.DrawRect(p1, p2, p3, door_color, edge_color, edge_width, darkness);
	}
	
    void Interact(Character& character, Direction) override {
        if (character.IsPlayer() && character.IsActive()) {         
            // Проверяем наличие ключа в инвентаре
            auto* key_item = character.GetContext().inventory.FindItem(
                [this](const auto& item) {
                    auto as_key = item ? item->AsKey() : nullptr; //item->AsKey();
                    return as_key && as_key->GetName() == *key_;
                });

            if (key_item) {
                qDebug() << "Opening door with key:" << QString::fromStdString(*key_);
                is_opened_ = true;
            } else {
                qDebug() << "Door requires key:" << QString::fromStdString(*key_);
            }
        }
    }

    bool CanPass(const Character& character, Direction) const override {
        if (is_opened_) {
            return true;
        }

        // Если дверь закрыта, проверяем наличие ключа
        if (key_ && character.IsPlayer()) {
            return character.GetContext().inventory.FindItem(
                [this](const auto& item) {
                    auto as_key = item->AsKey();
                    return as_key && as_key->GetName() == *key_;
                });
        }

        return false;
    }

private:
    bool is_opened_ = false;
    std::optional<std::string> key_;  // Имя ключа (если дверь закрыта на ключ)
    std::optional<QColor> color_;     // Цвет двери (из JSON)
};

class EmptyWall : public Wall {
public:
    using Wall::Wall;

public:
    void Draw(DrawContext&, Coordinate, Orientation) const override {}
	bool CanPass(const Character&, Direction) const override {
        return true;
    }
};

class EdgeWall : public Wall {
public:
    using Wall::Wall;
	EdgeWall(GameContext& context) {
        Asset asset = context.asset_loader.LoadTile("walls", "wall-white");
        SetAsset(asset);	
    }

public:
    bool CanPass(const Character&, Direction) const override {
        return false;
    }
};

