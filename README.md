## Bloxorz OpenGL Clone

### Abstract
The player wins the game when the block falls through the black hole(the block has to be vertical for it to fall through the hole). If the block falls down the edges then the game ends.

### Gameplay
![Functioning of the 3D Bloxorz Clone](https://github.com/kartikeypant/Bloxorz-OpenGL-Clone/blob/master/sample.gif)

### Rules of the Game
The tiles are of 4 types - normal tiles(gray color), fragile tiles(orange color), bridges(gray color) and switches(green color). 

* If the block falls down the edges, the player loses.
* If the block vertically stands on a fragile tile, the tile breaks. Hence, the player loses.
* To move on fragile tiles, the block should lie horizontally(on the longer side).
* When the block touches the switch in any orientation, a state of the bridge(open or closed) is toggled.


### Controls and Camera
Controls of the Game are pretty straightforward:-
* UP-DOWN-LEFT-RIGHT keys to move the block.
* 'V' to toggle the View.
* 'A','S','D','R' to change angle of rotation in 'Helicopter View'
* 'J','I','K','L' to change position of camera in 'Helicopter View'

There are 4 views:

* 'Tower View' (Default) : the camera sits on a tower, to the side of the plane of playing, observing it an angle.
* 'Top View' : this a top-down view, as if we are looking vertically downwards from a position in the sky.
* 'Follow-cam View': the camera follows the block from a location behind and above the block.
* 'Helicopter View': the camera is movable using controls as defined above.
