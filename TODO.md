EDITOR
------

BUGS
----
* when enabling the editor in OBJECTS mode with only collision planes enabled, don't show dynamic meshes
* fix onigiri texture
* imported models are inverted along X
* avoid phasing through corners

ASSETS
------
* define outside of world.cpp

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
* rotate character more based on intended motion

* trotting vs. running animation
* dashing animation
* jumping animation
* sitting animation

* slingshot velocity affected by dashing tier
* proper nighttime handling
* lights dimming during daytime
* turn camera after wall kicks
* increased FoV in tiered dash modes
* killplane below level only
* signs/neon
* other food collectibles
* seagulls
* area titles
* train station model updates
* npc model improvements/highlight when nearby
* collectibles
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
* define an optimized Matrix4f * Vec3f operation that skips the w component
* cache textures
* atmospheric scattering improvements
* area lights
* skylight contribution gradation/probe-based GI
* planar reflections
* infinite particles
* sound
* Mesh::MeshPack() ?