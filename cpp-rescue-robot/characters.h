#pragma once

#include "character.h"
//#include "player.h"
#include "utility/random.h"

enum Phase {
    BROWNIAN, // Ходит хаотично, пока не встретил робота.
    FOLLOW,   // Встретил робота, ходит за роботом.
};

class Victim : public Character {
public:
    Victim(GameContext& context, Coordinate position, Direction dir)
		: Character(context, position, dir) {
        SetSprites(context.asset_loader.LoadWithDirection("characters", "victim"));
        NextStep();        
    }
    void Interact(Character& character, Direction) override {
        phase_ = Phase::FOLLOW;
        leader_player_ = &character;
    }

    void Wait(Callback wait_cb = [](){}){
        guard_ = GetContext().timer.PlanCallback(GetContext().timer.Now() + 200, wait_cb);
    }

    void MakeBrownianStep() {
        if (steps_ > 0 && CanPassWall(GetDirection()) && CanGo(GetDirection())) {
            steps_--;
            DoTransition(GetDirection(), [this](){NextStep(); });

           // return;
        } else {

       // if (steps_ < 1  || !(CanPassWall(GetDirection()) && CanGo(GetDirection()))) {
            auto dirs = std::vector{
                Direction::kDown, Direction::kLeft,
                Direction::kRight, Direction::kUp
            };
            dirs.erase(std::find(dirs.begin(), dirs.end(), GetDirection()));

            SetDirection(GetContext().random.GetRandomElem(dirs));

            steps_ = GetContext().random.GetInRange(2, 8);

            Wait([this](){NextStep();});           
           // steps_--;
        }


    }

    void MakeFollowStep() {
        if (GetDistance(GetPosition(), leader_player_->GetPosition()) > 5) {
            phase_ = Phase::BROWNIAN;
            NextStep();
            return;
        }

        auto direction_toward = GetDirectionToward(GetPosition(), leader_player_->GetPrevPosition());
        if (!direction_toward ||!(CanPassWall(leader_player_->GetDirection()) && leader_player_->CanGo(GetDirection())) ) {
            Wait([this](){NextStep(); });
        } else {
            SetDirection(direction_toward.value());
            DoTransition(GetDirection(), [this](){NextStep(); });
        }
    }

    void NextStep() {
        switch (phase_) {
        case Phase::BROWNIAN:
            MakeBrownianStep();
            //return;
            break;
        case Phase::FOLLOW:
            MakeFollowStep();
            break;
        default:
            return;
        }

    }

private:
    Phase phase_ = Phase::BROWNIAN;
    Character *leader_player_ = nullptr;
    int steps_ = GetContext().random.GetInRange(2, 8);
    Timer::Guard guard_{};
};
