#include "Serialize.h"

#include "Creature.h"
#include "Item.h"
#include "Geometry.h"

void ISerializer::srz_vec2(Vec2& v) { srz_value(v); }
void ISerializer::srz_vec3(Vec3& v) { srz_value(v); }
void ISerializer::srz_box2(Box2& b) { srz_value(b); }

void ISerializer::srz_name_hash(NameHash& h) { srz_value(h); }
void ISerializer::srz_creature_handle(Creature::Handle& h) { srz_value(h); }
void ISerializer::srz_item_handle(Item::Handle& h) { srz_value(h); }
