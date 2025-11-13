#pragma once

#include <optional>

#include "context.h"
#include <QDebug>

class Character;

class Object {
public:
    Object(GameContext& context, Coordinate position)
        : context_{context}
        , position_{position}
        , prev_position_{position}   {
        PlaceToMap(position_);
    }

    ~Object() {
        if (position_.x_pos >= 0) { // Проверяем валидность позиции
        RemoveFromMap(position_);
        }
    }

public:
    // Метод Interact будет вызываться, когда персонаж character, двигаясь в
    // направлении dir, попытается переместиться на клетку, в которой расположен объект.
    // Например, метод Interact у лестницы будет перемещать персонажа на другой этаж,
    // если он подошёл с нужной стороны.
    virtual void Interact(Character&, Direction) {}

    // Метод CanCover определяет, может ли персонаж character встать на клетку с объектом.
    virtual bool CanCover(const Character&, Direction) const {
        return !IsVisible();
    }

	// Отрисовывем объект.
    //virtual void Draw(DrawContext& context) const = 0;

    virtual void Draw(DrawContext&) const {
        if (!IsVisible()) return;

        if (position_.x_pos < 0 || position_.y_pos < 0 || position_.z_pos < 0) {
            qWarning() << "Attempted to draw object with invalid position";
            return;
        }

        qWarning() << "Draw() not implemented for object";
    }


    Coordinate GetPosition() const {
        return position_;
    }

    virtual void SetPosition(Coordinate position) {
        target_position_.reset();
		RemoveFromMap(position_);
        prev_position_ = position_;
        position_ = position;
        PlaceToMap(position_);
    }
	
	virtual CoordinateF GetExactPos() const {
		return CoordinateF(position_);
	}

    Coordinate GetPrevPosition() const {
        return prev_position_;
    }

	void SetTargetPosition(Coordinate position) {
		if (target_position_.has_value()) {
            RemoveFromMap(target_position_.value());
		}
			
        target_position_ = position;
        PlaceToMap(target_position_.value());
	}

    /*void Disappear() {
        if (!visibility_) return; // Уже невидим

        qDebug() << "Object disappeared at" << position_.x_pos << position_.y_pos;
        visibility_ = false;
        RemoveFromMap(position_);
        position_ = Coordinate{-1, -1, -1}; // Невалидная позиция
        if (target_position_) {
            RemoveFromMap(target_position_.value());
            target_position_.reset();
        }
    }*/

    void Disappear() {
        if (!visibility_) return;  // Уже невидим

        qDebug() << "[Disappear] Object at" << position_.x_pos << position_.y_pos << position_.z_pos << "is disappearing";

        // Удаляем из object_map текущую позицию
        if (position_.x_pos >= 0 && position_.y_pos >= 0 && position_.z_pos >= 0) {
            RemoveFromMap(position_);
        }

        // Удаляем из карты target-позицию, если объект был в движении
        if (target_position_) {
            RemoveFromMap(*target_position_);
            target_position_.reset();
        }

        // Не трогаем position_ напрямую — он остаётся как есть
        // Можно будет использовать для отладки, если нужно

        visibility_ = false;
      //  removed_ = true;  // Новый флаг, говорящий, что объект больше не участвует в логике
    }





    bool IsVisible() const {
        return visibility_;
    }

    GameContext& GetContext() const {
        return context_;
    }

private:
    void RemoveFromMap(Coordinate position) {
       // qDebug() << "[Object] RemoveFromMap:" << position.x_pos << position.y_pos << position.z_pos << "ptr:" << this;
        context_.object_map.Remove(position, this);
    }

    void PlaceToMap(Coordinate position) {
        //qDebug() << "[Object] PlaceToMap:" << position.x_pos << position.y_pos << position.z_pos << "ptr:" << this;
        context_.object_map.Place(position, this);
    }

private:
    GameContext& context_;
    Coordinate position_;
    Coordinate prev_position_{};
	// Есть значение, когда объект перемещается с одной клетки на другую.
	std::optional<Coordinate> target_position_;
    bool visibility_ = true;
};
