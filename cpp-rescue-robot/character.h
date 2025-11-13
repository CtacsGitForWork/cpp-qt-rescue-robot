#pragma once

#include <QDebug>
//#include <typeinfo>
#include <algorithm>

#include "utility/assets.h"
//#include "utility/random.h"
#include "utility/animation.h"

#include "field.h"
#include "object.h"

class Character : public Object {
public:
    Character(GameContext& context, Coordinate position, Direction dir)
        : Object(context, position)
        , dir_{dir} {
    }

public:
    virtual bool IsActive() const {
        return false;
    }

    virtual bool IsPlayer() const {
        return false;
    }

    Wall* GetWall(Direction dir) const {
        auto field = this->GetContext().field.GetFloor(this->GetPosition().z_pos).GetWall(this->GetPosition(), dir);
        return field.get();
    }

    void Draw(DrawContext& context) const override {
        if (!IsVisible()) {
            qDebug() << "Object is hidden!";
            return;
        }

        if (!sprites_.has_value()) {
            return;
        }
		
		CoordinateF char_pos = GetExactPos();
		const Asset* asset;
		
        if (!walk_sprites_.has_value()) {
            asset = &sprites_.value().Get(dir_);
        } else {
			const AnimatedAsset& assets = walk_sprites_.value().Get(dir_);
			
			if (transition_.has_value()) {
                asset = &assets.GetAt(transition_->GetProgress());
			} else {
				asset = &assets.GetFrame(0);
			}
		}

		double darkness = context.darkener.GetDarkness(char_pos);
        //qDebug() << typeid(*this).name() << "::Draw darkness = " << darkness;
        context.painter.DrawObject(*asset, char_pos, darkness);
    }
	
	bool NowComing() const {
        if (transition_) {
            return true;
        }
        return false;
      //  return transition_.has_value();
	}
	
	virtual int GetStepDuration() const {
        return 200;
	}

	CoordinateF GetExactPos () const override {
        /*if (!transition_.has_value()) {
			return CoordinateF(GetPosition());

        }*/
        if (transition_) {
            return transition_->GetPosition();
        }
        //return transition_.value().GetPosition();        
        return GetPosition();
	}
	
	void SetPosition(Coordinate position) override {
        if (transition_.has_value()) {
			transition_.value().StopNow();
			transition_.reset();
        }

		Object::SetPosition(position);
	}
	
    void SetDirection(Direction dir) {
        dir_ = dir;
    }

    Direction GetDirection() const {
        return dir_;
    }

    bool CanPassWall(Direction dir) const {
        Coordinate pos = GetPosition();
        std::shared_ptr<Wall> wall = GetContext().field.GetFloor(pos.z_pos).GetWall({pos.x_pos, pos.y_pos}, dir);
        if (wall != nullptr) {
            return wall->CanPass(*this, dir);
        }
        return true;
    }

    bool CanGo(Direction dir) const {
        Coordinate pos = GetPosition() + Coordinate::FromDirection(dir);


        auto objects = GetContext().object_map.Get(pos);
        return std::all_of(objects.begin(), objects.end(), [&](Object* obj) {
            return obj->CanCover(*this, dir);
        });

    }
	
    /*
	void DoTransition(Direction dir, Callback at_end_action = {}) {
		if (transition_.has_value()) {
			transition_.value().StopNow();
			transition_.reset();
		}
		
		auto cur_pos = GetPosition();
		auto dst_pos = cur_pos + Coordinate::FromDirection(dir);
		SetTargetPosition(dst_pos);
		
        auto post_effect_action = [=]() {
			SetPosition(dst_pos);
			at_end_action();
		};
		
        transition_.emplace( Transition<CoordinateF>(
			GetContext().timer,
			CoordinateF(cur_pos),
			CoordinateF(dst_pos),
			200,					// Duration.
			post_effect_action
        ));
    }*/

    void DoTransition(Direction dir, Callback at_end = {}) {
        if (transition_) {
            transition_->StopNow();            
        }
        auto cur_pos = GetPosition();
        auto dst_pos = cur_pos + Coordinate::FromDirection(dir);
        SetTargetPosition(dst_pos);

        transition_.emplace(GetContext().timer, cur_pos, dst_pos, GetStepDuration(), [=, this](){
            SetPosition(dst_pos);

            if (at_end) {
                at_end();
            }
        });
    }

protected:
    void SetSprites(const AssetInDirections& sprites) {
        sprites_ = sprites;
    }

    void SetWalkSprites(const AnimatedAssetInDirections& sprites) {
        walk_sprites_ = sprites;
    }

private:
    Direction dir_;
    std::optional<AssetInDirections> sprites_;
    std::optional<AnimatedAssetInDirections> walk_sprites_;
	
	// Когда перемещаемся между двумя клетками.
    std::optional<Transition<CoordinateF>> transition_;
};
