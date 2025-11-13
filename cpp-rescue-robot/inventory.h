#pragma once

#include "context.h"
#include "qcolor.h"
#include "utility/assets.h"
#include "utility/painter.h"


class InventoryKey;


class InventoryObject {
public:
    // Возвратит указатель на InventoryKey, если объект является ключом.
    virtual const InventoryKey* AsKey() const {
        return nullptr;
    }

    // Возвратит true, если объект является водой.
    virtual bool IsWater() const {
        return false;
    }

    // Нарисует элемент в инвентаре.
    virtual void Draw(DrawContext& context, int offset) const = 0;
};


class InventoryKey : public InventoryObject {
public:
    InventoryKey(const GameContext& context,
                 const std::string& name,
                 QColor color
                 ) : key_name_{name} {
        asset_ = context.asset_loader.LoadSprite("inventory", "inventory-key");
        if (asset_.pixmap.isNull()) {
            qWarning() << "Failed to load inventory-key sprite";
        }
        asset_.ChangeColor(color);
               // Отладочный вывод
        qDebug() << "InventoryKey created:" << name.c_str()
                 << "Color:" << color.name();
    }

    const std::string GetName() const {
        return key_name_;
    }

    const InventoryKey* AsKey() const override {
        return this;
    }

    void Draw(DrawContext& context, int offset) const override {
        context.painter.DrawInventoryItem(offset, asset_);
    }

    const Asset& GetAsset() const { return asset_; }
private:
    std::string key_name_;
    Asset asset_;
};

class InventoryWater : public InventoryObject {
public:
    InventoryWater(const GameContext& context) {
        asset_ = context.asset_loader.LoadSprite("inventory", "drop");
    }

    bool IsWater() const override {
        return true;
    }

    void Draw(DrawContext& context, int offset) const override {
        context.painter.DrawInventoryItem(offset, asset_);
    }

private:
    Asset asset_;
};


class Inventory {
public:
    // Добавить предмет в инвентарь.
    bool Store(const std::shared_ptr<InventoryObject>& object) {
        try {
            if (!object) {
                qWarning() << "Attempt to store null object";
                return false;
            }

            // Для ключей проверяем дубликаты по имени
            if (auto key = object->AsKey()) {
                if (FindItem([&key](auto item) {
                        auto existing_key = item->AsKey();
                        return existing_key && existing_key->GetName() == key->GetName();
                    })) {
                    qWarning() << "Key already exists in inventory:"
                               << QString::fromStdString(key->GetName());
                    return false;
                }
            }

            inventory_.push_back(object);
            qDebug() << "Stored item. Inventory size:" << inventory_.size();
            return true;
        } catch (const std::exception& e) {
            qCritical() << "Store failed:" << e.what();
            return false;
        }
    }

    // Проверить наличие конкретного предмета.
    bool HasItem(const InventoryObject* item) const {
        for (const auto& i : inventory_) {
            if (i.get() == item) {
                return true;
            }
        }
        return false;
    }

    // Убрать предмет из инвентаря.
    bool Remove(const InventoryObject* item) {
        auto it = std::find_if(inventory_.begin(), inventory_.end(),
                               [item](const auto& i) { return i.get() == item; });
        if (it != inventory_.end()) {
            inventory_.erase(it);
            return true;
        }
        return false;
    }

    // Найти предмет по условию.
    template<class Comp>
    InventoryObject* FindItem(Comp compare) const {
       /* auto it = std::find_if(inventory_.begin(), inventory_.end(), compare);
        if (it != inventory_.end()) {
            return it->get();
        }
        return nullptr;*/

        try {
            auto it = std::find_if(inventory_.begin(), inventory_.end(), compare);
            return it != inventory_.end() ? it->get() : nullptr;
        } catch (const std::exception& e) {
            qCritical() << "FindItem failed:" << e.what();
            return nullptr;
        }
    }

    const auto& GetItems() const {
        return inventory_;
    }

private:
    std::vector<std::shared_ptr<InventoryObject>> inventory_;
};
