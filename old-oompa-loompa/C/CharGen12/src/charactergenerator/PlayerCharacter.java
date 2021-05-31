package charactergenerator;

import java.util.Vector;

/**
 * Title:        CoC Character Generator
 * Description:  A character for Call of Cthulhu
 * Copyright:    Copyright (c) 2002
 * Company:
 * @author Markus Svensson
 * @version 1.2
 */

public class PlayerCharacter {
  private int strength = 0;
  private int dexterity = 0;
  private int intelligence = 0;
  private int constitution = 0;
  private int apperance = 0;
  private int power = 0;
  private int size = 0;
  private int sanity = 0;
  private int education = 0;
  private int idea = 0;
  private int know = 0;
  private int luck = 0;
  private int hitPoints = 0;
  private int magicPoints = 0;
  private int moneyAvailable = 0;
  private int occupationPoints = 0;
  private int personalInterestPoints = 0;
  private int minAge = 0;
  private int era = 0;
  private Vector skills = new Vector();
  public int[] skillValues = new int[17];
  private String damageBonus;
  private Dice generator;

  /**
   * Constructor
   * @param era - The era of play
   */
  public PlayerCharacter(int era) {
    this.era = era;
    generator = new Dice();
  }

  /**
   * Set the skills vector
   * @param skills - The vector
   */
  public void setSkills(Vector skills){
    this.skills = skills;
  }

  /**
   * Get the skills vector
   * @return the skills vector
   */
  public Vector getSkills(){
    return skills;
  }

  /**
   * Set the skills vector
   * @param skills - The vector
   */
  public void setSkillValues(Vector skills){
    this.skillValues = skillValues;
  }

  /**
   * Getter methods
   * @return Number of hit points
   */
  public int getHitPoints(){
    return hitPoints;
  }

  public int getMagicPoints(){
    return magicPoints;
  }

  public int getMoneyAvailable(){
    return moneyAvailable;
  }

  public int getOccupationPoints(){
    return occupationPoints;
  }

  public int getPersonalInterestPoints(){
    return personalInterestPoints;
  }

  public int getMinAge(){
    return minAge;
  }

  public int getEra(){
    return era;
  }

  public int getStrength(){
    return strength;
  }

  public int getDexterity(){
    return dexterity;
  }

  public int getIntelligence(){
    return intelligence;
  }

  public int getConstitution(){
    return constitution;
  }

  public int getApperance(){
    return apperance;
  }

  public int getPower(){
    return power;
  }

  public int getSize(){
    return size;
  }

  public int getSanity(){
    return sanity;
  }

  public int getEducation(){
    return education;
  }

  public int getIdea(){
    return idea;
  }

  public int getKnow(){
    return know;
  }

  public int getLuck(){
    return luck;
  }

  public String getDamageBonus(){
    return damageBonus;
  }

  /**
   * Get the character as a String
   * @return String - The character, in String format
   */
  public String getString(){
    String temp = new String();
    temp = "Stats:\n";
    temp += "APP: " + apperance + "\n";
    temp += "CON: " + constitution + "\n";
    temp += "DEX: " + dexterity + "\n";
    temp += "EDU: " + education + "\n";
    temp += "INT: " + intelligence + "\n";
    temp += "POW: " + power + "\n";
    temp += "SIZ: " + size + "\n";
    temp += "STR: " + strength + "\n";
    temp += "SAN: " + sanity + "\n";
    temp += "-----------------------------\n";
    temp += "Damage Bonus: " + damageBonus + "\n";
    temp += "Idea: " + idea +"\n";
    temp += "Luck: " + luck + "\n";
    temp += "Know: " + know + "\n";
    temp += "Occupation SP: " + occupationPoints + "\n";
    temp += "Personal Interest SP: " + personalInterestPoints + "\n";
    temp += "Magic Points: " + magicPoints + "\n";
    temp += "Hit Points: " + hitPoints + "\n";
    temp += "Money Available: $" + moneyAvailable + "\n";
    temp += "-----------------------------\n";
    temp += "Skill / Value:\n";
    for(int i = 0; i < skills.size(); i++)
    {
      temp += "" + (String)skills.elementAt(i) + " / " + skillValues[i] + "%\n";
    }

    return temp;
  }

  /**
   * Generate the stats for the character
   */
  public void generateStats(){
    strength = generator.d6(3);
    constitution = generator.d6(3);
    power = generator.d6(3);
    dexterity = generator.d6(3);
    apperance = generator.d6(3);
    size = generator.d6(2) + 6;
    intelligence = generator.d6(2) + 6;
    education = generator.d6(3) + 3;
    sanity = power * 5;
    idea = intelligence * 5;
    luck = power * 5;
    know = education * 5;
    damageBonus = calcDB();
    hitPoints = calcHitPoints();
    magicPoints = power;
    moneyAvailable = calcMoneyAvailable(era);
    occupationPoints = education * 20;
    personalInterestPoints = intelligence * 10;
    minAge = education + 6;
  }

  /**
   * Calculate the damage bonus
   * @return The db
   */
  private String calcDB(){
    int result = size + strength;

    if(result >= 2 && result <= 12)
      return "-1D6";
    else if(result >= 13 && result <= 16)
      return "-1D4";
    else if(result >= 17 && result <= 24)
      return "NONE";
    else if(result >= 25 && result <= 32)
      return "+1D4";
    else if(result >= 33 && result <= 40)
      return "+1D6";
    else
      return "Error!";
  }

  /**
   * Calculate the money the character has availabale
   * @param era - The era of play
   * @return The amount of money available
   */
  private int calcMoneyAvailable(int era){
    int roll = generator.d10();
    if(era == 1890)
      return roll * 500;
    else if(era == 1920){
      if(roll == 1)
        return 4500;
      else
        return (4500 + (1000 * (roll - 1)));
    }
    else if(era == 1990){
      if(roll == 1)
        return 15000;
      else
        return (15000 + (10000 * (roll - 1)));
    }
    else
      return -1;
  }

  /**
   * Calculate the character's hitpoints
   * @return The number of hitpoints
   */
  private int calcHitPoints(){
    if((constitution + size) % 2 != 0)
      return (((constitution + size) / 2) + 1);
    else
      return ((constitution + size) / 2);
  }
}