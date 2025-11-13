#pragma once

#include "utility/utility.h"
#include "utility/timer.h"

class Object;
class Field;
class RandomGen;
class AssetLoader;
class Timer;
class Inventory;

struct GameContext {
    ObjectMap<Object>& object_map;
    Field& field;
    RandomGen& random;
    AssetLoader& asset_loader;
    Timer& timer;
    Inventory& inventory;
};

class Painter;
class Darkener;

struct DrawContext {
    Painter& painter;
    const Darkener& darkener;
};
