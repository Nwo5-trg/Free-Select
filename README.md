# Free Select / Select Shenanigans
lasso select and changing select color

## How To Use
click "*#ifdef GEODE_IS_MACOS (a) #endif* ***modifier key***" as well as however u multiselect on ur platform

## Modifier Key
ok so on windows u can change the modifier thru custom keybinds by default its alt i think

with mac u set the modifier(s) (theres 2 cuz by default its r/l option) to keycodes thru settings
why? you might ask, well maybe if custom keybinds worked for me and some other mac users or at the very least released its optional api to the index then maybe it wouldnt be like this but noooo, so you are stuck with this mac users (me included)

for ios and android uhh read the setting or smth
```json
"lasso-always-enabled": {
    "type": "bool",
    "name": "Lasso Always Enabled",
    "description": "makes lasso the default instead of rect select | on mobile used to disable the lasso select part of the mod and also works in reverse cuz :3c",
    "default": false
}
```

## My Scuffed Lasso Select System
i mean idk why anyone would care but

it saves what pos ur touch is at every frame, then when u release ur last pos gets connected to ur first with a straight line

then it does some silly math i dont understand to get a bunch of points inside of the selection probably
(basically using gridsize as the unit it goes thru every space from bl to tr and checks if its inside the selection)

then it loops thru the points and based on `"make-points-boxes"` it either gets all objects on that point and adds it to the selection (like if u just clicked somewhere) or turns every point into a rect the size of grid size and selects everything that overlaps with that rect

