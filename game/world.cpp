#include "world.h"
#include "game_meshes.h"
#include "collisions.h"
#include "game_constants.h"
#include "macros.h"

using namespace Gamma;

// @temporary
struct Platform {
  Vec3f position;
  Vec3f scale;
  Vec3f rotation;
  Vec3f color;
};

internal void loadCollisionPlanes(GmContext* context, GameState& state, const std::string& levelName) {
  u64 start = Gm_GetMicroseconds();

  // @todo eventually store as binary data
  auto worldData = Gm_LoadFileContents("./game/levels/" + levelName + "/data_collision_planes.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  state.collisionPlanes.clear();
  objects("platform").reset();

  for (auto& line : lines) {
    if (line.size() == 0) {
      continue;
    }

    auto parts = Gm_SplitString(line, ",");
    auto& platform = create_object_from("platform");

    #define df(n) stof(parts[n])
    #define di(n) stoi(parts[n])

    platform.position = Vec3f(df(0), df(1), df(2));
    platform.scale = Vec3f(df(3), df(4), df(5));
    platform.rotation = Quaternion(df(6), df(7), df(8), df(9));
    platform.color = pVec4(di(10), di(11), di(12));

    commit(platform);

    Collisions::addObjectCollisionPlanes(platform, state.collisionPlanes);
  }

  Console::log("Loaded collision planes in", Gm_GetMicroseconds() - start, "us");
}

internal void loadWorldObjects(GmContext* context, const std::string& levelName) {
  u64 start = Gm_GetMicroseconds();

  // @todo eventually store as binary data
  auto worldData = Gm_LoadFileContents("./game/levels/" + levelName + "/data_world_objects.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  std::string meshName;

  for (u32 i = 0; i < lines.size(); i++) {
    auto& line = lines[i];

    if (line.size() == 0) {
      continue;
    }

    if (line[0] == '@') {
      meshName = line.substr(1);
    } else {
      auto parts = Gm_SplitString(line, ",");
      auto& object = create_object_from(meshName);

      #define df(n) stof(parts[n])
      #define di(n) stoi(parts[n])

      object.position = Vec3f(df(0), df(1), df(2));
      object.scale = Vec3f(df(3), df(4), df(5));
      object.rotation = Quaternion(df(6), df(7), df(8), df(9));
      object.color = pVec4(di(10), di(11), di(12));

      commit(object);
    }
  }

  Console::log("Loaded world objects in", Gm_GetMicroseconds() - start, "us");
}

internal void loadLights(GmContext* context, const std::string& levelName) {
  u64 start = Gm_GetMicroseconds();

  // @todo eventually store as binary data
  auto worldData = Gm_LoadFileContents("./game/levels/" + levelName + "/data_lights.txt");
  auto lines = Gm_SplitString(worldData, "\n");

  for (auto& line : lines) {
    if (line.size() == 0) {
      continue;
    }

    auto parts = Gm_SplitString(line, ",");
    auto& light = create_light((LightType)stoi(parts[0]));

    #define df(n) stof(parts[n])

    light.position = Vec3f(df(1), df(2), df(3));
    light.radius = df(4);
    light.color = Vec3f(df(5), df(6), df(7));
    light.power = df(8);
    light.direction = Vec3f(df(9), df(10), df(11));
    light.fov = df(12);
  }

  Console::log("Loaded lights in", Gm_GetMicroseconds() - start, "us");
}

internal void loadNpcData(GmContext* context, GameState& state, const std::string& levelName) {
  // @todo eventually store as binary data
  auto npcDataContents = Gm_LoadFileContents("./game/levels/" + levelName + "/data_npcs.txt");
  auto lines = Gm_SplitString(npcDataContents, "\n");

  // @temporary
  u32 i = 0;

  // @temporary
  while (i < lines.size()) {
    if (lines[i][0] == '@') {
      NonPlayerCharacter npc;

      // @todo parse NPC @type

      i++;

      npc.position = Gm_ParseVec3f(lines[i]);

      i++;
  
      std::string dialogueLine;

      while (i < lines.size() && lines[i][0] != '@') {
        auto line = lines[i++];

        if (line[0] == '-') {
          npc.dialogue.push_back(dialogueLine);

          dialogueLine = "";

          continue;
        }

        if (dialogueLine.size() > 0) {
          dialogueLine += '\n';
        }

        dialogueLine += line;
      }

      state.npcs.push_back(npc);
    } else {
      i++;
    }
  }

  // @temporary
  for (auto& npc : state.npcs) {
    auto& object = create_object_from("npc");

    object.position = npc.position;
    object.color = Vec3f(1.f, 0, 1.f);
    object.scale = Vec3f(NPC_RADIUS, NPC_HEIGHT, NPC_RADIUS);

    commit(object);
  }
}

internal void loadEntityData(GmContext* context, GameState& state, const std::string& levelName) {
  // @todo eventually store as binary data
  auto entityDataContents = Gm_LoadFileContents("./game/levels/" + levelName + "/data_entities.txt");
  auto lines = Gm_SplitString(entityDataContents, "\n");

  // @temporary
  std::string entityName;

  // @temporary
  for (u32 i = 0; i < lines.size(); i++) {
    auto& line = lines[i];

    if (line.size() == 0) {
      continue;
    }

    if (line[0] == '@') {
      entityName = line.substr(1);
    } else if (entityName == "slingshot") {
      auto parts = Gm_SplitString(line, ",");

      Vec3f position = Vec3f(stof(parts[0]), stof(parts[1]), stof(parts[2]));
      float xzVelocity = stof(parts[3]);
      float yVelocity = stof(parts[4]);
      float initialRotation = stof(parts[5]);

      state.slingshots.push_back({
        .position = position,
        .xzVelocity = xzVelocity,
        .yVelocity = yVelocity,
        .initialRotation = initialRotation
      });
    }
  }

  // @temporary
  for (auto& slingshot : state.slingshots) {
    auto& object = create_object_from("slingshot");

    object.position = slingshot.position;
    object.scale = Vec3f(60.f);
    object.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), slingshot.initialRotation);
    object.color = DEFAULT_SLINGSHOT_COLOR;

    commit(object);
  }
}

internal void unloadCurrentLevel(GmContext* context, GameState& state) {
  Gm_ResetScene(context);

  // @todo properly reset all game state
  state.collisionPlanes.clear();
  state.npcs.clear();
  state.slingshots.clear();
  state.jetstreams.clear();
  state.initialMovingObjects.clear();

  #if GAMMA_DEVELOPER_MODE
    state.lastSolidGroundPositions.clear();
  #endif
}

internal void loadLevel(GmContext* context, GameState& state, const std::string& levelName) {
  // Create meshes
  // @todo create meshes for the current level only
  {
    for (auto& asset : GameMeshes::meshAssets) {
      add_mesh(asset.name, asset.maxInstances, asset.create());

      auto& mesh = *mesh(asset.name);
      auto& attributes = asset.attributes;

      // @todo handle all mesh attributes
      mesh.type = attributes.type;
      mesh.foliage = attributes.foliage;
      mesh.texture = attributes.texture;
      mesh.normals = attributes.normals;
      mesh.maxCascade = attributes.maxCascade;
      mesh.canCastShadows = attributes.canCastShadows;
      mesh.emissivity = attributes.emissivity;
      mesh.roughness = attributes.roughness;
      mesh.silhouette = attributes.silhouette;
    }

    for (auto& asset : GameMeshes::dynamicMeshPieces) {
      add_mesh(asset.name, asset.maxInstances, asset.create());

      auto& mesh = *mesh(asset.name);
      auto& attributes = asset.attributes;

      // @todo handle all mesh attributes
      mesh.type = attributes.type;
      mesh.foliage = attributes.foliage;
      mesh.texture = attributes.texture;
      mesh.normals = attributes.normals;
      mesh.maxCascade = attributes.maxCascade;
      mesh.canCastShadows = attributes.canCastShadows;
      mesh.emissivity = attributes.emissivity;
      mesh.roughness = attributes.roughness;
      mesh.silhouette = attributes.silhouette;
    }
  }

  // @temporary
  // @todo allow non-serializable lights to be defined in a level parameters file
  {
    auto& light = create_light(LightType::DIRECTIONAL_SHADOWCASTER);

    light.direction = Vec3f(-0.2f, -1.f, -1.f);
    light.color = Vec3f(1.f);
    light.serializable = false;

    save_light("scene-light", &light);
  }

  loadCollisionPlanes(context, state, levelName);
  loadWorldObjects(context, levelName);
  loadLights(context, levelName);
  loadNpcData(context, state, levelName);
  loadEntityData(context, state, levelName);

  World::rebuildDynamicMeshes(context);

  state.currentLevelName = levelName;
}

internal void rebuildDynamicStaircases(GmContext* context) {
  objects("stair-step").reset();

  const auto sidePoints = {
    Vec3f(1.f, 0, 0),
    Vec3f(-1.f, 0, 0),
    Vec3f(0, 1.f, 0),
    Vec3f(0, -1.f, 0),
    Vec3f(0, 0, 1.f),
    Vec3f(0, 0, -1.f)
  };

  std::vector<Vec3f> t_points;

  for (auto& staircase : objects("staircase")) {
    auto& scale = staircase.scale;
    auto rotation = staircase.rotation.toMatrix4f();
    Vec3f rotationDirectionXz = staircase.rotation.getDirection().xz();
    float yRotation = -1.f * atan2f(rotationDirectionXz.z, rotationDirectionXz.x);

    t_points.clear();

    // Determine the transformed staircase bounds
    {
      for (auto& point : sidePoints) {
        t_points.push_back(staircase.position + (rotation * (scale * point)).toVec3f());
      }
    }

    Vec3f start;
    Vec3f end;

    // Establish the start and end of the staircase
    {
      float furthest = 0.f;

      for (auto& t_point : t_points) {
        for (auto& t_point2 : t_points) {
          float distance = (t_point2 - t_point).magnitude();

          if (distance > furthest) {
            start = t_point;
            end = t_point2;

            furthest = distance;
          }
        }
      }
    }

    // Build the staircase steps
    {
      u32 totalSteps = u32((start - end).magnitude() / 30.f);

      for (u32 i = 0; i < totalSteps; i++) {
        auto& step = create_object_from("stair-step");

        step.position = Vec3f::lerp(start, end, i / float(totalSteps));
        step.color = Vec3f(0.f);
        step.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yRotation);

        // Scale the steps to the width of the staircase platform
        if (staircase.scale.x > staircase.scale.z) {
          step.scale = Vec3f(staircase.scale.z, 3.f, 10.f);
        } else {
          step.scale = Vec3f(10.f, 3.f, staircase.scale.x);
        }

        commit(step);
      }
    }
  }

  #if GAMMA_DEVELOPER_MODE
    u16 totalStairSteps = objects("stair-step").totalActive();

    Console::log("Generated", std::to_string(totalStairSteps), "stair steps");
  #endif
}

internal void rebuildLamppostLights(GmContext* context) {
  objects("japanese-lamppost-light").reset();

  for (auto& post : objects("japanese-lamppost")) {
    auto& light = create_object_from("japanese-lamppost-light");

    light.position = post.position + Vec3f(0, 0.78f, 0) * post.scale.y;
    light.scale = post.scale;
    light.rotation = post.rotation;
    light.color = Vec3f(1.f, 0.9f, 0.75f);

    commit(light);
  }

  #if GAMMA_DEVELOPER_MODE
    u16 totalLights = objects("japanese-lamppost-light").totalActive();

    Console::log("Generated", std::to_string(totalLights), "lamppost lights");
  #endif
}

internal void rebuildElectricalPoleWires(GmContext* context) {
  objects("wire").reset();

  for (auto& pole : objects("electrical-pole")) {
    for (auto& p : objects("electrical-pole")) {
      if (
        p._record.id == pole._record.id &&
        p._record.generation == pole._record.generation
      ) {
        continue;
      }

      float distance = (pole.position - p.position).magnitude();
      float yDistance = pole.position.y - p.position.y;

      // Only generate wires for poles close enough to one another,
      // and within a certain y distance threshold
      if (distance < 2000.f && yDistance > 0.f && yDistance < 500.f) {
        u8 totalWirePieces = 10;
        Vec3f start = pole.position + Vec3f(0, pole.scale.y, 0);
        Vec3f end = p.position + Vec3f(0, p.scale.y, 0);
        float sagDistance = distance / 10.f;

        std::vector<Vec3f> points;

        // Define a discrete set of points forming the wire curve
        for (u8 i = 0; i <= totalWirePieces; i++) {
          float alpha = float(i) / float(totalWirePieces);
          float sag = (1.f - powf(alpha * 2.f - 1.f, 2)) * sagDistance;
          Vec3f point = Vec3f::lerp(start, end, alpha) - Vec3f(0, sag, 0);

          points.push_back(point);
        }

        // Create wire segments connecting the wire points
        for (u8 i = 0; i < points.size() - 1; i++) {
          auto& currentPoint = points[i];
          auto& nextPoint = points[i + 1];
          Vec3f path = nextPoint - currentPoint;
          float distance = path.magnitude();

          // Calculate the wire rotation (pitch + yaw)
          float yaw = atan2f(path.x, path.z);

          // Rotate the path onto the y/z plane so we can
          // calculate the pitch as a function of y/z
          path.z = path.x * sinf(yaw) + path.z * cosf(yaw);

          float pitch = atan2f(path.y, path.z);

          // Create the individual wire segment
          {
            auto& wire = create_object_from("wire");

            wire.position = (currentPoint + nextPoint) / 2.f;
            wire.scale = Vec3f(3.f, 3.f, distance / 2.f);
            wire.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yaw);
            wire.rotation *= Quaternion::fromAxisAngle(wire.rotation.getLeftDirection(), pitch);
            wire.color = Vec3f(0.1f);

            commit(wire);
          }
        }
      }
    }
  }

  #if GAMMA_DEVELOPER_MODE
    u16 totalWires = objects("wire").totalActive();

    Console::log("Generated", std::to_string(totalWires), "wire pieces");
  #endif
}

internal void rebuildDynamicBuildings(GmContext* context) {
  objects("building-1-body").reset();
  objects("building-1-frame").reset();

  for (auto& building : objects("building-1")) {
    auto& body = create_object_from("building-1-body");
    auto& frame = create_object_from("building-1-frame");

    body.position = building.position;
    body.scale = building.scale;
    body.rotation = building.rotation;
    body.color = building.color;

    frame.position = building.position;
    frame.scale = building.scale;
    frame.rotation = building.rotation;
    frame.color = Vec3f(1.f);

    commit(body);
    commit(frame);
  }
}

void World::initializeGameWorld(GmContext* context, GameState& state) {
  context->scene.zNear = 5.f;
  context->scene.zFar = 50000.f;

  // Global meshes
  {
    add_mesh("platform", 1000, Mesh::Cube());
    mesh("platform")->disabled = true;

    add_mesh("player", 1, Mesh::Model("./game/assets/cat.obj"));
    mesh("player")->roughness = 0.9f;

    add_mesh("ocean", 1, Mesh::Disc(12));
    add_mesh("ocean-floor", 1, Mesh::Disc(12));

    mesh("ocean")->type = MeshType::WATER;
    mesh("ocean")->canCastShadows = false;
    mesh("ocean-floor")->canCastShadows = false;

    add_mesh("npc", 100, Mesh::Cube());

    add_mesh("slingshot", 100, Mesh::Model("./game/assets/slingshot.obj"));
    mesh("slingshot")->roughness = 0.9f;
  }

  auto& ocean = create_object_from("ocean");

  ocean.position = Vec3f(0, OCEAN_PLANE_Y_POSITION, 0);
  ocean.scale = Vec3f(50000.f, 1.f, 50000.f);

  commit(ocean);

  auto& oceanFloor = create_object_from("ocean-floor");

  oceanFloor.position = Vec3f(0, OCEAN_PLANE_Y_POSITION - 500.f, 0);
  oceanFloor.scale = Vec3f(50000.f, 1.f, 50000.f);
  oceanFloor.color = Vec3f(0.3f, 0.7f, 0.9f);

  commit(oceanFloor);

  auto& player = create_object_from("player");

  player.scale = Vec3f(PLAYER_RADIUS);

  // @todo make spawn position/direction configurable based on level
  // umimura-alpha
  // player.position = Vec3f(-30.f, 243.f, 2200.f);

  // umimura
  player.position = Vec3f(-670.f, 4375.f, 2575.f);

  player.color = Vec3f(1.f, 0.4f, 0.4f);

  state.direction = Vec3f(0, 0, -1.f);

  commit(player);

  // Load world data
  {
    // unloadCurrentLevel(context, state);
    loadLevel(context, state, "umimura");

    // Hide dynamic mesh placeholders
    for (auto& asset : GameMeshes::meshAssets) {
      if (asset.dynamic) {
        mesh(asset.name)->disabled = true;
      }
    }
  }
}

void World::rebuildDynamicMeshes(GmContext* context) {
  rebuildDynamicStaircases(context);
  rebuildLamppostLights(context);
  rebuildElectricalPoleWires(context);
  rebuildDynamicBuildings(context);
}