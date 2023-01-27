EDITOR
------
* adjust camera3p azimuth when respawning the player
* SHIFT + mousedrag should center on the collision point
* when changing to collision planes mode, show collision planes
* when creating an object, make it selected + change to position mode
* when toggling collision planes, change editor mode
* count # of active meshes

BUGS
----
* enabling editor with objects disabled restores dynamic object placeholders
* prevent crashes on commands without args
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
* midair jump forward/down action
* daytime sky color changes
* area titles
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