#pragma once

#include <QDebug>

#include "character.h"

class Player : public Character {
public:
    Player(GameContext& context, Coordinate position, Direction dir)
        : Character(context, position, dir) {
        qInfo() << QString("Player spawned on %1 dir %2").
            arg(QString::fromStdString(ToString(position))).
            arg(QString::fromStdString(ToString(dir)));

        SetSprites(context.asset_loader.LoadWithDirection("characters", "robot"));
        SetWalkSprites(context.asset_loader.LoadAnimatedWithDirection("characters", "robot-walk", 19));
    }

public:
    bool IsActive() const override {
        return true;
    }

    bool IsPlayer() const override {
        return true;
    }

    void GoCommand(Direction dir, Callback post_action) {
          Coordinate target_position = GetPosition();

        if (!CanPassWall(dir)) {
            qInfo() << "Player stoped on wall";
              GetContext().field.GetFloor(target_position.z_pos).GetWall(Coordinate2D(target_position), dir)->Interact(*this, dir);

            //for (auto& object : GetContext().object_map.Get(GetPosition() + Coordinate::FromDirection(dir))) {
            //    object->Interact(*this, dir);
            //}

            return;
        }

        if (!CanGo(dir)) {
            qInfo() << "Player stoped on object";
            /* auto objects = GetContext().object_map.Get(target_position + Coordinate::FromDirection(dir));
             for (const auto& obj : objects) {
                 obj->Interact(*this, dir);
             }*/

            for (auto& object : GetContext().object_map.Get(GetPosition() + Coordinate::FromDirection(dir))) {
                object->Interact(*this, dir);
            }

            return;
        }

        qInfo() << "Player moves";
        //SetPosition(target_position + Coordinate::FromDirection(dir));
        DoTransition(dir, post_action);
    }
};
