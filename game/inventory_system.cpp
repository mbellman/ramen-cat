#include "inventory_system.h"

void InventorySystem::collectItem(GmContext* context, InventoryItem& item) {
  float time = get_scene_time();

  if (item.count == 0) {
    item.firstCollectionTime = time;
  }

  item.count++;
  item.lastCollectionTime = time;
}