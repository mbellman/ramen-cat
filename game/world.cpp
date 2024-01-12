#include "world.h"
#include "game_meshes.h"
#include "collisions.h"
#include "game_constants.h"
#include "editor.h"
#include "macros.h"

using namespace Gamma;

// @temporary
struct Platform {
  Vec3f position;
  Vec3f scale;
  Vec3f rotation;
  Vec3f color;
};

internal void loadStaticCollisionPlanes(GmContext* context, GameState& state, const std::string& levelName) {
  u64 start = Gm_GetMicroseconds();

  // @todo eventually store as binary data
  auto worldData = Gm_LoadFileContents("./game/levels/" + levelName + "/data_collision_planes.txt");
  auto lines = Gm_SplitString(worldData, "\n");

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

internal void loadWorldObjects(GmContext* context, GameState& state, const std::string& levelName) {
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

    // Keep track of the original light power figure for serialization
    light.basePower = light.power;
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
  for (auto& asset : GameMeshes::meshAssets) {
    mesh(asset.name)->objects.reset();
  }

  for (auto& asset : GameMeshes::dynamicMeshPieces) {
    mesh(asset.name)->objects.reset();
  }

  auto& scene = context->scene;
  u32 lightIndex = 0;

  // @todo remove all serializable lights from light store

  while (lightIndex < scene.lights.size()) {
    auto* light = scene.lights[lightIndex];

    if (light->serializable) {
      if (
        light->type == LightType::DIRECTIONAL_SHADOWCASTER ||
        light->type == LightType::POINT_SHADOWCASTER ||
        light->type == LightType::SPOT_SHADOWCASTER
      ) {
        context->renderer->destroyShadowMap(light);
      }

      Gm_VectorRemove(scene.lights, light);

      delete light;
    } else {
      lightIndex++;
    }
  }

  // @todo only do this in developer mode
  Editor::resetGameEditor();

  state.collisionPlanes.clear();
  state.npcs.clear();
  state.slingshots.clear();
  state.jetstreams.clear();
  state.initialMovingObjects.clear();

  state.velocity = Vec3f(0.f);
  state.currentPitch = 0.f;
  state.currentYaw = 0.f;

  #if GAMMA_DEVELOPER_MODE
    state.lastSolidGroundPositions.clear();
  #endif
}

internal void copyMeshAttributes(Mesh& mesh, const MeshAttributes& attributes) {
  mesh.type = attributes.type;
  mesh.texture = attributes.texture;
  mesh.normals = attributes.normals;
  mesh.maxCascade = attributes.maxCascade;
  mesh.animation = attributes.animation;
  mesh.emissivity = attributes.emissivity;
  mesh.roughness = attributes.roughness;
  mesh.canCastShadows = attributes.canCastShadows;
  mesh.useMipmaps = attributes.useMipmaps;
  mesh.useCloseTranslucency = attributes.useCloseTranslucency;
  mesh.useXzPlaneTexturing = attributes.useXzPlaneTexturing;
}

internal void loadGameMeshes(GmContext* context, GameState& state) {
  for (auto& asset : GameMeshes::meshAssets) {
    add_mesh(asset.name, asset.maxInstances, asset.create());

    auto& mesh = *mesh(asset.name);
    auto& attributes = asset.attributes;

    copyMeshAttributes(mesh, attributes);
  }

  for (auto& asset : GameMeshes::dynamicMeshPieces) {
    add_mesh(asset.name, asset.maxInstances, asset.create());

    auto& mesh = *mesh(asset.name);
    auto& attributes = asset.attributes;

    copyMeshAttributes(mesh, attributes);
  }
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

internal void rebuildStreetlampLights(GmContext* context) {
  objects("streetlamp-light").reset();
  objects("streetlamp-frame").reset();

  for (auto& lamp : objects("streetlamp")) {
    auto& lampLight = create_object_from("streetlamp-light");
    auto& frame = create_object_from("streetlamp-frame");

    Vec3f offset = Vec3f(lamp.scale.x * 0.17f, lamp.scale.y * 0.74f, 0);
    Vec3f r_offset = lamp.rotation.toMatrix4f().transformVec3f(offset);

    lampLight.position = lamp.position + r_offset;
    lampLight.scale = Vec3f(lamp.scale.x, lamp.scale.y * 1.2f, lamp.scale.z) * 0.12f;
    lampLight.rotation = lamp.rotation;
    lampLight.color = Vec3f(1.f, 0.8f, 0.6f);

    frame.position = lamp.position;
    frame.scale = lamp.scale;
    frame.rotation = lamp.rotation;
    frame.color = Vec3f(0.7f);

    commit(lampLight);
    commit(frame);
  }
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

// @todo allow dynamic meshes to define their own parts,
// and behavior for generating parts
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

  objects("yuki-building-1-frame").reset();

  for (auto& building : objects("yuki-building-1")) {
    auto& frame = create_object_from("yuki-building-1-frame");

    frame.position = building.position;
    frame.scale = building.scale;
    frame.rotation = building.rotation;
    frame.color = Vec3f(0.5f, 0.4f, 0.3f);

    commit(frame);
  }

  objects("yuki-building-2-frame").reset();

  for (auto& building : objects("yuki-building-2")) {
    auto& frame = create_object_from("yuki-building-2-frame");

    frame.position = building.position;
    frame.scale = building.scale;
    frame.rotation = building.rotation;
    frame.color = Vec3f(0.2f, 0.15f, 0.15f);

    commit(frame);
  }

  objects("yuki-building-3-frame").reset();

  for (auto& building : objects("yuki-building-3")) {
    auto& frame = create_object_from("yuki-building-3-frame");

    frame.position = building.position;
    frame.scale = building.scale;
    frame.rotation = building.rotation;
    frame.color = Vec3f(0.3f, 0.25f, 0.2f);

    commit(frame);
  }

  objects("b1-base").reset();
  objects("b1-windows").reset();

  for (auto& b : objects("b1")) {
    auto& base = create_object_from("b1-base");
    auto& windows = create_object_from("b1-windows");

    base.position = windows.position = b.position;
    base.scale = windows.scale = b.scale;
    base.rotation = windows.rotation = b.rotation;

    windows.color = Vec3f(0.5f, 0.75f, 1.f);

    commit(base);
    commit(windows);
  }

  objects("b2-base").reset();
  objects("b2-levels").reset();
  objects("b2-columns").reset();
  objects("b2-windows").reset();

  for (auto& b : objects("b2")) {
    auto& base = create_object_from("b2-base");
    auto& levels = create_object_from("b2-levels");
    auto& columns = create_object_from("b2-columns");
    auto& windows = create_object_from("b2-windows");

    base.position = levels.position = columns.position = windows.position = b.position;
    base.scale = levels.scale = columns.scale = windows.scale = b.scale;
    base.rotation = levels.rotation = columns.rotation = windows.rotation = b.rotation;

    base.color = Vec3f::lerp(b.color.toVec3f(), Vec3f(1.f), 0.5f);
    levels.color = Vec3f(1.f);
    columns.color = b.color;
    windows.color = Vec3f(0.5f, 0.75f, 1.f);

    commit(base);
    commit(levels);
    commit(columns);
    commit(windows);
  }

  objects("b3-base").reset();
  objects("b3-levels").reset();
  objects("b3-columns").reset();

  for (auto& b : objects("b3")) {
    auto& base = create_object_from("b3-base");
    auto& levels = create_object_from("b3-levels");
    auto& columns = create_object_from("b3-columns");

    base.position = levels.position = columns.position = b.position;
    base.scale = levels.scale = columns.scale = b.scale;
    base.rotation = levels.rotation = columns.rotation = b.rotation;

    base.color = Vec3f::lerp(b.color.toVec3f(), Vec3f(1.f), 0.5f);
    levels.color = Vec3f(1.f);
    columns.color = b.color;

    commit(base);
    commit(levels);
    commit(columns);
  }
}

internal void rebuildAcUnitFans(GmContext* context) {
  objects("ac-fan").reset();

  for (auto& unit : objects("ac-unit")) {
    auto& fan = create_object_from("ac-fan");
    Vec3f horizontalOffset = unit.rotation.getLeftDirection().invert() * unit.scale.x * 0.38f;
    Vec3f forwardOffset = unit.rotation.getDirection().invert() * unit.scale.z * 0.2f;

    fan.position = unit.position + horizontalOffset + forwardOffset;
    fan.scale = unit.scale * 0.4f;
    fan.rotation = unit.rotation;
    fan.color = Vec3f(0.1f);

    commit(fan);
  }
}

internal void rebuildWindTurbines(GmContext* context) {
  objects("wind-turbine").reset();

  for (auto& base : objects("wind-turbine-base")) {
    auto& turbine = create_object_from("wind-turbine");
    Vec3f verticalOffset = base.rotation.getUpDirection() * base.scale.z * 1.13f;
    Vec3f forwardOffset = base.rotation.getDirection() * base.scale.z * 0.1f;

    turbine.position = base.position + verticalOffset + forwardOffset;
    turbine.scale = base.scale;
    turbine.rotation = base.rotation;
    turbine.color = Vec3f(1.f);

    commit(turbine);
  }
}

internal void applyLevelSettings_UmimuraAlpha(GmContext* context, GameState& state) {
  auto& player = get_player();

  player.position = Vec3f(-30.f, 243.f, 2200.f);

  context->scene.zFar = 50000.f;

  Gm_EnableFlags(GammaFlags::RENDER_HORIZON_ATMOSPHERE);

  mesh("ocean")->disabled = false;
  mesh("ocean-floor")->disabled = false;
}

internal void applyLevelSettings_Umimura(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::RENDER_HORIZON_ATMOSPHERE);

  auto& player = get_player();

  player.position = Vec3f(-670.f, 4400.f, 2575.f);

  context->scene.zFar = 50000.f;

  {
    add_mesh("market-particles", 1000, Mesh::Particles(true));

    auto& particles = mesh("market-particles")->particles;

    particles.spawn = Vec3f(270.f, 1500.f, -3500.f);
    particles.spread = 1500.f;
    particles.medianSize = 1.5f;
    particles.sizeVariation = 0.5f;
    particles.medianSpeed = 1.f;
    particles.deviation = 20.f;
  }

  mesh("ocean")->disabled = false;
  mesh("ocean-floor")->disabled = false;
}

internal void applyLevelSettings_Overworld(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::RENDER_HORIZON_ATMOSPHERE);

  auto& player = get_player();

  player.position = Vec3f(7588.f, 29412.f, 23176.f);

  context->scene.zFar = 200000.f;

  mesh("ocean")->disabled = false;
  mesh("ocean-floor")->disabled = false;

  auto& ocean = mesh("ocean")->objects[0];
  auto& oceanFloor = mesh("ocean-floor")->objects[0];

  ocean.scale = Vec3f(200000.f, 1.f, 200000.f);
  oceanFloor.scale = Vec3f(200000.f, 1.f, 200000.f);

  commit(ocean);
  commit(oceanFloor);
}

internal void applyLevelSettings_Yukimura(GmContext* context, GameState& state) {
  Gm_DisableFlags(GammaFlags::RENDER_HORIZON_ATMOSPHERE);

  auto& player = get_player();

  player.position = Vec3f(-670.f, 140.f, -230.f);

  context->scene.zFar = 15000.f;

  mesh("ocean")->disabled = true;
  mesh("ocean-floor")->disabled = true;
}

void World::initializeGameWorld(GmContext* context, GameState& state) {
  context->scene.zNear = 5.f;
  context->scene.zFar = 50000.f;

  // Global meshes
  {
    // @todo rename static_collision_box, increase total instances
    add_mesh("platform", 1000, Mesh::Cube());
    mesh("platform")->disabled = true;

    add_mesh("dynamic_collision_box", 10000, Mesh::Cube());
    mesh("dynamic_collision_box")->disabled = true;

    add_mesh("player", 1, Mesh::Model("./game/assets/cat.obj"));
    mesh("player")->type = MeshType::DEFAULT_WITH_OCCLUSION_SILHOUETTE;
    mesh("player")->texture = "./game/assets/cat.png";
    mesh("player")->roughness = 0.9f;
    mesh("player")->emissivity = 0.3f;
    mesh("player")->useMipmaps = false;
    mesh("player")->useCloseTranslucency = true;

    add_mesh("glider", 1, Mesh::Model("./game/assets/glider.obj"));
    mesh("glider")->roughness = 0.4f;
    mesh("glider")->useCloseTranslucency = true;

    add_mesh("ocean", 1, Mesh::Disc(12));
    add_mesh("ocean-floor", 1, Mesh::Disc(12));

    mesh("ocean")->type = MeshType::OCEAN;
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
  player.color = Vec3f(1.f, 0.4f, 0.4f);

  commit(player);

  auto& glider = create_object_from("glider");

  glider.scale = Vec3f(PLAYER_RADIUS);
  glider.color = Vec3f(0.1f);

  commit(glider);

  state.direction = Vec3f(0, 0, -1.f);

  loadGameMeshes(context, state);

  // Hide dynamic mesh placeholders
  {
    for (auto& asset : GameMeshes::meshAssets) {
      if (asset.dynamic) {
        mesh(asset.name)->disabled = true;
      }
    }
  }
}

void World::rebuildDynamicMeshes(GmContext* context) {
  rebuildDynamicStaircases(context);
  rebuildStreetlampLights(context);
  rebuildElectricalPoleWires(context);
  rebuildDynamicBuildings(context);
  rebuildAcUnitFans(context);
  rebuildWindTurbines(context);
}

void World::rebuildDynamicCollisionPlanes(GmContext* context, GameState& state) {
  objects("dynamic_collision_box").reset();

  for (auto& cube : objects("cube")) {
    auto& box = create_object_from("dynamic_collision_box");

    box.position = cube.position;
    box.rotation = cube.rotation;
    box.scale = cube.scale;
    box.color = Vec3f(0.5f, 0.5f, 1.f);

    commit(box);
  }

  for (auto& box : objects("dynamic_collision_box")) {
    Collisions::addObjectCollisionPlanes(box, state.collisionPlanes);
  }
}

void World::loadLevel(GmContext* context, GameState& state, const std::string& levelName) {
  unloadCurrentLevel(context, state);

  // Apply level settings
  {
    typedef std::function<void(GmContext*, GameState&)> LevelSettingsFunction;

    std::map<std::string, LevelSettingsFunction> levelSettingsFunctionMap = {
      { "umimura-alpha", applyLevelSettings_UmimuraAlpha },
      { "umimura", applyLevelSettings_Umimura },
      { "overworld", applyLevelSettings_Overworld },
      { "yukimura", applyLevelSettings_Yukimura }
    };

    if (levelSettingsFunctionMap.find(levelName) != levelSettingsFunctionMap.end()) {
      auto applyLevelSettings = levelSettingsFunctionMap.at(levelName);
      auto& player = get_player();

      applyLevelSettings(context, state);

      state.previousPlayerPosition = player.position;
      state.levelSpawnPosition = player.position;
    }
  }

  // @temporary
  // @todo allow non-serializable lights to be defined in a level parameters file
  {
    if (context->scene.lightStore.find("scene-light") == context->scene.lightStore.end()) {
      auto& light = create_light(LightType::DIRECTIONAL_SHADOWCASTER);

      light.direction = Vec3f(-0.2f, -1.f, -1.f);
      light.color = Vec3f(1.f);
      light.serializable = false;

      save_light("scene-light", &light);
    }
  }

  loadStaticCollisionPlanes(context, state, levelName);
  loadWorldObjects(context, state, levelName);
  loadLights(context, levelName);
  loadNpcData(context, state, levelName);
  loadEntityData(context, state, levelName);

  World::rebuildDynamicMeshes(context);
  World::rebuildDynamicCollisionPlanes(context, state);

  // Save initial reference copies of moving objects
  {
    for (auto& asset : GameMeshes::meshAssets) {
      if (asset.moving) {
        for (auto& object : objects(asset.name)) {
          state.initialMovingObjects.push_back(object);
        }
      }
    }

    for (auto& asset : GameMeshes::dynamicMeshPieces) {
      if (asset.moving) {
        for (auto& object : objects(asset.name)) {
          state.initialMovingObjects.push_back(object);
        }
      }
    }
  }

  state.currentLevelName = levelName;
}