#declare W = 3.5;
#declare H = 13.7;
#declare D = 6;


#include "shapes.pov"

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



#switch(clock)
  #range(0*15,0*15+14)
    // normal stone
    object {
      Domino0
      PlaceDominoStone(clock-7-0*15)
    }
    #break
  #range(1*15,1*15+14)
    // blocker ... well it doesn't fall, but still
    object {
      Domino1
      PlaceDominoStone(clock-7-1*15)
    }
    #break
  #range(2*15,2*15+14-1)
    // splitter, has only 14 images, and the angles of the
    // stones are given by an array 
    // TODO the lower end overlaps outside of the other stone, which
    // looks ugly, therw we need to cut off the corners
    union {
      intersection {
        object {
          Domino2
          PlaceDominoStone( 7-SplitPos[2*(clock-2*15)+0])
        }
        object {
          DominoLeftPlane
          PlaceDominoStone(-7+SplitPos[2*(clock-2*15)+1])
          translate x*0.5
        }
      }
      intersection {
        object {
          Domino2
          PlaceDominoStone(-7+SplitPos[2*(clock-2*15)+1])
        }
        object {
          DominoRightPlane
          PlaceDominoStone( 7-SplitPos[2*(clock-2*15)+0])
          translate -x*0.5
        }
      }
    }
    #break
  #range(3*15-1,3*15-1)
    // exploder, only one image
    object {
      Domino3
      PlaceDominoStone(0)
    }
    #break
  #range(3*15,3*15+14)
    // delay
    object {
      Domino4
      PlaceDominoStone(clock-7-3*15)
    }
    #break
  #range(4*15-1,4*15+14)
    // tumbler
    object {
      Domino5
      PlaceDominoStone(clock-7-4*15)
    }
    #break
  #range(5*15-1,5*15+14)
    // bridgle builder
    object {
      Domino6
      PlaceDominoStone(clock-7-5*15)
    }
    #break
  #range(6*15-1,6*15+14)
    // vanisher
    object {
      Domino7
      PlaceDominoStone(clock-7-6*15)
    }
    #break
  #range(7*15-1,7*15+14)
    // trigger
    object {
      Domino8
      PlaceDominoStone(clock-7-7*15)
    }
    #break
  #range(8*15-1,8*15+14)
    // raiser the raiser will be resorted by the 
    // assembly script, it will also be cropped and what not
    // ...
    object {
      Domino9
      PlaceDominoStone(clock-7-8*15)
    }
    #break
  #range(9*15-1,9*15+14)
    // connector
    object {
      Domino10
      PlaceDominoStone(clock-7-9*15)
    }
    #break
  #range(10*15-1,10*15+14)
    // connector
    object {
      Domino11
      PlaceDominoStone(clock-7-10*15)
    }
    #break
#end

camera {
  orthographic

  location <0, (H+1.5*3)/2, -0.00001>
  look_at  <0, (H+1.5*3)/2, 0>

  up <0, H+1.5*3, 0>
  right <3*(H+1.5*3), 0, 0>

  normal { gradient 400*(x+y) scale 5 translate -2*x-y bump_size 5.3 }
}

light_source { <0, H+10*1000, -1*1000> color rgb <4, 4, 4> }  

