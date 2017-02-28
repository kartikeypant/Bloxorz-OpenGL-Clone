## Bloxorz OpenGL Clone

### Functioning of the Game
![Functioning of the 3D Bloxorz Clone](https://github.com/kartikeypant/Bloxorz-OpenGL-Clone/blob/master/sample.gif)

### Rules of the Game
The movable block is a cuboid made of 2 cubes stacked on top of each other. The length of the side of the cube is same as that of a tile. The Player wins the game when the block falls through the black hole(the block has to be vertical for it to fall through the hole). If the block falls down the edges then the game ends and the player has to repeat the level.

The tiles are of 4 types - normal tiles(gray color), fragile tiles(orange color), bridges(gray color) and switches(green color). In order for a bridge to be constructed, a switch (in a different location) needs to be pressed by the movable block. Bridges can have 2 states - open and closed, which can be toggled as per the requirements of the player. It is not necessary to remain on the switch to keep the bridge in its current state. The next kind of tiles - fragile tiles, are marked using orange color. If the block stands up vertically, then the tile breaks. To move on fragile tiles, the block should lie horizontally (on the longer side).

### Controls and Camera
Controls of the Game are pretty straightforward:-
* UP-DOWN-LEFT-RIGHT keys to move the block.
* 'V' to toggle the View.
* 'A','S','D','R' to change angle of rotation in 'Helicopter View'
* 'J','I','K','L' to change position of camera in 'Helicopter View'

There are 4 views:
1. 'Tower View' (Default) : the camera sits on a tower, to the side of the plane of playing, observing it an angle.
2. 'Top View' : this a top-down view, as if we are looking vertically downwards from a position in the sky.
3. 'Follow-cam View': the camera follows the block from a location behind and above the block.
4. 'Helicopter View': the camera is movable using controls as defined above.
