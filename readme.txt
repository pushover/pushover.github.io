PUSHOVER
===========

Pushover is a faithful reimplementation of the game with the same name published
in 1992 by Ocean. It contains the same levels and uses the same graphics and
sound as the original game.


Gameplay
-----------

The task of the game is to rearrange the dominos on the different patforms so
that you can start a chainreation that makes all dominos topple over. You may
rearrange all dominos (except for one kind of domino) and place them wherever
suitable (except in front of the door).


You win the level, when:

- all dominos (except for the blocker) have toppled
- no dominos have crashed, they may fall off the screen though
- the trigger domino fell last
- and you reached the exit door within the time limit

The dominos:

All in all there are 10 different types of dominos:

- Standard, completely yellow. There is nothing special with this stone, it
  falls when pushed.
- Blocker, completely red. This domino can not fallover, so it is the only kind
  of stone they may still be standing when the level is solved. Dominos falling
  against this stone will bounce back, if possible
- Tumbler, big red stripe. This domino will stand up again after falling and
  will continue to tubmle until it hits an obstacle of rests on an other stone
- Delay stone, diagonally divided. This domino will take some time until if
  falls, when it is pushed. Dominos falling against this stone will bounce back
  and later on this stone will fall
- Splitter, horizontally divided. This stone will split into 2 stones, one
  falling to the left and the other falling to the right. The splitter can not
  be pushed it must be split by a stone falling onto it from above.
- Exploder, vertically divided. This stone will blast a gap into the platform it
  is standing on, when it is pushed. Neither the ant nor the pushing domino are
  harmed by that, the pushing domino will fall into the gap
- Bridger, 1 hrozontal strip. The bridger will try to conned the edge it is
  standing on with the next edge, if it is close enough, if not it will simply
  fall into the gap.
- Vanisher, 2 horizontal strips. The Vanisher will disappear as soon as it lies
  flat on the ground. This is the only stone you may place in front of doors
- Trigger, 3 horizontal strips. This stone will open the exit door, as soon as
  it lies completely flat and all other conditions are met (see above). This is
  the only stone that you may not move around
- Ascender, vertical strip. This stone will start to rise as soon as it is
  pushed. It will rise until is hits the ceiling, then it will start to flip
  into the direction it was initially pushed. When you fall into a gap while
  holdig this stone it will also rise and stay at the ceiling until pushed.

Controls

The ant is controlled using the cursor keys and space. Use the space key to pick
up the domino behind the ant or to place it down where you are currently
standing. To push press first up to let the ent enter the row of dominos. Then
press space and either left or right corsor key depending on if you want to push
the domino to your left or your right.

Hints

If you don't know where to start in a level, simply if one stone a push and see
what happens. This helps very often to get a general idea how to solve a level
and where the problem is

If you forgot which domino has what kind of special property press F1 to get a
short help. This window also displays a short hint, once the time of the level
is out.

The first few levels introduce you to the dominos here you can explore how the
different dominos behave in different situations


Files
--------

Pushover places a few files on your harddisc in everyday running. Those files
will be placed in your home directory. This directory is in

-  Eigene Dateien\Pushover  on German Windows systems, don't know what it is
                            called in other languages
-  ~/.pushover              on Unix systems

The following files are saved:

- solved.txt: This file contains checksums of all the levels that you have
  successfully solved. In the level selection dialog those levels contain a
  mark. If you loose this file thos marks are gone
- *.rec: These filed contain recordings of activities within a level. They are
  automatically created whenever you solve a level but you can also actively
  make a recording by pressing 'r' while you play a level. When you observe
  something strange while playing, make a recording and send it to me. Also when
  the game crashes a recording will be saved.  You can delete these files
  whenever you want. You can distinguish the recordings by the prefix in their
  name. "Sol" stands for solved levels, "Man" for manually created recordings
  and "Err" for recordings made when the program crashed.


Graphics
----------

Right now Pushover uses scaled versions of the graphics of the original game.
You are very much invited to improve those graphics. Please contact me if you
are interested so that I can tell you what the state of affairs is. But I will
tell here the basics

I have already replaced the dominos with new graphics, but the graphics for the
ant, the background themes still need improvements.

The backgrounds are made out of 20x13 blocks. Each block has a size of 40x48
pixel. The source of that is the non square pixel of the 320x200 resolution of
the original game. For each theme there is a PNG image file containing all the
blocks that may be used by the levels. To make is possible to put the blocks
more freely into the PNG file a LUA file accompanies the image. This LUA files
contains the block positions of all the used blocks. Right now all the blocks
are one below another so the LUA files contain ever increasing y positions and
always the same x positon.

It is already implemented to use transparency within the blocks. All the
existing levels use just one layer and thus need completely opague tiles. But
many of those tiles could be separated into a stack of different tiles. This
then means the level need to be updated to actually use a stack of tiles instead
of just one. Right now we are limited to 8 layers, but if necessary this can be
made dynamic.

It is also planned to have something like animated tiles, but they have to be
kept at a low count. Not too many frames and not too many animations. They are
not intended to make the background dynamic, but to rather be a little finishing
touch to the graphics. Possibilities are trees that move from time to time in a
breeze, a bird that sails through the sky from time to time....

The ant is more complicated. The image ant.png contains all possible animation
images for the ant, one animation below the other. I have an additional GIMP
image that contains in seperate layers possible surrondings of the ant in
different animation frames (like ladders, steps, ground, a carried domino...). I
will happily provide that image to the interested artist.


Level Designers
-----------------

Pushover will eventually get a level editor, but right now it hasn't, please be
patient.

But here are already some rules that you have to adhere to if you want your
levels included into the program

- They need to be put under the GPL licence, or something compatible. Otherwise
  inclusion is legally not possible, copyright stays with you, of course
- Please only contribute complete sets and no single levels. They don't need to
  be long 10 Levels is enough but this way you can keep up a constant scheme and
  theme logic of the levels
- You absolutely _must_ provide a recording of one possible solution to the
  level. That solution is not within the distribution, just within the source
  code repository. It is used to ensure that the solution of your level is still
  possible after we made changes to the program. This way we limit possible
  frustration
- Do only send non compressed level sets. We need those for the inclusion in the
  source. And we need those for possible future updates
- Use the index file to resort levels, don't rename the files. This way
  inserting a level becomes very easy
- you must not have more than one animated tile per level background

Credits
---------

My thanks go to the original developers of the game. .....


