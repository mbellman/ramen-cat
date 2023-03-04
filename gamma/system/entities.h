#pragma once

#include <functional>
#include <string>
#include <vector>

#include "math/geometry.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "math/Quaternion.h"
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
    float radius = 100.f;
    Vec3f color = Vec3f(1.f);
    float power = 1.f;
    Vec3f direction = Vec3f(0.f, -1.f, 0.f);
    float fov = 90.f;
    float basePower = 1.f;
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
   * @size 6 bytes
   */
  struct ObjectRecord {
    u16 meshIndex = 0;
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
   * @size 52 bytes
   */
  struct Object {
    ObjectRecord _record;
    Vec3f position;
    Vec3f scale;
    Quaternion rotation;
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
     * Defines translucent particles, rendered as points with
     * radial gradients.
     */
    PARTICLES = 0xA0,
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
    PRESET_ANIMATED = 0xFC,
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
   * Particles
   * ---------
   *
   * @todo description
   */
  struct Particles {
    Vec3f spawn;
    float spread = 100.f;
    float minimumRadius = 1.f;
    float medianSpeed = 1.f;
    float speedVariation = 0.f;
    float medianSize = 1.f;
    float sizeVariation = 0.f;
    float deviation = 0.f;
    std::vector<Vec3f> path;
    bool useGpuParticles = false;
    bool isCircuit = true;
  };

  /**
   * PresetAnimationType
   * -------------------
   *
   * Different categories of preset animations, describing different types
   * of geometry displacement behavior.
   */
  enum PresetAnimationType {
    NONE,
    FLOWER,
    LEAF,
    BIRD,
    CLOTH
  };

  /**
   * PresetAnimation
   * ---------------
   *
   * @todo description
   */
  struct PresetAnimation {
    PresetAnimationType type = PresetAnimationType::NONE;
    float speed = 1.f;
    float factor = 1.f;
  };

  /**
   * MeshAttributes
   * --------------
   *
   * @todo description
   */
  struct MeshAttributes {
    /**
     * Defines the mesh type.
     */
    MeshType type = MeshType::DEFAULT;
    /**
     * An optional albedo texture for the mesh.
     */
    std::string texture = "";
    /**
     * An optional normal map texture for the mesh.
     */
    std::string normals = "";
    /**
     * The probe name to use for probe reflectors.
     */
    std::string probe = "";
    /**
     * Controls the maximum directional cascaded shadow
     * map that the mesh objects should be rendered to.
     */
    u8 maxCascade = 3;
    /**
     * Configuration for particle meshes.
     */
    Particles particles;
    /**
     * Configuration for preset animated meshes.
     */
    PresetAnimation animation;
    /**
     * Controls how intensely mesh objects render in their
     * natural color, without a light source contribution.
     */
    float emissivity = 0.f;
    /**
     * Controls light scattering behavior from the surface
     * of mesh objects. A higher roughness corresponds to
     * more of a matte appearance, whereas a lower roughness
     * approximates more glossy/metallic surfaces.
     */
    float roughness = 0.6f;
    /**
     * Controls whether the mesh's instances are rendered
     * to shadow maps, enabling them to cast shadows.
     */
    bool canCastShadows = true;
    /**
     * Controls whether the mesh instances should render silhouettes
     * when occluded by other geometry.
     */
    bool silhouette = false;
    /**
     * Controls whether we should use mipmaps for the mesh textures.
     */
    bool useMipmaps = true;
    /**
     * Controls whether mesh object pixels should be made 'translucent'
     * when close up to the near plane.
     */
    bool useCloseTranslucency = false;
    /**
     * Controls whether geometry is textured across the xz plane.
     */
    bool useXzPlaneTexturing = false;
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
  struct Mesh : MeshAttributes {
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
     * The name of the mesh.
     */
    std::string name = "";
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
     * Controls whether the mesh and its instances are
     * ignored in all rendering passes.
     */
    bool disabled = false;

    static Mesh* Cube();
    static Mesh* Sphere(u8 divisions = 5);
    static Mesh* Model(const char* path);
    static Mesh* Model(const std::vector<std::string>& paths);
    static Mesh* Particles(bool useGpuParticles = false);
    static Mesh* Plane(u32 size, bool useLoopingTexture = false);
    static Mesh* Disc(u32 slices);
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