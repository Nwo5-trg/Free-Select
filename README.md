# Free Select / Select Shenanigans
lasso select and changing select color

## How To Use
click a modifier key as well as however u multiselect on ur platform

## Modifier Key
ok so for windows/mac, [heres the enum class figure it out](https://github.com/cocos2d/cocos2d-x/blob/cocos2d-x-3.13/cocos/base/CCEventKeyboard.h#L48) by default its alt on windows and option on mac

for ios and android uhh read the setting or smth
```json
"lasso-always-enabled": {
    "type": "bool",
    "name": "Lasso Always Enabled",
    "description": "makes lasso the default instead of rect select | on mobile used to disable the lasso select part of the mod and also works in reverse cuz :3c",
    "default": false
}
```

## Scuffed Lasso Select System
i mean idk why anyone would care but

it saves what pos ur touch is at every frame, then when u release ur last pos gets connected to ur first with a straight line

then it does some silly math i dont understand to get a bunch of points inside of the selection probably
(basically using gridsize as the unit it goes thru every space from bl to tr and checks if its inside the selection)

then it loops thru the points and based on `"make-points-boxes"` it either gets all objects on that point and adds it to the selection (like if u just clicked somewhere) or turns every point into a rect the size of grid size and selects everything that overlaps with that rect

