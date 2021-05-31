package charactergenerator.gui;

import charactergenerator.PlayerCharacter;
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import javax.swing.border.*;

/**
 * Title:        MainGUI
 * Description:  The main GUI class for the CoC Character Generator
 * Copyright:    Copyright (c) 2002
 * Company:
 * @author Markus Svensson
 * @version 1.2
 */

public class MainGUI extends JFrame {
  private final int NO_COMMAND = 0;
  private final int ROLL = 1;
  private final int CLEAR = 2;
  private final int ABOUT = 3;
  private final int QUIT = 4;
  private final int HELP = 5;
  private final int SKILLS = 6;
  private final int SAVE = 7;

  private int command = NO_COMMAND;
  private int era = 0;

  private BorderLayout bottonLayout = new BorderLayout();
  private JPanel northPanel = new JPanel();
  private JPanel westPanel = new JPanel();
  private JPanel eastPanel = new JPanel();
  private JPanel centerPanel = new JPanel();
  private JPanel southPanel = new JPanel();
  private JPanel leftPanel = new JPanel();
  private JPanel rightPanel = new JPanel();
  private GridLayout gridLayout1 = new GridLayout();
  private GridLayout gridLayout2 = new GridLayout();
  private JTextField knowTF = new JTextField();
  private JTextField dbTF = new JTextField();
  private JTextField conTF = new JTextField();
  private JTextField intTF = new JTextField();
  private JTextField ideaTF = new JTextField();
  private JTextField luckTF = new JTextField();
  private JTextField sizTF = new JTextField();
  private JTextField strTF = new JTextField();
  private JTextField powTF = new JTextField();
  private JTextField sanTF = new JTextField();
  private JTextField eduTF = new JTextField();
  private JTextField dexTF = new JTextField();
  private JTextField appTF = new JTextField();
  private JLabel ideaLabel = new JLabel();
  private JLabel luckLabel = new JLabel();
  private JLabel knowLabel = new JLabel();
  private JLabel dbLabel = new JLabel();
  private JLabel strLabel = new JLabel();
  private JLabel eduLabel = new JLabel();
  private JLabel dexLabel = new JLabel();
  private JLabel appLabel = new JLabel();
  private JLabel conLabel = new JLabel();
  private JLabel intLabel = new JLabel();
  private JLabel powLabel = new JLabel();
  private JLabel sanLabel = new JLabel();
  private JLabel sizLabel = new JLabel();
  private JButton rollButton = new JButton();
  private JButton clearButton = new JButton();
  private JMenuBar menuBar = new JMenuBar();
  private JMenu fileMenu = new JMenu();
  private JMenuItem rollMenuItem = new JMenuItem();
  private JMenuItem quitMenuItem = new JMenuItem();
  private JMenu helpMenu = new JMenu();
  private JMenuItem aboutMenuItem = new JMenuItem();
  private JMenuItem clearMenuItem = new JMenuItem();
  private JComboBox eraComboBox = new JComboBox(new Integer[] {new Integer(1920), new Integer(1890), new Integer(1990)});
  private JLabel eraLabel = new JLabel();
  private GridLayout gridLayout3 = new GridLayout();
  private JLabel hpLabel = new JLabel();
  private JLabel mpLabel = new JLabel();
  private JLabel maLabel = new JLabel();
  private JLabel opLabel = new JLabel();
  private JLabel piLabel = new JLabel();
  private JLabel ageLabel = new JLabel();
  private JTextField maTF = new JTextField();
  private JTextField hpTF = new JTextField();
  private JTextField mpTF = new JTextField();
  private JTextField piTF = new JTextField();
  private JTextField opTF = new JTextField();
  private JTextField ageTF = new JTextField();
  private JMenuItem helpMenuItem = new JMenuItem();
  private JButton skillsButton = new JButton();
  private JMenuItem saveMenuItem = new JMenuItem();

  /**
   * Constructor
   */
  public MainGUI() {
      jbInit();
      this.setSize(450, 400);
  }

  /**
   * Init the gui
   */
  private void jbInit(){
    setTitle("CoC Character Generator");
    this.setDefaultCloseOperation(3);
    this.setJMenuBar(menuBar);
    this.getContentPane().setLayout(bottonLayout);
    centerPanel.setLayout(gridLayout2);
    gridLayout1.setColumns(2);
    gridLayout1.setRows(0);
    gridLayout1.setHgap(2);
    gridLayout1.setVgap(2);
    leftPanel.setLayout(gridLayout1);
    leftPanel.setBorder(BorderFactory.createEtchedBorder());
    knowTF.setBorder(BorderFactory.createEtchedBorder());
    knowTF.setEditable(false);
    knowTF.setText("--");
    knowTF.setHorizontalAlignment(SwingConstants.CENTER);
    dbTF.setBorder(BorderFactory.createEtchedBorder());
    dbTF.setEditable(false);
    dbTF.setText("--");
    dbTF.setHorizontalAlignment(SwingConstants.CENTER);
    conTF.setBorder(BorderFactory.createEtchedBorder());
    conTF.setEditable(false);
    conTF.setText("--");
    conTF.setHorizontalAlignment(SwingConstants.CENTER);
    intTF.setBorder(BorderFactory.createEtchedBorder());
    intTF.setEditable(false);
    intTF.setText("--");
    intTF.setHorizontalAlignment(SwingConstants.CENTER);
    ideaTF.setBorder(BorderFactory.createEtchedBorder());
    ideaTF.setEditable(false);
    ideaTF.setText("--");
    ideaTF.setHorizontalAlignment(SwingConstants.CENTER);
    luckTF.setBorder(BorderFactory.createEtchedBorder());
    luckTF.setEditable(false);
    luckTF.setText("--");
    luckTF.setHorizontalAlignment(SwingConstants.CENTER);
    sizTF.setBorder(BorderFactory.createEtchedBorder());
    sizTF.setEditable(false);
    sizTF.setText("--");
    sizTF.setHorizontalAlignment(SwingConstants.CENTER);
    strTF.setBorder(BorderFactory.createEtchedBorder());
    strTF.setEditable(false);
    strTF.setText("--");
    strTF.setHorizontalAlignment(SwingConstants.CENTER);
    powTF.setBorder(BorderFactory.createEtchedBorder());
    powTF.setEditable(false);
    powTF.setText("--");
    powTF.setHorizontalAlignment(SwingConstants.CENTER);
    sanTF.setBorder(BorderFactory.createEtchedBorder());
    sanTF.setEditable(false);
    sanTF.setText("--");
    sanTF.setHorizontalAlignment(SwingConstants.CENTER);
    eduTF.setBorder(BorderFactory.createEtchedBorder());
    eduTF.setEditable(false);
    eduTF.setText("--");
    eduTF.setHorizontalAlignment(SwingConstants.CENTER);
    dexTF.setBorder(BorderFactory.createEtchedBorder());
    dexTF.setEditable(false);
    dexTF.setText("--");
    dexTF.setHorizontalAlignment(SwingConstants.CENTER);
    appTF.setBorder(BorderFactory.createEtchedBorder());
    appTF.setEditable(false);
    appTF.setText("--");
    appTF.setHorizontalAlignment(SwingConstants.CENTER);
    ideaLabel.setBorder(BorderFactory.createEtchedBorder());
    ideaLabel.setToolTipText("Idea (INT*5)");
    ideaLabel.setHorizontalAlignment(SwingConstants.CENTER);
    ideaLabel.setText("Idea");
    luckLabel.setBorder(BorderFactory.createEtchedBorder());
    luckLabel.setToolTipText("Luck (POW*5)");
    luckLabel.setHorizontalAlignment(SwingConstants.CENTER);
    luckLabel.setText("Luck");
    knowLabel.setBorder(BorderFactory.createEtchedBorder());
    knowLabel.setToolTipText("Knowledge (EDU*5)");
    knowLabel.setHorizontalAlignment(SwingConstants.CENTER);
    knowLabel.setText("Know");
    dbLabel.setBorder(BorderFactory.createEtchedBorder());
    dbLabel.setToolTipText("Damage Bonus");
    dbLabel.setHorizontalAlignment(SwingConstants.CENTER);
    dbLabel.setText("Dam. Bonus");
    strLabel.setBorder(BorderFactory.createEtchedBorder());
    strLabel.setToolTipText("Strength (3D6)");
    strLabel.setHorizontalAlignment(SwingConstants.CENTER);
    strLabel.setText("STR");
    eduLabel.setBorder(BorderFactory.createEtchedBorder());
    eduLabel.setToolTipText("Education (3D6+3)");
    eduLabel.setHorizontalAlignment(SwingConstants.CENTER);
    eduLabel.setText("EDU");
    dexLabel.setBorder(BorderFactory.createEtchedBorder());
    dexLabel.setToolTipText("Dexterity (3D6)");
    dexLabel.setHorizontalAlignment(SwingConstants.CENTER);
    dexLabel.setText("DEX");
    appLabel.setBorder(BorderFactory.createEtchedBorder());
    appLabel.setToolTipText("Apperance (3D6)");
    appLabel.setHorizontalAlignment(SwingConstants.CENTER);
    appLabel.setText("APP");
    conLabel.setBorder(BorderFactory.createEtchedBorder());
    conLabel.setToolTipText("Constitution (3D6)");
    conLabel.setHorizontalAlignment(SwingConstants.CENTER);
    conLabel.setText("CON");
    intLabel.setBorder(BorderFactory.createEtchedBorder());
    intLabel.setToolTipText("Inteligence (2D6+6)");
    intLabel.setHorizontalAlignment(SwingConstants.CENTER);
    intLabel.setText("INT");
    powLabel.setBorder(BorderFactory.createEtchedBorder());
    powLabel.setToolTipText("Power (3D6)");
    powLabel.setHorizontalAlignment(SwingConstants.CENTER);
    powLabel.setText("POW");
    sanLabel.setBorder(BorderFactory.createEtchedBorder());
    sanLabel.setToolTipText("Sanity (POW*5)");
    sanLabel.setHorizontalAlignment(SwingConstants.CENTER);
    sanLabel.setText("SAN");
    sizLabel.setBorder(BorderFactory.createEtchedBorder());
    sizLabel.setToolTipText("Size (2D6+6)");
    sizLabel.setHorizontalAlignment(SwingConstants.CENTER);
    sizLabel.setText("SIZ");
    rollButton.setToolTipText("Roll dice");
    rollButton.setText("Roll");
    rollButton.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        rollButton_mouseReleased(e);
      }
    });
    rightPanel.setBorder(BorderFactory.createEtchedBorder());
    gridLayout3.setColumns(2);
    gridLayout3.setRows(0);
    gridLayout3.setHgap(2);
    gridLayout3.setVgap(2);
    rightPanel.setLayout(gridLayout3);
    clearButton.setToolTipText("Clear");
    clearButton.setText("Clear");
    clearButton.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        clearButton_mouseReleased(e);
      }
    });
    fileMenu.setText("File");
    rollMenuItem.setText("Roll");
    rollMenuItem.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        rollMenuItem_mouseReleased(e);
      }
    });
    saveMenuItem.setText("Save");
    saveMenuItem.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        saveMenuItem_mouseReleased(e);
      }
    });
    quitMenuItem.setText("Quit");
    quitMenuItem.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        quitMenuItem_mouseReleased(e);
      }
    });
    helpMenu.setText("Help");
    aboutMenuItem.setText("About");
    aboutMenuItem.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        aboutMenuItem_mouseReleased(e);
      }
    });
    clearMenuItem.setText("Clear");
    clearMenuItem.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        clearMenuItem_mouseReleased(e);
      }
    });
    eraComboBox.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        eraComboBox_actionPerformed(e);
      }
    });
    eraLabel.setText("Era of play:");
    eraComboBox.setToolTipText("Select your era");
    hpLabel.setBorder(BorderFactory.createEtchedBorder());
    hpLabel.setToolTipText("Hit points");
    hpLabel.setHorizontalAlignment(SwingConstants.CENTER);
    hpLabel.setText("Hit Points");
    mpLabel.setBorder(BorderFactory.createEtchedBorder());
    mpLabel.setToolTipText("Magic points");
    mpLabel.setHorizontalAlignment(SwingConstants.CENTER);
    mpLabel.setText("Magic Points");
    maLabel.setBorder(BorderFactory.createEtchedBorder());
    maLabel.setToolTipText("Money availabale");
    maLabel.setHorizontalAlignment(SwingConstants.CENTER);
    maLabel.setText("Money Ava.");
    opLabel.setBorder(BorderFactory.createEtchedBorder());
    opLabel.setToolTipText("Occupation skill points");
    opLabel.setHorizontalAlignment(SwingConstants.CENTER);
    opLabel.setText("Occ. Points");
    piLabel.setBorder(BorderFactory.createEtchedBorder());
    piLabel.setToolTipText("Personal interest skill points");
    piLabel.setHorizontalAlignment(SwingConstants.CENTER);
    piLabel.setText("Pi. Points");
    ageLabel.setBorder(BorderFactory.createEtchedBorder());
    ageLabel.setToolTipText("Minimum age");
    ageLabel.setHorizontalAlignment(SwingConstants.CENTER);
    ageLabel.setText("Min. Age");
    maTF.setBorder(BorderFactory.createEtchedBorder());
    maTF.setEditable(false);
    maTF.setText("--");
    maTF.setHorizontalAlignment(SwingConstants.CENTER);
    hpTF.setBorder(BorderFactory.createEtchedBorder());
    hpTF.setEditable(false);
    hpTF.setText("--");
    hpTF.setHorizontalAlignment(SwingConstants.CENTER);
    mpTF.setBorder(BorderFactory.createEtchedBorder());
    mpTF.setEditable(false);
    mpTF.setText("--");
    mpTF.setHorizontalAlignment(SwingConstants.CENTER);
    piTF.setBorder(BorderFactory.createEtchedBorder());
    piTF.setEditable(false);
    piTF.setText("--");
    piTF.setHorizontalAlignment(SwingConstants.CENTER);
    opTF.setBorder(BorderFactory.createEtchedBorder());
    opTF.setEditable(false);
    opTF.setText("--");
    opTF.setHorizontalAlignment(SwingConstants.CENTER);
    ageTF.setBorder(BorderFactory.createEtchedBorder());
    ageTF.setEditable(false);
    ageTF.setText("--");
    ageTF.setHorizontalAlignment(SwingConstants.CENTER);
    helpMenuItem.setText("Help");
    helpMenuItem.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        helpMenuItem_mouseReleased(e);
      }
    });
    skillsButton.setEnabled(false);
    skillsButton.setToolTipText("Distribute skill points");
    skillsButton.setText("Skills");
    skillsButton.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        skillsButton_mouseReleased(e);
      }
    });
    rightPanel.add(ageLabel, null);
    rightPanel.add(ageTF, null);
    rightPanel.add(opLabel, null);
    rightPanel.add(opTF, null);
    rightPanel.add(piLabel, null);
    rightPanel.add(piTF, null);
    rightPanel.add(mpLabel, null);
    rightPanel.add(mpTF, null);
    rightPanel.add(hpLabel, null);
    rightPanel.add(hpTF, null);
    rightPanel.add(maLabel, null);
    rightPanel.add(maTF, null);
    centerPanel.add(leftPanel, null);
    leftPanel.add(appLabel, null);
    leftPanel.add(appTF, null);
    leftPanel.add(conLabel, null);
    leftPanel.add(conTF, null);
    leftPanel.add(dexLabel, null);
    leftPanel.add(dexTF, null);
    leftPanel.add(eduLabel, null);
    leftPanel.add(eduTF, null);
    leftPanel.add(intLabel, null);
    leftPanel.add(intTF, null);
    leftPanel.add(powLabel, null);
    leftPanel.add(powTF, null);
    leftPanel.add(sizLabel, null);
    leftPanel.add(sizTF, null);
    leftPanel.add(strLabel, null);
    leftPanel.add(strTF, null);
    leftPanel.add(sanLabel, null);
    leftPanel.add(sanTF, null);
    leftPanel.add(dbLabel, null);
    leftPanel.add(dbTF, null);
    leftPanel.add(ideaLabel, null);
    leftPanel.add(ideaTF, null);
    leftPanel.add(luckLabel, null);
    leftPanel.add(luckTF, null);
    leftPanel.add(knowLabel, null);
    leftPanel.add(knowTF, null);
    this.getContentPane().add(northPanel, BorderLayout.NORTH);
    northPanel.add(eraLabel, null);
    northPanel.add(eraComboBox, null);
    northPanel.add(skillsButton, null);
    this.getContentPane().add(westPanel, BorderLayout.WEST);
    this.getContentPane().add(eastPanel, BorderLayout.EAST);
    this.getContentPane().add(centerPanel, BorderLayout.CENTER);
    centerPanel.add(rightPanel, null);
    rightPanel.add(rollButton, null);
    this.getContentPane().add(southPanel,  BorderLayout.SOUTH);
    rightPanel.add(clearButton, null);
    menuBar.add(fileMenu);
    menuBar.add(helpMenu);
    fileMenu.add(rollMenuItem);
    fileMenu.add(clearMenuItem);
    fileMenu.add(clearMenuItem);
    fileMenu.addSeparator();
    fileMenu.add(saveMenuItem);
    fileMenu.addSeparator();
    fileMenu.add(quitMenuItem);
    helpMenu.add(helpMenuItem);
    helpMenu.addSeparator();
    helpMenu.add(aboutMenuItem);
  }

  /**
   * Action handler
   * @param e - The event
   */
  synchronized void rollButton_mouseReleased(MouseEvent e) {
    if(rollButton.isEnabled()){
      command = ROLL;
      this.skillsButton.setEnabled(true);
      notify();
    }
  }

  /**
   * Action handler
   * @param e - The event
   */
  synchronized void clearButton_mouseReleased(MouseEvent e) {
    command = CLEAR;
    this.skillsButton.setEnabled(false);
    this.rollButton.setEnabled(true);
    this.rollMenuItem.setEnabled(true);
    notify();
  }

  /**
   * Action handler
   * @param e - The event
   */
  synchronized void rollMenuItem_mouseReleased(MouseEvent e) {
    if(rollMenuItem.isEnabled()){
      command = ROLL;
      this.skillsButton.setEnabled(true);
      notify();
    }
  }

  /**
   * Action handler
   * @param e - The event
   */
  synchronized void clearMenuItem_mouseReleased(MouseEvent e) {
    command = CLEAR;
    this.skillsButton.setEnabled(false);
    this.rollButton.setEnabled(true);
    this.rollMenuItem.setEnabled(true);
    notify();
  }

  /**
    * Action handler
    * @param e - The event
    */
   synchronized void saveMenuItem_mouseReleased(MouseEvent e) {
     command = SAVE;
     notify();
  }

  /**
   * Action handler
   * @param e - The event
   */
  synchronized void quitMenuItem_mouseReleased(MouseEvent e) {
    command = QUIT;
    notify();
  }

  /**
   * Action handler
   * @param e - The event
   */
  synchronized void helpMenuItem_mouseReleased(MouseEvent e) {
    command = HELP;
    notify();
  }

  /**
   * Action handler
   * @param e - The event
   */
  synchronized void aboutMenuItem_mouseReleased(MouseEvent e) {
    command = ABOUT;
    notify();
  }

  /**
   * Action handler
   * @param e - The event
   */
  void eraComboBox_actionPerformed(ActionEvent e) {
    Integer temp = (Integer)eraComboBox.getSelectedItem();
    era = temp.intValue();
  }

  /**
   * Action handler
   * @param e - The event
   */
  public synchronized void skillsButton_mouseReleased(MouseEvent e) {
    if(skillsButton.isEnabled()){
      command = SKILLS;
      this.rollMenuItem.setEnabled(false);
      this.rollButton.setEnabled(false);
      notify();
    }
  }

  /**
   * Reset the command
   */
  public synchronized void resetCommand(){
    command = NO_COMMAND;
  }

  /**
   * Get the command
   * @return The command
   */
  public synchronized int getCommand(){
    if(command == NO_COMMAND){
      try{
        wait();
      }catch(InterruptedException ie){};
    }
    return command;
  }

  /**
   * Clear all text areas
   */
  public void clear(){
    knowTF.setText("--");
    dbTF.setText("--");
    conTF.setText("--");
    intTF.setText("--");
    ideaTF.setText("--");
    luckTF.setText("--");
    sizTF.setText("--");
    strTF.setText("--");
    powTF.setText("--");
    sanTF.setText("--");
    eduTF.setText("--");
    dexTF.setText("--");
    appTF.setText("--");
    maTF.setText("--");
    hpTF.setText("--");
    mpTF.setText("--");
    piTF.setText("--");
    opTF.setText("--");
    ageTF.setText("--");
  }

  /**
   * Get the stats and print them at the right place
   * @param stats - The Character
   */
  public void displayStats(PlayerCharacter stats){
    knowTF.setText(""+stats.getKnow());
    dbTF.setText(""+stats.getDamageBonus());
    conTF.setText(""+stats.getConstitution());
    intTF.setText(""+stats.getIntelligence());
    ideaTF.setText(""+stats.getIdea());
    luckTF.setText(""+stats.getLuck());
    sizTF.setText(""+stats.getSize());
    strTF.setText(""+stats.getStrength());
    powTF.setText(""+stats.getPower());
    sanTF.setText(""+stats.getSanity());
    eduTF.setText(""+stats.getEducation());
    dexTF.setText(""+stats.getDexterity());
    appTF.setText(""+stats.getApperance());
    maTF.setText(""+stats.getMoneyAvailable());
    hpTF.setText(""+stats.getHitPoints());
    mpTF.setText(""+stats.getMagicPoints());
    piTF.setText(""+stats.getPersonalInterestPoints());
    opTF.setText(""+stats.getOccupationPoints());
    ageTF.setText(""+stats.getMinAge());
  }

  /**
   * Get the era
   * @return The era
   */
  public int getEra(){
    if(era == 0)
      return 1920;
    else
      return era;
  }
}