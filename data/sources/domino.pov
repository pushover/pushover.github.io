#declare domino = div(clock, 100);
#declare position = mod(clock, 100);

#declare carryStart = 90;   // the index position, where the carried image starts

#switch(position)
  #case (carryStart+2)
    #declare W = 3;    // Domino width
    #declare H = 13.7;   // Domino height
    #declare D = 8.5;    // Domino depth
    #break
  #case (carryStart+6)
    #declare W = 3.5;    // Domino width
    #declare H = 13.7;   // Domino height
    #declare D = 8;    // Domino depth
    #break
  #else
    #declare W = 3.5;
    #declare H = 13.7;
    #declare D = 6;
#end

#include "data/sources/shapes.pov"

#declare Angles = array[8] {0, 5, 10, 20, 40, 55, 78, 90};
#declare SplitPos=array[28] { 0,0,  1,1,  2,2,  3,3,  4,4,  5,5,  6,6,  7,7,  1,0,  0,1,  2,0,  0,2,  1,2,  2,1 };

#macro PlaceDominoStone(Angle)
  #if (Angle >= 0)
    translate -W/2*x
    rotate -z*Angles[Angle]
    translate W/2*x*(7-Angle)/7
  #else
    translate W/2*x
    rotate z*Angles[-Angle]
    translate -W/2*x*(Angle+7)/7
  #end
#end

#macro PlaceCarryDominoStone(num)
  #switch(num)
    #case (0)
      PlaceDominoStone(3)
      #break
    #case (1)
      PlaceDominoStone(-3)
      #break
    #case (2)
      rotate y*90
      #break
    #case (3)
      PlaceDominoStone(-2)
      rotate y*5
      translate -y*0.15
      #break
    #case (4)
      PlaceDominoStone(-3)
      rotate y*30
      translate -x*0.2
      #break
    #case (5)
      PlaceDominoStone(2)
      rotate -y*50
      #break
    #case (6)
      PlaceDominoStone(2)
      rotate -y*70
      translate -y*0.15
      translate x*0.05
      #break
  #end
#end


#switch (position)
  #range (0,14)
    // normal stone, coration images
    object {
      Domino(domino)
      PlaceDominoStone(position-7)
    }
    #break;
  #range (carryStart,carryStart+6)
    // carried stone images
    object {
      Domino(domino)
      PlaceCarryDominoStone(position-carryStart)
    }
    #break;
#end

#if (domino = 2 & position >= 15 & position <= 28)
  // special splitter images, has only 14 images, and the angles of the
  // stones are given by an array
  union {
    intersection {
      object {
        Domino(domino)
        PlaceDominoStone( 7-SplitPos[2*(position-15)+0])
      }
      object {
        DominoLeftPlane
        PlaceDominoStone(-7+SplitPos[2*(position-15)+1])
        translate x*0.5
      }
    }
    intersection {
      object {
        Domino(domino)
        PlaceDominoStone(-7+SplitPos[2*(position-15)+1])
      }
      object {
        DominoRightPlane
        PlaceDominoStone( 7-SplitPos[2*(position-15)+0])
        translate -x*0.5
      }
    }
  }
#end

camera {
  orthographic

  #if (position < carryStart)
    location <0, (H+1.5*3)/2, -0.0001>
    look_at  <0, (H+1.5*3)/2, 0>
  #else
    location <0, (H+1.5*3)/2, -2.1>
    look_at  <0, (H+1.5*3)/2, -1>
  #end

  up <0, H+1.5*3, 0>
  right <3*(H+1.5*3), 0, 0>

  normal { gradient 400*(x+y) scale 5 translate -2*x-y bump_size 5.3 }
}

light_source { <0, H+10*1000, -1*1000> color rgb <4, 4, 4> }

