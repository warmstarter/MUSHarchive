package charactergenerator;
import charactergenerator.gui.*;
import java.io.*;
import java.awt.*;
import javax.swing.*;
import java.util.*;

/**
 * Title:        Controller
 * Description:  Controller class for the CoC Character Generator
 * Copyright:    Copyright (c) 2002
 * Company:
 * @author Markus Svensson
 * @version 1.2
 */

public class Controller extends Thread {
  private final int NO_COMMAND = 0;
  private final int ROLL = 1;
  private final int CLEAR = 2;
  private final int ABOUT = 3;
  private final int QUIT = 4;
  private final int HELP = 5;
  private final int SKILLS = 6;
  private final int SAVE = 7;

  private int command;
  private int era;
  private Browser browser;
  private About about;
  private MainGUI gui;
  private PlayerCharacter stats;
  private SkillsFrame skillsGui;

  /**
   * Constructor
   */
  public Controller() {
    gui = new MainGUI();
    gui.setVisible(true);
    run();
  }

  /**
   * Handle a command
   */
  private void handleCommand(){
    if(command == ROLL){
      era = gui.getEra();
      stats = new PlayerCharacter(era);
      stats.generateStats();
      gui.displayStats(stats);
    }
    else if(command == CLEAR)
      gui.clear();
    else if(command == ABOUT)
      about = new About(gui, "About", true);
    else if(command == HELP)
      browser = new Browser("help.html");
    else if(command == SKILLS)
      skillsGui = new SkillsFrame(stats);
    else if(command == QUIT)
      System.exit(0);
    else if(command == SAVE){
     try{
       this.writeFile();
     }catch(IOException IOE){
       System.err.println("Save failed!");
     }
    }
  }

  /**
   * Save the character to a txt-file.
   */
  private void writeFile() throws IOException{
    JFileChooser chooser = new JFileChooser();
    chooser.setDialogType(JFileChooser.SAVE_DIALOG);
    chooser.setCurrentDirectory(new File("."));
    int returnValue = chooser.showSaveDialog(gui);
    if(returnValue == chooser.APPROVE_OPTION){
      File file = chooser.getSelectedFile();
      FileWriter writer = new FileWriter(file);
      writer.write(stats.getString(), 0, stats.getString().length());
      writer.flush();
      writer.close();
    }
  }

  /**
   * Run the thread
   */
  public void run(){
    while(true){
      command = gui.getCommand();
      gui.resetCommand();
      handleCommand();
    }
  }
}