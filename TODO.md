EDITOR
------

BUGS
----
* air dash complete spin (somersault instead?)
* wall kicking at same y coordinate as last solid ground position
* when enabling the editor in OBJECTS mode with only collision planes enabled, don't show dynamic meshes
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
* curved building structures

GAME
----
* jumps proportional to key held time
* rotate character more based on intended motion
* level unloading (in progress)
* sitting animation
* proper nighttime handling
* increased FoV in tiered dash modes/dash fx
* signs/neon
* other food collectibles
* seagulls
* lights dimming during daytime
* slingshot velocity affected by dashing tier
* turn camera after wall kicks
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
* increase directional shadow range
* cache textures
* atmospheric scattering improvements
* area lights
* skylight contribution gradation/probe-based GI
* planar reflections
* infinite particles
* sound
* Mesh::MeshPack() ?