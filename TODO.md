EDITOR
------
* count # of vertices over all instances of a mesh
* show # of dynamic entity pieces
* camera-relative repositioning hotkey
* uniform scaling hotkey
* hitbox offset
* debug camera position

BUGS
----
* jumping at high framerates on sloped surfaces
* clouds texture near vec3(0, 1, 0)
* fix flying off downward slopes
* imported models are inverted along X
* rotated collision plane wall kick height
* avoid phasing through corners
* collision plane gaps/discontinuities

ASSETS
------
* flowers
* update mini rooves to look more japanese
* staircase step improvements
* stars
* murals/paintings
* stars
* weeds LoD
* lamp LoD
* curved building structures

GAME
----
* allow small off-ground delays before jumps
* camera radius pan-out when velocity is high enough
* silhouettes of certain objects behind things
* area titles
* npc model improvements/highlight when nearby
* collectibles
* hot-reloading NPCs/entities data
* avoid running off edges without input
* bouncy platforms/objects
* food carts
* lens flare
* capes/gliding
* npc movement paths
* controller support
* compound entities
* animated cat behavior (scaled spheres as appendages for now)
* day/night cycle
* catnip power up + screen-warp effect
* dialogue events/camera changes during dialogue
* cylinder collision refactor

ENGINE
------
* cache textures
* atmospheric scattering improvements
* proper clouds handling
* sound
* planar reflections
* infinite particles
* skylight contribution gradation
* Mesh::MeshPack() ?