#include "world.h"
#include "game_meshes.h"
#include "collisions.h"
#include "game_constants.h"
#include "procedural_meshes.h"
#include "vehicle_system.h"
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
  // @temporary
  for (auto& person : objects("person")) {
    NonPlayerCharacter npc;

    npc.position = person.position;

    npc.dialogue.push_back("I'm a chuckster!");

    state.npcs.push_back(npc);
  }


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
}

internal void unloadCurrentLevel(GmContext* context, GameState& state) {
  for (auto& asset : GameMeshes::meshAssets) {
    mesh(asset.name)->objects.reset();

    for (auto& piece : asset.pieces) {
      mesh(piece.name)->objects.reset();
    }
  }

  for (auto& asset : GameMeshes::proceduralMeshParts) {
    mesh(asset.name)->objects.reset();
  }

  // @todo remove
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
  mesh.animation = attributes.animation;
  mesh.particles = attributes.particles;
  mesh.ocean = attributes.ocean;
  mesh.texture = attributes.texture;
  mesh.normals = attributes.normals;
  mesh.maxCascade = attributes.maxCascade;
  mesh.emissivity = attributes.emissivity;
  mesh.roughness = attributes.roughness;
  mesh.canCastShadows = attributes.canCastShadows;
  mesh.useLowestLevelOfDetailForShadows = attributes.useLowestLevelOfDetailForShadows;
  mesh.useMipmaps = attributes.useMipmaps;
  mesh.useCloseTranslucency = attributes.useCloseTranslucency;
  mesh.useXzPlaneTexturing = attributes.useXzPlaneTexturing;
}

// @todo create a loadGameMeshFromAsset() function to simplify this
internal void loadGameMeshes(GmContext* context, GameState& state) {
  GameMeshes::loadAllMeshAssets();

  for (auto& asset : GameMeshes::meshAssets) {
    add_mesh(asset.name, asset.maxInstances, asset.create());

    auto& mesh = *mesh(asset.name);
    auto& attributes = asset.attributes;

    copyMeshAttributes(mesh, attributes);

    for (auto& piece : asset.pieces) {
      add_mesh(piece.name, piece.maxInstances, piece.create());

      auto& pieceMesh = *mesh(piece.name);
      auto& attributes = piece.attributes;

      copyMeshAttributes(pieceMesh, attributes);

      // Inherit additional properties from the base mesh
      pieceMesh.maxCascade = mesh.maxCascade;
      pieceMesh.canCastShadows = mesh.canCastShadows;
    }
  }

  for (auto& asset : GameMeshes::proceduralMeshParts) {
    add_mesh(asset.name, asset.maxInstances, asset.create());

    auto& mesh = *mesh(asset.name);
    auto& attributes = asset.attributes;

    copyMeshAttributes(mesh, attributes);
  }

  // @todo remove
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
    float yRotation = -1.f * atan2f(rotationDirectionXz.z, rotationDirectionXz.x) - Gm_PI * 0.5f;

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

        float jitter = Gm_Modf(step.position.x * 1.23f + step.position.z * 4.56f, 1.f) * 0.1f - 0.05f;

        step.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), yRotation + jitter);
        step.color = Vec3f(0.7f, 0.9f, 1.f);

        // Scale the steps to the width of the staircase platform
        if (staircase.scale.x > staircase.scale.z) {
          step.scale = Vec3f(70.f, 70.f, staircase.scale.z);
        } else {
          step.scale = Vec3f(staircase.scale.x, 70.f, 70.f);
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

// @todo define as pieces
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

// @todo move to a common file
#define is_same_object(a, b) a._record.id == b._record.id && a._record.generation == b._record.generation

internal void rebuildElectricalWires(GmContext* context) {
  // Electrical pole wires
  for (auto& p1 : objects("electrical-pole")) {
    for (auto& p2 : objects("electrical-pole")) {
      if (is_same_object(p1, p2)) continue;

      float distance = (p1.position - p2.position).magnitude();
      float yDistance = p1.position.y - p2.position.y;

      // Only generate wires for poles close enough to one another,
      // and within a certain y distance threshold
      if (distance < 2000.f && yDistance > 0.f && yDistance < 500.f) {
        Vec3f start = p1.position + Vec3f(0, p1.scale.y, 0);
        Vec3f end = p2.position + Vec3f(0, p2.scale.y, 0);

        ProceduralMeshes::buildWireFromStartToEnd(context, start, end, 2.f, Vec3f(0.1f));
      }
    }
  }

  // Wood electrical pole wires
  for (auto& p1 : objects("wood-electrical-pole")) {
    for (auto& p2 : objects("wood-electrical-pole")) {
      if (is_same_object(p1, p2)) {
        continue;
      }

      float distance = (p1.position - p2.position).magnitude();
      float yDistance = p1.position.y - p2.position.y;

      // Only generate wires for poles close enough to one another,
      // and within a certain y distance threshold
      if (distance < 20000.f && yDistance > 0.f && yDistance < 4000.f) {
        // Wire 1
        {
          Vec3f start = p1.position + Vec3f(0, p1.scale.y * 1.2f, 0) + (p1.rotation.toMatrix4f() * Vec3f(0, 0, p1.scale.z * 0.8f)).toVec3f();
          Vec3f end = p2.position + Vec3f(0, p2.scale.y * 1.2f, 0) + (p2.rotation.toMatrix4f() * Vec3f(0, 0, p2.scale.z * 0.8f)).toVec3f();

          ProceduralMeshes::buildWireFromStartToEnd(context, start, end, 10.f, Vec3f(0.7f));
        }

        // Wire 2
        {
          Vec3f start = p1.position + Vec3f(0, p1.scale.y * 1.63f, 0) + (p1.rotation.toMatrix4f() * Vec3f(0, 0, p1.scale.z * 0.8f)).toVec3f();
          Vec3f end = p2.position + Vec3f(0, p2.scale.y * 1.63f, 0) + (p2.rotation.toMatrix4f() * Vec3f(0, 0, p2.scale.z * 0.8f)).toVec3f();

          ProceduralMeshes::buildWireFromStartToEnd(context, start, end, 10.f, Vec3f(0.7f));
        }
      }
    }
  }
}

internal void rebuildMiniFlagWires(GmContext* context) {
  const static auto FLAG_COLORS = {
    Vec3f(1.f, 0.5f, 0.2f),
    Vec3f(1.f, 0.2f, 0.1f),
    Vec3f(0.2f, 0.5f, 1.f),
    Vec3f(0.2f, 1.f, 0.5f)
  };

  objects("mini-flag").reset();

  for (auto& s1 : objects("flag-wire-spawn")) {
    for (auto& s2 : objects("flag-wire-spawn")) {
      if (is_same_object(s1, s2)) {
        continue;
      }

      float distance = (s1.position - s2.position).magnitude();
      float yDistance = s1.position.y - s2.position.y;

      if (distance < 3000.f && yDistance > 0.f && yDistance < 1000.f) {
        auto& start = s1.position;
        auto& end = s2.position;

        // Construct the wire
        ProceduralMeshes::buildWireFromStartToEnd(context, start, end, 4.f, Vec3f(0.5f));

        // Add mini flag decorations
        u8 totalWirePieces = 10;
        Vec3f direction = end - start;
        float sagDistance = direction.magnitude() / 10.f;
        float angle = atan2f(direction.x, direction.z) + Gm_HALF_PI;

        for (u8 i = 1; i < totalWirePieces; i++) {
          float alpha = float(i) / float(totalWirePieces);
          float sag = (1.f - powf(alpha * 2.f - 1.f, 2)) * sagDistance;
          Vec3f position = Vec3f::lerp(start, end, alpha) - Vec3f(0, sag + 10.f, 0);
          u8 colorIndex = u8(Gm_Modf(position.x + position.z, 4.f));

          auto& flag = create_object_from("mini-flag");
          float sizeVariance = Gm_Modf(position.x, 10.f);

          flag.position = position - Vec3f(0, 55.f, 0) - Vec3f(0, sizeVariance, 0);
          flag.scale = Vec3f(60.f - sizeVariance, 60.f + sizeVariance * 2.f, 60.f);
          flag.color = *(FLAG_COLORS.begin() + colorIndex);
          flag.rotation = Quaternion::fromAxisAngle(Vec3f(0, 1.f, 0), angle);

          commit(flag);
        }
      }
    }
  }

  #if GAMMA_DEVELOPER_MODE
    Console::log("Generated", objects("mini-flag").totalActive(), "mini flags");
  #endif
}

// @todo create as pieces
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

  objects("wood-facade-base").reset();
  objects("wood-facade-cover").reset();

  for (auto& it : objects("wood-facade")) {
    auto& base = create_object_from("wood-facade-base");
    auto& cover = create_object_from("wood-facade-cover");

    base.position = cover.position = it.position;
    base.scale = cover.scale = it.scale;
    base.rotation = cover.rotation = it.rotation;

    base.color = Vec3f(1.f, 0.8f, 0.6f);
    cover.color = Vec3f(0.8f, 0.4f, 0.1f);

    commit(base);
    commit(cover);
  }

  objects("bridge-1-floor").reset();
  objects("bridge-1-supports").reset();
  objects("bridge-1-roof").reset();

  for (auto& bridge : objects("bridge-1")) {
    auto& floor = create_object_from("bridge-1-floor");
    auto& supports = create_object_from("bridge-1-supports");
    auto& roof = create_object_from("bridge-1-roof");

    floor.position = supports.position = roof.position = bridge.position;
    floor.scale = supports.scale = roof.scale = bridge.scale;
    floor.rotation = supports.rotation = roof.rotation = bridge.rotation;

    floor.color = Vec3f(0.75f);
    supports.color = Vec3f(1.f, 0.7f, 0.3f);
    roof.color = Vec3f(0.8f, 0.6f, 0.4f);

    commit(floor);
    commit(supports);
    commit(roof);
  }

  objects("dynamic-wave-sign").reset();

  auto invertColor = [](const Vec3f& color) {
    return Vec3f(1.f - color.x, 1.f - color.y, 1.f - color.z);
  };

  for (auto& sign : objects("wave-sign")) {
    auto& s = create_object_from("dynamic-wave-sign");
    auto& s2 = create_object_from("dynamic-wave-sign");

    s.position = sign.position;
    // @todo consider rotation
    s2.position = sign.position + Vec3f(0, sign.scale.y * 0.25f, 0);

    s.scale = sign.scale;
    s2.scale = sign.scale * Vec3f(1.f, 0.7f, 1.f);

    s.rotation = s2.rotation = sign.rotation;

    s.color = sign.color;
    s2.color = Vec3f::lerp(invertColor(sign.color.toVec3f()), Vec3f(1.f), 0.5f);

    commit(s);
    commit(s2);
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

// @todo define as pieces
internal void rebuildSignRoofs(GmContext* context) {
  objects("sign-roof-supports").reset();
  objects("sign-roof-signs").reset();

  for (auto& base : objects("sign-roof")) {
    auto& supports = create_object_from("sign-roof-supports");
    auto& signs = create_object_from("sign-roof-signs");

    supports.position = signs.position = base.position;
    supports.rotation = signs.rotation = base.rotation;
    supports.scale = signs.scale = base.scale;

    supports.color = Vec3f(0.9f, 0.6f, 0.3f);
    signs.color = base.color;

    commit(supports);
    commit(signs);
  }
}

internal void rebuildPetals(GmContext* context) {
  objects("petal").reset();

  for (auto& spawn : objects("petal-spawn")) {
    for (u16 i = 0; i < 1000; i++) {
      auto& petal = create_object_from("petal");

      petal.position = spawn.position + Vec3f(
        Gm_Randomf(-5000.f, 5000.f),
        Gm_Randomf(-5000.f, 5000.f),
        Gm_Randomf(-5000.f, 5000.f)
      );

      petal.color = Vec3f(1.f, Gm_Randomf(0.3f, 0.6f), Gm_Randomf(0.3f, 0.6f));

      commit(petal);
    }
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

  player.position = Vec3f(6400.f, 28980.f, 20530.f);

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

internal void applyLevelSettings_Zone1(GmContext* context, GameState& state) {
  Gm_EnableFlags(GammaFlags::RENDER_HORIZON_ATMOSPHERE);

  auto& player = get_player();

  player.position = Vec3f(6405.f, 27085.f, 20530.f);

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
    mesh("player")->emissivity = 0.5f;
    mesh("player")->useMipmaps = false;
    mesh("player")->useCloseTranslucency = true;

    auto& playerLight = create_light(LightType::POINT);

    playerLight.radius = 0.f;
    playerLight.serializable = false;

    save_light("player-light", &playerLight);

    add_mesh("glider", 1, Mesh::Model("./game/assets/glider.obj"));
    mesh("glider")->roughness = 0.4f;
    mesh("glider")->useCloseTranslucency = true;

    add_mesh("air-dash-target", 1, Mesh::Model("./game/assets/entities/target.obj"));
    mesh("air-dash-target")->roughness = 0.2f;
    mesh("air-dash-target")->emissivity = 0.5f;

    add_mesh("speech-bubble", 1, Mesh::Model("./game/assets/entities/speech-bubble.obj"));
    mesh("speech-bubble")->emissivity = 0.5f;
    mesh("speech-bubble")->canCastShadows = false;

    add_mesh("ocean", 1, Mesh::Disc(12));
    add_mesh("ocean-floor", 1, Mesh::Disc(12));

    mesh("ocean")->type = MeshType::OCEAN;
    mesh("ocean")->canCastShadows = false;
    mesh("ocean-floor")->canCastShadows = false;

    // @todo remove
    add_mesh("npc", 100, Mesh::Cube());

    // @todo remove
    add_mesh("slingshot", 100, Mesh::Model("./game/assets/slingshot.obj"));
    mesh("slingshot")->roughness = 0.9f;

    #if GAMMA_DEVELOPER_MODE
      add_mesh("motion_indicator", 4, Mesh::Cube());
      mesh("motion_indicator")->emissivity = 1.f;
      mesh("motion_indicator")->canCastShadows = false;

      create_object_from("motion_indicator");
      create_object_from("motion_indicator");
      create_object_from("motion_indicator");
      create_object_from("motion_indicator");
    #endif
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

  auto& target = create_object_from("air-dash-target");

  target.scale = Vec3f(0.f);
  target.color = Vec3f(1.f, 0.9f, 0.4f);

  commit(target);

  create_object_from("speech-bubble");

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
  auto start = Gm_GetMicroseconds();

  {
    for (auto& asset : GameMeshes::meshAssets) {
      for (auto& piece : asset.pieces) {
        objects(piece.name).reset();

        for (auto& source : objects(asset.name)) {
          auto& pieceObject = create_object_from(piece.name);

          pieceObject.position = source.position;
          pieceObject.scale = source.scale;
          pieceObject.rotation = source.rotation;

          if (piece.rebuild != nullptr) {
            piece.rebuild(source, pieceObject);
          }

          commit(pieceObject);
        }
      }
    }
  }

  rebuildDynamicStaircases(context);
  rebuildStreetlampLights(context);
  rebuildDynamicBuildings(context);
  rebuildAcUnitFans(context);
  rebuildWindTurbines(context);
  rebuildSignRoofs(context);
  rebuildPetals(context);

  // @todo move these to procedural_meshes.cpp
  {
    objects("wire").reset();

    rebuildElectricalWires(context);
    rebuildMiniFlagWires(context);
  }

  ProceduralMeshes::rebuildProceduralMeshes(context);

  for (auto& asset : GameMeshes::proceduralMeshParts) {
    Console::log("Created", objects(asset.name).totalActive(), asset.name, "meshes");
  }

  Console::log("Rebuilt dynamic meshes in", (Gm_GetMicroseconds() - start), "us");
}

void World::rebuildDynamicCollisionPlanes(GmContext* context, GameState& state) {
  const static Vec3f DEFAULT_COLOR = Vec3f(0.5f, 0.5f, 1.f);

  objects("dynamic_collision_box").reset();

  for (auto& cube : objects("cube")) {
    auto& box = create_object_from("dynamic_collision_box");

    box.position = cube.position;
    box.rotation = cube.rotation;
    box.scale = cube.scale;
    box.color = DEFAULT_COLOR;

    commit(box);
  }

  for (auto& staircase : objects("staircase")) {
    auto& box = create_object_from("dynamic_collision_box");

    box.position = staircase.position;
    box.rotation = staircase.rotation;
    box.scale = staircase.scale;
    box.color = DEFAULT_COLOR;

    commit(box);
  }

  for (auto& cube : objects("concrete-stack")) {
    auto& box = create_object_from("dynamic_collision_box");

    box.position = cube.position;
    box.rotation = cube.rotation;
    box.scale = cube.scale;
    box.color = DEFAULT_COLOR;

    commit(box);
  }

  for (auto& b1 : objects("b1")) {
    auto& box = create_object_from("dynamic_collision_box");

    box.position = b1.position;
    box.rotation = b1.rotation;
    box.scale = b1.scale * Vec3f(1.1f, 2.055f, 1.1f);
    box.color = DEFAULT_COLOR;

    commit(box);
  }

  for (auto& b2 : objects("b2")) {
    auto& box = create_object_from("dynamic_collision_box");
    auto& box2 = create_object_from("dynamic_collision_box");

    box.position = box2.position = b2.position;
    box.rotation = box2.rotation = b2.rotation;
    box.scale = b2.scale * Vec3f(0.925f, 1.245f, 0.79f);
    box2.scale = b2.scale * Vec3f(0.8f, 1.41f, 0.68f);
    box.color = box2.color = DEFAULT_COLOR;

    commit(box);
    commit(box2);
  }

  for (auto& b3 : objects("b3")) {
    auto& box = create_object_from("dynamic_collision_box");

    box.position = b3.position;
    box.rotation = b3.rotation;
    box.scale = b3.scale * Vec3f(1.025f, 1.345f, 1.018f);
    box.color = box.color = DEFAULT_COLOR;

    commit(box);
  }

  for (auto& box : objects("dynamic_collision_box")) {
    Collisions::addObjectCollisionPlanes(box, state.collisionPlanes);
  }

  #if GAMMA_DEVELOPER_MODE
    u16 total = objects("dynamic_collision_box").totalActive();

    Console::log("Rebuilt", std::to_string(total), "dynamic collision boxes");
  #endif
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
      { "yukimura", applyLevelSettings_Yukimura },
      { "zone-1", applyLevelSettings_Zone1 }
    };

    if (levelSettingsFunctionMap.find(levelName) != levelSettingsFunctionMap.end()) {
      auto applyLevelSettings = levelSettingsFunctionMap.at(levelName);
      auto& player = get_player();

      applyLevelSettings(context, state);

      state.previousPlayerPosition = player.position;
      state.levelSpawnPosition = player.position;
    }

    if (Gm_StringStartsWith(levelName, "zone")) {
      // @todo
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

      for (auto& piece : asset.pieces) {
        if (piece.moving) {
          for (auto& object : objects(piece.name)) {
            state.initialMovingObjects.push_back(object);
          }
        }
      }
    }

    for (auto& asset : GameMeshes::proceduralMeshParts) {
      if (asset.moving) {
        for (auto& object : objects(asset.name)) {
          state.initialMovingObjects.push_back(object);
        }
      }
    }

    // @todo remove
    for (auto& asset : GameMeshes::dynamicMeshPieces) {
      if (asset.moving) {
        for (auto& object : objects(asset.name)) {
          state.initialMovingObjects.push_back(object);
        }
      }
    }
  }

  VehicleSystem::rebuildVehicleTracks(context, state);

  state.currentLevelName = levelName;
}