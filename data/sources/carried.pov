
#declare domino = mod(clock, 11);
#declare position = div(clock, 11);

#switch(position)
  #range (0, 1)
    #declare W = 3.5;
    #declare H = 13.7;
    #declare D = 6;
    #break
  #case (2)
    #declare W = 3;    // Domino width
    #declare H = 13.7;   // Domino height
    #declare D = 8.5;    // Domino depth
    #break
  #range (3, 5)
    #declare W = 3.5;
    #declare H = 13.7;
    #declare D = 6;
    #break
  #case (6)
    #declare W = 3.5;    // Domino width
    #declare H = 13.7;   // Domino height
    #declare D = 8;    // Domino depth
    #break
#end

#include "shapes.pov"

#macro Domino(num)
  #switch(num)
    #case (0)
      Domino0
      #break;
    #case (1)
      Domino1
      #break;
    #case (2)
      Domino2
      #break;
    #case (3)
      Domino3
      #break;
    #case (4)
      Domino4
      #break;
    #case (5)
      Domino5
      #break;
    #case (6)
      Domino6
      #break;
    #case (7)
      Domino7
      #break;
    #case (8)
      Domino8
      #break;
    #case (9)
      Domino9
      #break;
    #case (10)
      Domino10
      #break;
  #end
#end

#declare Angles = array[8] {0, 5, 10, 20, 40, 55, 78, 90};

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

#macro Position(num)
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

object {
  object
  {
    Domino(domino)
    Position(position)
  }
}



camera {
  orthographic

  location <0, (H+1.5*3)/2, -2>
  look_at  <0, (H+1.5*3)/2, -1>

  up <0, H+1.5*3, 0>
  right <3*(H+1.5*3), 0, 0>

  normal { gradient 400*(x+y) scale 5 translate -2*x-y bump_size 5.3 }
}

light_source { <0, H+10*1000, -1*1000> color rgb <4, 4, 4> }  

