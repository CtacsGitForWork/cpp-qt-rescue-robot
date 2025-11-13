#pragma once

#include <QDebug>

#include "utility/geometry.h"
#include "game.h"

class Controller {
public:
    Controller(Game& game) : game_{game} {}

public:
    void OnMoveKey(Direction dir) {
        qInfo() << QString("<<< Moving in direction %1").arg(QString::fromStdString(ToString(dir)));
        Player& player = game_.GetPlayer();
        Direction current_dir = player.GetDirection();

        if (player.NowComing()) {
            return;
        }

        // Задаём направление, в котором будем двигаться.
        if (current_dir == dir) {
            qInfo() << "Trying to go";
            dir_ = dir;
            MovePlayer();
        } else {
            qInfo() << "Changing direction";
            player.SetDirection(dir);
        }
        qInfo() << QString(">>> New player position: %1 dir %2").
                   arg(QString::fromStdString(ToString(player.GetPosition()))).
                   arg(QString::fromStdString(ToString(dir)));
    }

    void OnReleaseMoveKey(Direction) {       
            dir_.reset();    
    }

    void SetRedrawCallback(const std::function<void()>& callback) {
        callback_ = callback;
    }

    void Tick() {
        game_.GetContext().timer.UpdateTime(clock_.GetElapsedTime());
        callback_();
    }

private:
    void MovePlayer() {
        if (!dir_.has_value()) {
            return;
        }

        Callback post_action = [this]() {
            this->MovePlayer();
        };

        Player& player = game_.GetPlayer();
        Direction current_dir = player.GetDirection();
        qDebug() << "Here" << player.NowComing();
        player.GoCommand(current_dir, post_action);
    }

private:
    Game& game_;
    Clock clock_;
    std::optional<Direction> dir_;
    std::function<void()> callback_;
};

