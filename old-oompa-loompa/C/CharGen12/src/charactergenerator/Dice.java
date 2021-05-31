package charactergenerator;

import Math.*;

/**
 * Title:        CoC Character Generator
 * Description:  The dice simulator
 * Copyright:    Copyright (c) 2002
 * Company:
 * @author Markus Svensson
 * @version 1.0
 */

public class Dice {

  public Dice() {
  }

  /**
   * Roll one d6
   * @return The result
   */
  public int d6(){
    return d6(1);
  }

  /**
   * Roll toRoll number of d6
   * @param toRoll - The number of d6 to roll
   * @return The added result of all d6'es rolled
   */
  public int d6(int toRoll){
    int temp = 0;
    for(int i = 0; i < toRoll; i++)
      temp += (int)((Math.random() * 6)+1);
    return temp;
  }

  /**
   * Roll one D10
   * @return The result
   */
  public int d10(){
    return d10(1);
  }

  /**
   * Roll toRoll number of D10
   * @param toRoll - The number of rolls
   * @return The added result of all d10's rolled
   */
  public int d10(int toRoll){
    int temp = 0;
    for(int i = 0; i < toRoll; i++)
      temp += (int)((Math.random() * 10)+1);
    return temp;
  }
}