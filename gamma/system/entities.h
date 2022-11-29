#pragma once

#include <functional>
#include <string>
#include <vector>

#include "math/geometry.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "system/ObjectPool.h"
#include "system/ObjLoader.h"
#include "system/packed_data.h"
#include "system/type_aliases.h"

namespace Gamma {
  enum LightType {
    POINT,
    DIRECTIONAL,
    SPOT,
    POINT_SHADOWCASTER,
    DIRECTIONAL_SHADOWCASTER,
    SPOT_SHADOWCASTER
  };

  /**
   * Light
   * -----
   *
   * Defines a light source, which affects scene illumination
   * and color/reflective properties of illuminated surfaces.
   */
  struct Light {
    Vec3f position;
    float radius = 100.0f;
    Vec3f color = Vec3f(1.0f);
    float power = 1.0f;
    Vec3f direction = Vec3f(0.0f, -1.0f, 0.0f);
    float fov = 90.0f;
    u32 type = LightType::POINT;
    bool isStatic = false;
    bool serializable = true;
    // @todo std::vector<u32> shadowMapMeshes (?)
  };

  /**
   * ObjectRecord
   * ------------
   *
   * A unique identifier for an object which allows it
   * to be looked up on the appropriate Mesh and at its
   * corresponding index, with ID checks for referential
   * integrity.
   *
   * @size 8 bytes
   */
  struct ObjectRecord {
    u16 meshIndex = 0;
    // @todo remove this and allow meshes to be 'deactivated' when freed
    u16 meshId = 0;
    // @todo u32 id with 24 bits for id and 8 for generation,
    // allowing up to ~16.77 million objects per pool
    u16 id = 0;
    u16 generation = 0;
  };

  /**
   * Object
   * ------
   *
   * Objects are derived from Meshes, defining individual
   * instances of a Mesh distributed throughout a scene,
   * each with its own transformations.
   *
   * @size 48 bytes
   */
  struct Object {
    ObjectRecord _record;
    Vec3f position;
    Vec3f scale;
    Vec3f rotation;
    pVec4 color;
  };

  /**
   * A set of Mesh types, particular to all instances of a Mesh,
   * controlling the rendering priority and properties of those
   * instances.
   */
  enum MeshType {
    /**
     * @todo description
     */
    SKYBOX = 0x00,
    /**
     * @todo description
     */
    EMISSIVE = 0x50,
    /**
     * Defines a set of GPU-accelerated particles, with custom
     * parameters for controlling particle behavior.
     */
    PARTICLE_SYSTEM = 0xA0,
    /**
     * Defines translucent Meshes which refract and lens the
     * objects and geometry behind them.
     */
    REFRACTIVE = 0xF0,
    /**
     * Defines Meshes which reflect surrounding geometry in
     * screen space, or the sky where geometry is not reflected.
     */
    REFLECTIVE = 0xFA,
    /**
     * @todo description
     */
    PROBE_REFLECTOR = 0xFB,
    /**
     * @todo description
     */
    FOLIAGE = 0xFC,
    /**
     * @todo description
     */
    WATER = 0xFD,
    /**
     * Defines standard Meshes without any unique rendering properties.
     */
    DEFAULT = 0xFF
  };

  /**
   * MeshLod
   * -------
   *
   * Defines vertices/face elements and instances particular
   * to specific Mesh levels of detail.
   */
  struct MeshLod {
    /**
     * Defines the starting face element in the LOD model.
     */
    u32 elementOffset = 0;
    /**
     * Defines the number of face elements in the LOD model.
     */
    u32 elementCount = 0;
    /**
     * Defines the starting instance in the LOD group.
     */
    u32 instanceOffset = 0;
    /**
     * Defines the number of instances in the LOD group.
     */
    u32 instanceCount = 0;
    /**
     * Defines the starting vertex in the LOD model.
     */
    u32 vertexOffset = 0;
    /**
     * Defines the number of vertices in the LOD model.
     */
    u32 vertexCount = 0;
  };

  /**
   * ParticleSystem
   * --------------
   *
   * @todo description
   */
  struct ParticleSystem {
    Vec3f spawn;
    float spread = 100.f;
    float minimumRadius = 1.f;
    float medianSpeed = 1.f;
    float speedVariation = 0.f;
    float medianSize = 1.f;
    float sizeVariation = 0.f;
    float deviation = 0.f;
    std::vector<Vec3f> path;
    bool isCircuit = true;
  };

  /**
   * FoliageType
   * -----------
   *
   * Different categories of foliage, describing different types
   * of geometry displacement behavior.
   */
  enum FoliageType {
    NONE,
    FLOWER,
    BRANCH,
    LEAF
  };

  /**
   * Foliage
   * -------
   *
   * @todo description
   */
  struct Foliage {
    FoliageType type = FoliageType::NONE;
    float speed = 1.f;
  };

  /**
   * Mesh
   * ----
   *
   * A Mesh serves as a static reference model from which
   * individual Objects can be created, where Objects
   * only contain transformation properties and other
   * instance-specific attributes.
   */
  struct Mesh {
    /**
     * The index of the mesh in a scene's Mesh array,
     * used for efficient mesh lookups.
     */
    u16 index = 0;
    /**
     * A unique ID for the mesh. If a mesh retrieved
     * at a specific index in a scene's Mesh array
     * does not match an expected ID (e.g., if the
     * mesh structure has been recycled), the reference
     * should be considered stale.
     */
    u16 id = 0;
    /**
     * Static mesh vertices in model space.
     */
    std::vector<Vertex> vertices;
    /**
     * Dynamic mesh vertices, based on the static vertices.
     * Remains empty unless transformGeometry() is used.
     *
     * @todo remove?
     */
    std::vector<Vertex> transformedVertices;
    /**
     * Vertex indices for each triangle face of the mesh,
     * defined in groups of three.
     */
    std::vector<u32> faceElements;
    /**
     * The LOD groups for the Mesh, if applicable.
     *
     * @see MeshLod
     */
    std::vector<MeshLod> lods;
    /**
     * A collection of objects representing unique instances
     * of the mesh.
     */
    ObjectPool objects;
    /**
     * An optional albedo texture for the mesh.
     */
    std::string texture = "";
    /**
     * An optional normal map texture for the mesh.
     */
    std::string normalMap = "";
    /**
     * An optional specularity map texture for the mesh.
     *
     * @todo This doesn't actually control specularity yet
     */
    std::string specularityMap = "";
    /**
     * The probe name to use for probe reflectors.
     */
    std::string probe = "";
    /**
     * Defines the mesh type.
     *
     * @see MeshType
     */
    u8 type = MeshType::DEFAULT;
    /**
     * Controls the maximum directional cascaded shadow
     * map that the mesh objects should be rendered to.
     */
    u8 maxCascade = 3;
    /**
     * Controls whether the mesh's instances are rendered
     * to shadow maps, enabling them to cast shadows.
     */
    bool canCastShadows = true;
    /**
     * Controls whether the mesh and its instances are
     * ignored in all rendering passes.
     */
    bool disabled = false;
    /**
     * Configuration for particle system meshes.
     */
    ParticleSystem particleSystem;
    /**
     * Configuration for foliage meshes.
     */
    Foliage foliage;
    /**
     * Controls how intensely mesh objects render in their
     * natural color, without a light source contribution.
     *
     * @todo material parameters?
     */
    float emissivity = 0.f;

    static Mesh* Cube();
    static Mesh* Model(const char* path);
    static Mesh* Model(const std::vector<std::string>& paths);
    static Mesh* Particles();
    static Mesh* Plane(u32 size, bool useLoopingTexture = false);
    // @todo Sphere(u32 divisions)
    // @todo Cylinder(u32 divisions)

    // @todo remove?
    void transformGeometry(std::function<void(const Vertex&, Vertex&)> handler);
  };

  /**
   * Gm_FreeMesh
   * -----------
   */
  void Gm_FreeMesh(Mesh* mesh);
}