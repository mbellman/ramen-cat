EDITOR
------
* keep LIGHTS mode when re-enabling editor

BUGS
----
* fix flying off downward slopes (more accurate snap-to-ground behavior)
* don't snap to ground for a brief duration after jumps
* falling down when maximimizing the window on startup (???)

* imported models are inverted along X
* rotated collision plane wall kick height
* avoid phasing through corners
* collision plane gaps/discontinuities

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
* wires
* slingshot velocity affected by dashing tier
* proper nighttime handling
* turn camera after wall kicks
* increased FoV in tiered dash modes
* killplane below level only

* signs/neon stuff
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
* compound entities
* animated cat behavior (scaled spheres as appendages for now)
* catnip power up + screen-warp effect
* dialogue events/camera changes during dialogue
* cylinder collision refactor

ENGINE
------
* cache textures
* atmospheric scattering improvements
* sound
* planar reflections
* infinite particles
* skylight contribution gradation/probe-based GI
* Mesh::MeshPack() ?