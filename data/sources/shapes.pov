// this file contains the hapes of all the used dominos

#declare TxtY = texture {
  pigment { color rgb <1, 1, 0> }
  finish { ambient rgb <0.7, 0.4, 0> }
}

#declare TxtR = texture {
  pigment { color rgb <1, 0, 0> }
  finish { ambient rgb <0.4, 0, 0> }
}

#declare Fase = 0.3;

#declare Edges = intersection {
  plane { (x-z)/sqrt(2), -Fase translate  W/2*x }
  plane {(-x-z)/sqrt(2), -Fase translate -W/2*x }
  plane { (y-z)/sqrt(2), -Fase translate    H*y }
  plane {(-y-z)/sqrt(2), -Fase }

  plane { (x+y)/sqrt(2), -Fase translate  W/2*x+H*y }
  plane {(-x+y)/sqrt(2), -Fase translate -W/2*x+H*y }
  plane { (x-y)/sqrt(2), -Fase translate  W/2*x }
  plane {(-x-y)/sqrt(2), -Fase translate -W/2*x }

  plane { (x+z)/sqrt(2), -Fase translate  W/2*x+D*z }
  plane {(-x+z)/sqrt(2), -Fase translate -W/2*x+D*z }
  plane { (y+z)/sqrt(2), -Fase translate    H*y+D*z }
  plane {(-y+z)/sqrt(2), -Fase translate    D*z }

  plane { (x+y+z)/sqrt(3), -Fase*1.7 translate  W/2*x+H*y+D*z }
  plane { (x+y-z)/sqrt(3), -Fase*1.7 translate  W/2*x+H*y }
  plane { (x-y+z)/sqrt(3), -Fase*1.7 translate  W/2*x+D*z }
  plane { (x-y-z)/sqrt(3), -Fase*1.7 translate  W/2*x }
  plane {(-x+y+z)/sqrt(3), -Fase*1.7 translate -W/2*x+H*y+D*z }
  plane {(-x+y-z)/sqrt(3), -Fase*1.7 translate -W/2*x+H*y }
  plane {(-x-y+z)/sqrt(3), -Fase*1.7 translate -W/2*x+D*z }
  plane {(-x-y-z)/sqrt(3), -Fase*1.7 translate -W/2*x }
}

// standard
#declare Domino0 = intersection {
  box { <-W/2, 0, 0> <W/2, H, D> }
  object { Edges }
  texture { TxtY }
}

// blocker
#declare Domino1 = intersection {
  box { <-W/2, 0, 0> <W/2, H, D> }
  object { Edges }
  texture { TxtR }
}

// splitter
#declare Domino2 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, H/2, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, H/2, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

// exploder
#declare Domino3 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, H, D/2> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 0, D/2> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

// delay
#declare Domino4 = union {
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { y+z, 0 translate y }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { -y-z, 0 translate y }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtY }
  }
}

// tumbler
#declare Domino5 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, 1*H/4, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 1*H/4, 0> <W/2, 3*H/4, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 3*H/4, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

// bridger
#declare Domino6 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, 6*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 6*H/14, 0> <W/2, 8*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 8*H/14, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

// vanisher
#declare Domino7 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, 4*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 4*H/14, 0> <W/2, 6*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 6*H/14, 0> <W/2, 8*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 8*H/14, 0> <W/2, 10*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 10*H/14, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

// trigger
#declare Domino8 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, 2*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 2*H/14, 0> <W/2, 4*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 4*H/14, 0> <W/2, 6*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 6*H/14, 0> <W/2, 8*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 8*H/14, 0> <W/2, 10*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 10*H/14, 0> <W/2, 12*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 12*H/14, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

// raiser
#declare Domino9 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, H, D/3> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 0, D/3> <W/2, H, 2*D/3> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 0, 2*D/3> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

// tangled 1
#declare Domino10 = union {
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { y+z, 0 translate y }
      plane { y-z, 0 translate 0 }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { y+z, 0 translate y }
      plane { -y+z, 0 translate 0 }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { -y-z, 0 translate y }
      plane { y-z, 0 translate 0 }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { -y-z, 0 translate y }
      plane { -y+z, 0 translate 0 }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtR }
  }
}

// tangled 2
#declare Domino11 = union {
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { y+z, 0 translate y }
      plane { y-z, 0 translate 0 }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { y+z, 0 translate y }
      plane { -y+z, 0 translate 0 }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { -y-z, 0 translate y }
      plane { y-z, 0 translate 0 }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    intersection {
      box { <-W/2, 0, 0> <W/2, 1, 1> }
      plane { -y-z, 0 translate y }
      plane { -y+z, 0 translate 0 }
      scale <1, H, D>
    }
    object { Edges }
    texture { TxtY }
  }
}


// counter stopper 1
#declare Domino12 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, 6*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 6*H/14, 0> <W/2, 8*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 8*H/14, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtR }
  }
}

// counter stopper 2
#declare Domino13 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, 4*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 4*H/14, 0> <W/2, 6*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 6*H/14, 0> <W/2, 8*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 8*H/14, 0> <W/2, 10*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 10*H/14, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtR }
  }
}

// counter stopper 3
#declare Domino14 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, 2*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 2*H/14, 0> <W/2, 4*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 4*H/14, 0> <W/2, 6*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 6*H/14, 0> <W/2, 8*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 8*H/14, 0> <W/2, 10*H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 10*H/14, 0> <W/2, 12*H/14, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 12*H/14, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtR }
  }
}


// missile
#declare Domino15 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, H, D/3> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 0, D/3> <W/2, H, D/2> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 0, D/2> <W/2, H, 2*D/3> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 0, 2*D/3> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

// glider
#declare Domino16 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, 1*H/4, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2, 1*H/4, 0> <W/2, H/2-H/14, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, H/2+H/14, 0> <W/2, 3*H/4, D> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2, 3*H/4, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
}

#declare my = 0.00000001;

// starter
#declare Domino17 = union {
  intersection {
    box { <-W/2, 0, 0> <W/2, H, D> }
    object { Edges }
    texture { TxtY }
  }
  intersection {
    box { <-W/2-my, H/3, 1*D/7> <W/2+my, 2*H/3, 2*D/7> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2-my, H/3, 3*D/7> <W/2+my, 2*H/3, 4*D/7> }
    object { Edges }
    texture { TxtR }
  }
  intersection {
    box { <-W/2-my, H/3, 5*D/7> <W/2+my, 2*H/3, 6*D/7> }
    object { Edges }
    texture { TxtR }
  }
}

#declare DominoRightPlane = box { <-100, -100, -100> <W/2, 100, 100> }
#declare DominoLeftPlane =  box { <-W/2, -100, -100> <100, 100, 100> }

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
    #case (11)
      Domino11
      #break;
    #case (12)
      Domino12
      #break;
    #case (13)
      Domino13
      #break;
    #case (14)
      Domino14
      #break;
  #end
#end

