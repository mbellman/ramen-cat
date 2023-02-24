EDITOR
------

BUGS
----
* wall kicking at same y coordinate as last solid ground position
* when enabling the editor in OBJECTS mode with only collision planes enabled, don't show dynamic meshes

* clouds causing object outline misbehavior behind atmosphere
* air dash complete spin (somersault instead?)
* fix onigiri texture
* imported models are inverted along X
* avoid phasing through corners

ASSETS
------
* flowers
* update mini rooves to look more japanese
* staircase step improvements
* stars
* murals/paintings
* weeds LoD
* lamp LoD
* vines
* signs/neon
* other food collectibles

GAME
----
* rotate character more based on intended motion
* level unloading (in progress)
* fancier dashing effects
* sitting animation
* seagulls
* slingshot velocity affected by dashing tier
* area titles
* train station model updates
* npc model improvements/highlight when nearby
* hot-reloading NPCs/entities data
* avoid running off edges without input
* food carts
* lens flare
* capes/gliding
* npc movement paths
* controller support
* catnip power up + screen-warp effect
* dialogue events/camera changes during dialogue
* cylinder collision refactor

ENGINE
------
* optional texture mipmaps

* cache textures
* area lights
* skylight contribution gradation/probe-based GI
* planar reflections
* infinite particles
* sound
* Mesh::MeshPack() ?