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
  #range(0,14)
    // normal stone
    object {
      Domino0
      PlaceDominoStone(clock-7-0)
    }
    #break
  #range(100,114)
    // blocker ... well it doesn't fall, but still
    object {
      Domino1
      PlaceDominoStone(clock-7-100)
    }
    #break
  #range(200, 214)
    // splitter, first the normal stuff
    object {
      Domino2
      PlaceDominoStone(clock-7-200)
    }
    #break
  #range(215,215+13)

    // special splitter images, has only 14 images, and the angles of the
    // stones are given by an array
    // TODO the lower end overlaps outside of the other stone, which
    // looks ugly, therw we need to cut off the corners
    union {
      intersection {
        object {
          Domino2
          PlaceDominoStone( 7-SplitPos[2*(clock-215)+0])
        }
        object {
          DominoLeftPlane
          PlaceDominoStone(-7+SplitPos[2*(clock-215)+1])
          translate x*0.5
        }
      }
      intersection {
        object {
          Domino2
          PlaceDominoStone(-7+SplitPos[2*(clock-215)+1])
        }
        object {
          DominoRightPlane
          PlaceDominoStone( 7-SplitPos[2*(clock-215)+0])
          translate -x*0.5
        }
      }
    }
    #break
  #range(300,314)
    // exploder, only one image, but we still need them all
    object {
      Domino3
      PlaceDominoStone(clock-7-300)
    }
    #break
  #range(400,414)
    // delay
    object {
      Domino4
      PlaceDominoStone(clock-7-400)
    }
    #break
  #range(500,514)
    // tumbler
    object {
      Domino5
      PlaceDominoStone(clock-7-500)
    }
    #break
  #range(600,614)
    // bridgle builder
    object {
      Domino6
      PlaceDominoStone(clock-7-600)
    }
    #break
  #range(700,714)
    // vanisher
    object {
      Domino7
      PlaceDominoStone(clock-7-700)
    }
    #break
  #range(800,814)
    // trigger
    object {
      Domino8
      PlaceDominoStone(clock-7-800)
    }
    #break
  #range(900,914)
    // raiser the raiser will be resorted by the
    // assembly script, it will also be cropped and what not
    // ...
    object {
      Domino9
      PlaceDominoStone(clock-7-900)
    }
    #break
  #range(1000,1014)
    // connectorA
    object {
      Domino10
      PlaceDominoStone(clock-7-1000)
    }
    #break
  #range(1100,1114)
    // connectorB
    object {
      Domino11
      PlaceDominoStone(clock-7-1100)
    }
    #break

  #range(1200,1214)
    // counter1
    object {
      Domino12
      PlaceDominoStone(clock-7-1200)
    }
    #break
  #range(1300,1314)
    // counter2
    object {
      Domino13
      PlaceDominoStone(clock-7-1300)
    }
    #break
  #range(1400,1414)
    // counter3
    object {
      Domino14
      PlaceDominoStone(clock-7-1400)
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

