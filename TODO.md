EDITOR
------

BUGS
----
* remove lights when unloading level
* stop ground particles from appearing when player is standing still
* head jerking when switching animations

* clouds causing object outline misbehavior behind atmosphere
* air dash complete spin
* imported models are inverted along X
* avoid phasing through corners

ASSETS
------
* vines
* streetlamps
* flowers
* columns
* signs/neon
* stars
* murals/paintings
* painted domes/rooves
* other food collectibles
* weeds LoD
* lamp LoD

GAME
----
* variable camera radius control
* level unloading (in progress)
* level settings files
* player visibility light
* seagull height variation

* sun/moon
* allow SPACE to be queued when pressed just before hitting the ground
* dust clouds when super-jumping
* animation blending
* dynamic tail animations
* run along walls before wall kicking
* fancier dashing effects
* sitting animation
* slingshot velocity affected by dashing tier
* area titles
* train station model updates
* npc model improvements/highlight when nearby
* hot-reloading NPCs/entities data
* avoid running off edges without input
* lens flare
* capes/gliding
* npc movement paths
* controller support
* catnip power up + screen-warp effect
* dialogue events/camera changes during dialogue
* cylinder collision refactor

ENGINE
------
* skip lights at 0 power
* cache textures
* area lights
* skylight contribution gradation/probe-based GI
* planar reflections
* infinite particles
* sound
* Mesh::MeshPack() ?