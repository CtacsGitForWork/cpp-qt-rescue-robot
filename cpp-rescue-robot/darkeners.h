#pragma once

#include "darkener.h"
#include <algorithm>

class RadialDarkener : public Darkener {
public:
    RadialDarkener(CoordinateF start_point, int radius) : point_{ start_point }, radius_{ radius } {

    }

    double GetDarkness(CoordinateF pos) const override {
        return std::min(1., (pos - point_).Norm() / radius_);
    }

private:
    CoordinateF point_;
    int radius_;
};

class FlashlightDarkener : public Darkener {
public:
    FlashlightDarkener(CoordinateF start_point, int radius, Direction dir, int distance)
        : point_{ start_point }, radius_{ radius }, dir_{ dir }, distance_{ distance } {
    }

    double GetDarkness(CoordinateF pos) const override {
        auto vec = pos - point_;
        double length_in_dir = CoordinateF::Scalar(Coordinate::FromDirection(dir_), vec);

        if (length_in_dir > 0) {
            length_in_dir = std::max(0., length_in_dir - distance_);
        }

        double length_perpendicular = CoordinateF::Scalar(Coordinate::FromDirection(dir_).Rotate90(), vec);

        return std::min(1., std::hypot(length_in_dir, length_perpendicular) / radius_);
    }

private:
    CoordinateF point_;
    int radius_;
    Direction dir_;
    int distance_;
};
