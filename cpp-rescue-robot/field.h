#pragma once

#include <memory>
#include <QDebug>
//#include <typeinfo>

#include "utility/utility.h"
#include "utility/painter.h"
#include "darkener.h"

#include "context.h"

class Character;

class ElementWithAsset {
public:
    ElementWithAsset(const Asset& asset) : asset_(asset) {}
    ElementWithAsset() = default;

protected:
    const Asset& GetAsset() const {
        return asset_;
    }

    void SetAsset(const Asset& asset) {
        asset_ = asset;
    }

private:
    Asset asset_;
};

class Tile : public ElementWithAsset {
public:
    using ElementWithAsset::ElementWithAsset;

public:
    virtual void Draw(Coordinate pos, DrawContext& context) const {
        double darkness = context.darkener.GetDarkness(CoordinateF(pos));
        //qDebug() << typeid(*this).name() << "::Draw darkness = " << darkness;
        context.painter.DrawFloor(GetAsset(), pos, darkness);
    }
};

class Wall : public ElementWithAsset {
public:
    using ElementWithAsset::ElementWithAsset;

public:
    virtual bool CanPass(const Character& character, Direction dir) const = 0;
    virtual void Interact(Character&, Direction) {};
    virtual void Draw(DrawContext& context, Coordinate pos, Orientation dir) const {
        double darkness = context.darkener.GetDarkness(CoordinateF(pos));
        //qDebug() << typeid(*this).name() << "::Draw darkness = " << darkness;

        if (GetAsset().pixmap.isNull()) {
            qCritical() << "[Wall] Pixmap is NULL for door at" << pos.x_pos << pos.y_pos;
            return;
        }

        context.painter.DrawWall(GetAsset(), pos, dir, darkness);
    }
};

// struct Cell описывает одну клетку игрового поля.
// Достаточно хранить только левую и верхнюю стену.
// Правая стена будет левой для клетки справа, а
// нижняя будет верхней для клетки снизу.
struct Cell {
    std::shared_ptr<Tile> floor = nullptr;
    std::shared_ptr<Wall> left_wall = nullptr;
    std::shared_ptr<Wall> top_wall = nullptr;
};

class Floor {
public:
    Floor();
    Floor(int level, int w, int h)
        : level_(level)
        , width_(w)
        , height_{h}
        , cells_{Array2D<Cell>(w + 1, h + 1)} {
    }

public:
    int GetLevel() const {
        return level_;
    }

    void SetTile(Coordinate2D where, std::shared_ptr<Tile> tile) {
        cells_.Get(where).floor = tile;
    }

    void SetWall(Coordinate2D where, Direction dir, std::shared_ptr<Wall> wall) {
        GetWallPtr(where, dir) = wall;
    }

    std::shared_ptr<Wall> GetWall(Coordinate2D where, Direction dir) {
        return GetWallPtr(where, dir);
    }

    void DrawFloor(DrawContext& context) const {
        for (int i = 0; i < width_; ++i) {
            for (int j = 0; j < height_; ++j) {
                std::shared_ptr<Tile> tile = cells_.Get(i, j).floor;

                if (tile != nullptr) {
                    tile->Draw(Coordinate(i, j, level_), context);
                }
            }
        }
    }

    // Рисует целую линию горизонтальных стен.
    void DrawHWalls(DrawContext& context, int y) const {
        for (int i = 0; i <= width_; ++i) {
            std::shared_ptr<Wall> wall = cells_.Get(i, y).top_wall;

            if (wall != nullptr) {
                wall->Draw(context, Coordinate(i, y, level_), Orientation::kHorizontal);
            }
        }
    }

    void DrawVWall(DrawContext& context, Coordinate pos) const {
        std::shared_ptr<Wall> wall = cells_.Get(Coordinate2D(pos)).left_wall;

        if (wall != nullptr) {
            wall->Draw(context, pos, Orientation::kVertical);
        }
    }

private:
    std::shared_ptr<Wall>& GetWallPtr(Coordinate2D where, Direction dir) {
        switch (dir) {
        case Direction::kUp:
            return cells_.Get(where).top_wall;
        case Direction::kLeft:
            return cells_.Get(where).left_wall;
        case Direction::kRight:
            ++where.x_pos;
            return cells_.Get(where).left_wall;
        case Direction::kDown:
            ++where.y_pos;
            return cells_.Get(where).top_wall;
        }
    }

private:
    int level_ = 0;
    int width_ = 0;
    int height_ = 0;
    Array2D<Cell> cells_;
};

class Field {
public:
    Field() = default;
    Field(int width, int height)
        : width_{width}
        , height_{height}
        , levels_{} {
    }

public:
    int GetWidth() const {
        return width_;
    }

    int GetHeight() const {
        return height_;
    }

    Size GetRect() const {
        return {width_, height_};
    }

    void AddFloor(int level) {
        levels_.insert({level, Floor(level, width_,height_)});
    }

    Floor& GetFloor(int level) {
        return levels_.at(level);
    }

    const Floor& GetFloor(int level) const{
        return levels_.at(level);
    }

private:
    int width_ = 0;
    int height_ = 0;
    std::map<int, Floor> levels_{};
};
