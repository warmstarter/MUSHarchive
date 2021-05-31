package charactergenerator.gui;

import charactergenerator.PlayerCharacter;
import java.awt.*;
import javax.swing.*;
import java.util.Vector;
import java.io.*;
import java.awt.event.*;

/**
 * Title:        CoC Character Generator
 * Description:  This is the skill point allocation window, available since V1.1
 * Copyright:    Copyright (c) 2002
 * Company:
 * @author Markus Svensson
 * @since 1.1
 * @version 1.2
 */

public class SkillsFrame extends JFrame {
  private JPanel topPanel = new JPanel();
  private JPanel centerPanel = new JPanel();
  private JPanel bottomPanel = new JPanel();
  private JPanel rightPanel = new JPanel();
  private JPanel leftPanel = new JPanel();
  private JComboBox occComboBox;
  private JLabel occLabel = new JLabel();
  private BorderLayout borderLayout1 = new BorderLayout();
  private JLabel statusLabel = new JLabel();
  private BorderLayout borderLayout2 = new BorderLayout();
  private GridLayout gridLayout1 = new GridLayout();
  private JLabel spLabel = new JLabel();
  private JTextField spTextField = new JTextField();
  private JLabel piLabel = new JLabel();
  private JTextField piTextField = new JTextField();
  private JPanel cenPanel = new JPanel();
 private JLabel occLabel1 = new JLabel();
 private JLabel occLabel2 = new JLabel();
 private JLabel occLabel3 = new JLabel();
 private JLabel occLabel4 = new JLabel();
 private JLabel occLabel5 = new JLabel();
 private JLabel occLabel6 = new JLabel();
 private JLabel occLabel7 = new JLabel();
 private JLabel occLabel8 = new JLabel();
 private JLabel occLabel9 = new JLabel();
 private JLabel occLabel10 = new JLabel();
 private JLabel occLabel11 = new JLabel();
 private GridLayout gridLayout2 = new GridLayout();
 private JTextField occTF15 = new JTextField();
 private JTextField occTF16 = new JTextField();
 private JTextField occTF14 = new JTextField();
 private JTextField occTF11 = new JTextField();
 private JTextField occTF17 = new JTextField();
 private JTextField occTF3 = new JTextField();
 private JTextField occTF1 = new JTextField();
 private JTextField occTF5 = new JTextField();
 private JTextField occTF12 = new JTextField();
 private JTextField occTF10 = new JTextField();
 private JTextField occTF13 = new JTextField();
 private JLabel occLabel13 = new JLabel();
 private JLabel occLabel12 = new JLabel();
 private JLabel occLabel14 = new JLabel();
 private JLabel occLabel15 = new JLabel();
 private JLabel occLabel16 = new JLabel();
 private JLabel occLabel17 = new JLabel();
 private JTextField occTF9 = new JTextField();
 private JTextField occTF8 = new JTextField();
 private JTextField occTF7 = new JTextField();
 private JTextField occTF6 = new JTextField();
 private JTextField occTF4 = new JTextField();
 private JTextField occTF2 = new JTextField();

  private int occPoints;
  private int piPoints;
  private int oldValue, oldValue1, oldValue2, oldValue3, oldValue4, oldValue5, oldValue6,
  oldValue7, oldValue8, oldValue9, oldValue10, oldValue11, oldValue12, oldValue13, oldValue14,
  oldValue15, oldValue16, oldValue17;
  private int numOfPersSkills;
  private int orgValue;
  private Vector occupations = new Vector();
  private Vector skillsVector = new Vector();
  private String selectedOccupation;
  private PlayerCharacter character;
  private dlgToHigh errorDlg;

  /**
   * Constructor
   * @param character The character
   */
  public SkillsFrame(PlayerCharacter character){
    //this();
    this.character = character;
    this.occPoints = character.getOccupationPoints();
    this.piPoints = character.getPersonalInterestPoints();
    this.orgValue = occPoints;
    getOccupations();
    jbInit();
    this.setTitle("Occupation and Skills");
    this.setSize(450, 500);
    this.setVisible(true);
    errorDlg = new dlgToHigh(this, "", true);
  }

  /**
   * Init the GUI
   */
  private void jbInit(){
    topPanel.setLayout(borderLayout1);
    occComboBox = new JComboBox(occupations);
    occComboBox.setToolTipText("Choose your occupation");
    occComboBox.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occComboBox_actionPerformed(e);
      }
    });
    occLabel.setRequestFocusEnabled(false);
    occLabel.setToolTipText("Occupation");
    occLabel.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel.setHorizontalTextPosition(SwingConstants.CENTER);
    occLabel.setText("Occ:");
    leftPanel.setBorder(BorderFactory.createEtchedBorder());
    leftPanel.setRequestFocusEnabled(false);
    rightPanel.setBorder(BorderFactory.createEtchedBorder());
    rightPanel.setRequestFocusEnabled(false);
    bottomPanel.setBorder(BorderFactory.createEtchedBorder());
    bottomPanel.setRequestFocusEnabled(false);
    bottomPanel.setLayout(borderLayout2);
    statusLabel.setRequestFocusEnabled(false);
    statusLabel.setText("Skills");
    centerPanel.setLayout(gridLayout1);
    gridLayout1.setColumns(2);
    gridLayout1.setRows(0);
    gridLayout1.setHgap(2);
    gridLayout1.setVgap(2);
    spLabel.setRequestFocusEnabled(false);
    spLabel.setText("SP:");
    spTextField.setBorder(BorderFactory.createEtchedBorder());
    spTextField.setRequestFocusEnabled(false);
    spTextField.setToolTipText("Skill points available");
    spTextField.setEditable(false);
    spTextField.setText("" + occPoints);
    piLabel.setRequestFocusEnabled(false);
    piLabel.setText("PiP:");
    piTextField.setBorder(BorderFactory.createEtchedBorder());
    piTextField.setRequestFocusEnabled(false);
    piTextField.setToolTipText("Personal interest points available");
    piTextField.setEditable(false);
    piTextField.setText("" + piPoints);
    occLabel1.setBorder(BorderFactory.createEtchedBorder());
    occLabel1.setRequestFocusEnabled(false);
    occLabel1.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel2.setBorder(BorderFactory.createEtchedBorder());
    occLabel2.setRequestFocusEnabled(false);
    occLabel2.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel3.setBorder(BorderFactory.createEtchedBorder());
    occLabel3.setRequestFocusEnabled(false);
    occLabel3.setHorizontalAlignment(SwingConstants.CENTER);
    cenPanel.setLayout(gridLayout2);
    occLabel4.setBorder(BorderFactory.createEtchedBorder());
    occLabel4.setRequestFocusEnabled(false);
    occLabel4.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel5.setBorder(BorderFactory.createEtchedBorder());
    occLabel5.setRequestFocusEnabled(false);
    occLabel5.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel6.setBorder(BorderFactory.createEtchedBorder());
    occLabel6.setRequestFocusEnabled(false);
    occLabel6.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel7.setBorder(BorderFactory.createEtchedBorder());
    occLabel7.setRequestFocusEnabled(false);
    occLabel7.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel8.setBorder(BorderFactory.createEtchedBorder());
    occLabel8.setRequestFocusEnabled(false);
    occLabel8.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel9.setBorder(BorderFactory.createEtchedBorder());
    occLabel9.setRequestFocusEnabled(false);
    occLabel9.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel10.setBorder(BorderFactory.createEtchedBorder());
    occLabel10.setRequestFocusEnabled(false);
    occLabel10.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel11.setBorder(BorderFactory.createEtchedBorder());
    occLabel11.setRequestFocusEnabled(false);
    occLabel11.setHorizontalAlignment(SwingConstants.CENTER);
    gridLayout2.setColumns(2);
    gridLayout2.setHgap(2);
    gridLayout2.setRows(0);
    gridLayout2.setVgap(2);
    occLabel13.setBorder(BorderFactory.createEtchedBorder());
    occLabel13.setRequestFocusEnabled(false);
    occLabel13.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel12.setBorder(BorderFactory.createEtchedBorder());
    occLabel12.setRequestFocusEnabled(false);
    occLabel12.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel14.setBorder(BorderFactory.createEtchedBorder());
    occLabel14.setRequestFocusEnabled(false);
    occLabel14.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel15.setBorder(BorderFactory.createEtchedBorder());
    occLabel15.setRequestFocusEnabled(false);
    occLabel15.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel15.setHorizontalTextPosition(SwingConstants.LEADING);
    occLabel16.setBorder(BorderFactory.createEtchedBorder());
    occLabel16.setRequestFocusEnabled(false);
    occLabel16.setHorizontalAlignment(SwingConstants.CENTER);
    occLabel17.setBorder(BorderFactory.createEtchedBorder());
    occLabel17.setRequestFocusEnabled(false);
    occLabel17.setHorizontalAlignment(SwingConstants.CENTER);
    cenPanel.setRequestFocusEnabled(false);
    centerPanel.setRequestFocusEnabled(false);
    occTF1.setEnabled(false);
    occTF1.setHorizontalAlignment(SwingConstants.CENTER);
    occTF1.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF1_focusLost(e);
      }
    });
    occTF1.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF1_actionPerformed(e);
      }
    });
    occTF2.setEnabled(false);
    occTF2.setHorizontalAlignment(SwingConstants.CENTER);
    occTF2.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF2_focusLost(e);
      }
    });
    occTF2.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF2_actionPerformed(e);
      }
    });
    occTF3.setEnabled(false);
    occTF3.setHorizontalAlignment(SwingConstants.CENTER);
    occTF3.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF3_focusLost(e);
      }
    });
    occTF3.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF3_actionPerformed(e);
      }
    });
    occTF4.setEnabled(false);
    occTF4.setHorizontalAlignment(SwingConstants.CENTER);
    occTF4.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF4_focusLost(e);
      }
    });
    occTF4.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF4_actionPerformed(e);
      }
    });
    occTF5.setEnabled(false);
    occTF5.setHorizontalAlignment(SwingConstants.CENTER);
    occTF5.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF5_focusLost(e);
      }
    });
    occTF5.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF5_actionPerformed(e);
      }
    });
    occTF6.setEnabled(false);
    occTF6.setHorizontalAlignment(SwingConstants.CENTER);
    occTF6.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF6_focusLost(e);
      }
    });
    occTF6.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF6_actionPerformed(e);
      }
    });
    occTF7.setEnabled(false);
    occTF7.setHorizontalAlignment(SwingConstants.CENTER);
    occTF7.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF7_focusLost(e);
      }
    });
    occTF7.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF7_actionPerformed(e);
      }
    });
    occTF8.setEnabled(false);
    occTF8.setHorizontalAlignment(SwingConstants.CENTER);
    occTF8.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF8_focusLost(e);
      }
    });
    occTF8.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF8_actionPerformed(e);
      }
    });
    occTF9.setEnabled(false);
    occTF9.setHorizontalAlignment(SwingConstants.CENTER);
    occTF9.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF9_focusLost(e);
      }
    });
    occTF9.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF9_actionPerformed(e);
      }
    });
    occTF10.setEnabled(false);
    occTF10.setHorizontalAlignment(SwingConstants.CENTER);
    occTF10.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF10_focusLost(e);
      }
    });
    occTF10.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF10_actionPerformed(e);
      }
    });
    occTF11.setEnabled(false);
    occTF11.setHorizontalAlignment(SwingConstants.CENTER);
    occTF11.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF11_focusLost(e);
      }
    });
    occTF11.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF11_actionPerformed(e);
      }
    });
    occTF12.setEnabled(false);
    occTF12.setHorizontalAlignment(SwingConstants.CENTER);
    occTF12.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF12_focusLost(e);
      }
    });
    occTF12.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF12_actionPerformed(e);
      }
    });
    occTF13.setEnabled(false);
    occTF13.setHorizontalAlignment(SwingConstants.CENTER);
    occTF13.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF13_focusLost(e);
      }
    });
    occTF13.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF13_actionPerformed(e);
      }
    });
    occTF14.setEnabled(false);
    occTF14.setHorizontalAlignment(SwingConstants.CENTER);
    occTF14.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF14_focusLost(e);
      }
    });
    occTF14.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF14_actionPerformed(e);
      }
    });
    occTF15.setEnabled(false);
    occTF15.setHorizontalAlignment(SwingConstants.CENTER);
    occTF15.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF15_focusLost(e);
      }
    });
    occTF15.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF15_actionPerformed(e);
      }
    });
    occTF16.setEnabled(false);
    occTF16.setHorizontalAlignment(SwingConstants.CENTER);
    occTF16.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF16_focusLost(e);
      }
    });
    occTF16.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF16_actionPerformed(e);
      }
    });
    occTF17.setEnabled(false);
    occTF17.setHorizontalAlignment(SwingConstants.CENTER);
    occTF17.addFocusListener(new java.awt.event.FocusAdapter() {
      public void focusLost(FocusEvent e) {
        occTF17_focusLost(e);
      }
    });
    occTF17.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        occTF17_actionPerformed(e);
      }
    });
    topPanel.add(leftPanel,  BorderLayout.WEST);
    leftPanel.add(occLabel, null);
    leftPanel.add(occComboBox, null);
    topPanel.add(rightPanel,  BorderLayout.CENTER);
    rightPanel.add(spLabel, null);
    this.getContentPane().add(topPanel, BorderLayout.NORTH);
    this.getContentPane().add(bottomPanel, BorderLayout.SOUTH);
    bottomPanel.add(statusLabel, BorderLayout.CENTER);
    this.getContentPane().add(cenPanel, BorderLayout.CENTER);
    rightPanel.add(spTextField, null);
    rightPanel.add(piLabel, null);
    rightPanel.add(piTextField, null);
    cenPanel.add(occLabel1, null);
    cenPanel.add(occTF1, null);
    cenPanel.add(occLabel2, null);
    cenPanel.add(occTF2, null);
    cenPanel.add(occLabel3, null);
    cenPanel.add(occTF3, null);
    cenPanel.add(occLabel4, null);
    cenPanel.add(occTF4, null);
    cenPanel.add(occLabel5, null);
    cenPanel.add(occTF5, null);
    cenPanel.add(occLabel6, null);
    cenPanel.add(occTF6, null);
    cenPanel.add(occLabel7, null);
    cenPanel.add(occTF7, null);
    cenPanel.add(occLabel8, null);
    cenPanel.add(occTF8, null);
    cenPanel.add(occLabel9, null);
    cenPanel.add(occTF9, null);
    cenPanel.add(occLabel10, null);
    cenPanel.add(occTF10, null);
    cenPanel.add(occLabel11, null);
    cenPanel.add(occTF11, null);
    cenPanel.add(occLabel12, null);
    cenPanel.add(occTF12, null);
    cenPanel.add(occLabel13, null);
    cenPanel.add(occTF13, null);
    cenPanel.add(occLabel14, null);
    cenPanel.add(occTF14, null);
    cenPanel.add(occLabel15, null);
    cenPanel.add(occTF15, null);
    cenPanel.add(occLabel16, null);
    cenPanel.add(occTF16, null);
    cenPanel.add(occLabel17, null);
    cenPanel.add(occTF17, null);
  }

  /**
   * Event handler for the JComboBox
   * @param e - The event
   */
  void occComboBox_actionPerformed(ActionEvent e) {
    occPoints = orgValue;
    spTextField.setText("" + occPoints);
    selectedOccupation = (String)occComboBox.getSelectedItem();
    getSkills();
    setupSkills();
  }

  /**
   * Set all text fields as disabled and clear all text
   */
  void setAllDisabled(){
    occTF1.setText("0");
    occLabel1.setText("");
    occTF1.setEnabled(false);
    occTF2.setText("0");
    occLabel2.setText("");
    occTF2.setEnabled(false);
    occTF3.setText("0");
    occLabel3.setText("");
    occTF3.setEnabled(false);
    occTF4.setText("0");
    occLabel4.setText("");
    occTF4.setEnabled(false);
    occTF5.setText("0");
    occLabel5.setText("");
    occTF5.setEnabled(false);
    occTF6.setText("0");
    occLabel6.setText("");
    occTF6.setEnabled(false);
    occTF7.setText("0");
    occLabel7.setText("");
    occTF7.setEnabled(false);
    occTF8.setText("0");
    occLabel8.setText("");
    occTF8.setEnabled(false);
    occTF9.setText("0");
    occLabel9.setText("");
    occTF9.setEnabled(false);
    occTF10.setText("0");
    occLabel10.setText("");
    occTF10.setEnabled(false);
    occTF11.setText("0");
    occLabel11.setText("");
    occTF11.setEnabled(false);
    occTF12.setText("0");
    occLabel12.setText("");
    occTF12.setEnabled(false);
    occTF13.setText("0");
    occLabel13.setText("");
    occTF13.setEnabled(false);
    occTF14.setText("0");
    occLabel14.setText("");
    occTF14.setEnabled(false);
    occTF15.setText("0");
    occLabel15.setText("");
    occTF15.setEnabled(false);
    occTF16.setText("0");
    occLabel16.setText("");
    occTF16.setEnabled(false);
    occTF17.setText("0");
    occLabel17.setText("");
    occTF17.setEnabled(false);
  }

  /**
   * Show the skills on the GUI
   */
  private void setupSkills(){
    int i = 0;
    setAllDisabled();
    centerPanel.setVisible(false);
    // Add occupation names to the labels
    for(i = 0; i < skillsVector.size(); i++){
      switch(i){
        case 0:
          occLabel1.setText((String)skillsVector.get(i));
          occTF1.setEnabled(true);
          break;
        case 1:
          occLabel2.setText((String)skillsVector.get(i));
          occTF2.setEnabled(true);
          break;
        case 2:
          occLabel3.setText((String)skillsVector.get(i));
          occTF3.setEnabled(true);
          break;
        case 3:
          occLabel4.setText((String)skillsVector.get(i));
          occTF4.setEnabled(true);
          break;
        case 4:
          occLabel5.setText((String)skillsVector.get(i));
          occTF5.setEnabled(true);
          break;
        case 5:
          occLabel6.setText((String)skillsVector.get(i));
          occTF6.setEnabled(true);
          break;
        case 6:
          occLabel7.setText((String)skillsVector.get(i));
          occTF7.setEnabled(true);
          break;
        case 7:
          occLabel8.setText((String)skillsVector.get(i));
          occTF8.setEnabled(true);
          break;
        case 8:
          occLabel9.setText((String)skillsVector.get(i));
          occTF9.setEnabled(true);
          break;
        case 9:
          occLabel10.setText((String)skillsVector.get(i));
          occTF10.setEnabled(true);
          break;
        case 10:
          occLabel11.setText((String)skillsVector.get(i));
          occTF11.setEnabled(true);
          break;
        case 11:
          occLabel12.setText((String)skillsVector.get(i));
          occTF12.setEnabled(true);
          break;
        case 12:
          occLabel13.setText((String)skillsVector.get(i));
          occTF13.setEnabled(true);
          break;
        case 13:
          occLabel14.setText((String)skillsVector.get(i));
          occTF14.setEnabled(true);
          break;
        case 14:
          occLabel15.setText((String)skillsVector.get(i));
          occTF15.setEnabled(true);
          break;
        case 15:
          occLabel16.setText((String)skillsVector.get(i));
          occTF16.setEnabled(true);
          break;
        case 16:
          occLabel17.setText((String)skillsVector.get(i));
          occTF17.setEnabled(true);
          break;
      }
    }
    // Add the personal skills
    for(int j = i--; j < (numOfPersSkills + i); j++){
      switch(j){
        case 0:
          occLabel1.setText("Personal Skill");
          occTF1.setEnabled(true);
          break;
        case 1:
          occLabel2.setText("Personal Skill");
          occTF2.setEnabled(true);
          break;
        case 2:
          occLabel3.setText("Personal Skill");
          occTF3.setEnabled(true);
          break;
        case 3:
          occLabel4.setText("Personal Skill");
          occTF4.setEnabled(true);
          break;
        case 4:
          occLabel5.setText("Personal Skill");
          occTF5.setEnabled(true);
          break;
        case 5:
          occLabel6.setText("Personal Skill");
          occTF6.setEnabled(true);
          break;
        case 6:
          occLabel7.setText("Personal Skill");
          occTF7.setEnabled(true);
          break;
        case 7:
          occLabel8.setText("Personal Skill");
          occTF8.setEnabled(true);
          break;
        case 8:
          occLabel9.setText("Personal Skill");
          occTF9.setEnabled(true);
          break;
        case 9:
          occLabel10.setText("Personal Skill");
          occTF10.setEnabled(true);
          break;
        case 10:
          occLabel11.setText("Personal Skill");
          occTF11.setEnabled(true);
          break;
        case 11:
          occLabel12.setText("Personal Skill");
          occTF12.setEnabled(true);
          break;
        case 12:
          occLabel13.setText("Personal Skill");
          occTF13.setEnabled(true);
          break;
        case 13:
          occLabel14.setText("Personal Skill");
          occTF14.setEnabled(true);
          break;
        case 14:
          occLabel15.setText("Personal Skill");
          occTF15.setEnabled(true);
          break;
        case 15:
          occLabel16.setText("Personal Skill");
          occTF16.setEnabled(true);
          break;
        case 16:
          occLabel17.setText("Personal Skill");
          occTF17.setEnabled(true);
          break;
      }
    }
    centerPanel.setVisible(true);
  }

  /**
   * Loads all the occupations available from a file and
   * stores them in the Vector occupations.
   */
  private void getOccupations(){
    String occString;

    try{
      // Use InputStreamReader instead of FileReader - looks like it's working...
      InputStreamReader occ = new InputStreamReader(getClass().getResourceAsStream("occ/Occupation"));
      //FileReader occ = new FileReader(getClass().getResource("occ/Occupation").getFile());
      StreamTokenizer occupation = new StreamTokenizer(occ);
      occupation.eolIsSignificant(true);
      occupation.nextToken();
      while(occupation.ttype != StreamTokenizer.TT_EOF){
        occString = occupation.sval;
        occupation.nextToken();
        while(occupation.ttype != StreamTokenizer.TT_EOL && occupation.ttype != StreamTokenizer.TT_EOF){
          occString = occString + " " + occupation.sval;
          occupation.nextToken();
        }
        if(occString != null)
          occupations.add(occString);
        occupation.nextToken();
      }
      if(occ != null)
        occ.close();
    }
    catch(FileNotFoundException fnfex)
    {
      System.out.println("File not found!");
    }
    catch(IOException ioex)
    {
      System.out.println("I/O Failed!");
    }
  }

  /**
   * Loads the skills associated with a occupation from a file
   * with the same name as the occupation. Any blanks in the file
   * name is replaced with underscores (_). The skills are stored in the
   * Vector skillsVector and the number of "personal" skills are stored in the
   * int numOfPersSkills.
   */
  private void getSkills(){
    String skillString;
    skillsVector.clear();
    try{
      String fileName = "occ/"+selectedOccupation;
      fileName = fileName.replace(' ', '_');
      InputStreamReader fr = new InputStreamReader(getClass().getResourceAsStream(fileName));
      StreamTokenizer skills = new StreamTokenizer(fr);
      skills.eolIsSignificant(true);
      skills.nextToken();

      while(skills.ttype != StreamTokenizer.TT_EOF){
        if(skills.ttype == StreamTokenizer.TT_NUMBER){
          numOfPersSkills = (int)skills.nval;
          skills.nextToken();
        }
        else{
          skillString = skills.sval;
          skills.nextToken();
          while(skills.ttype != StreamTokenizer.TT_EOL && skills.ttype != StreamTokenizer.TT_EOF){
            skillString = skillString + " " + skills.sval;
            skills.nextToken();
          }
          if(skillString != null){
            skillsVector.add(skillString);
          }
          skills.nextToken();
        }
      }
      for(int i = 0; i < numOfPersSkills; i++){
        skillsVector.add("Personal Skill");
      }
      character.setSkills(skillsVector);
      if(fr != null)
        fr.close();
    }catch(FileNotFoundException fnfex)
    {
      System.out.println("File not found!");
    }
    catch(IOException ioex)
    {
      System.out.println("I/O Failed!");
    }
  }

  /**
   * Event handlers
   * @param e - The event to be handled
   */
  void occTF1_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF1.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF1.setText("" + oldValue);
      return;
    }

    if(oldValue == tmp.intValue())
      return;
    if(oldValue < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue)) < 0){
        occTF1.setText("" + oldValue);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue);
    }
    else if(oldValue > tmp.intValue()){
      occPoints += oldValue - tmp.intValue();
    }
    oldValue = new Integer(occTF1.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[0] = tmp.intValue();
  }

  void occTF2_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF2.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF2.setText("" + oldValue);
      return;
    }

    if(oldValue2 == tmp.intValue())
      return;
    if(oldValue2 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue2)) < 0){
        occTF2.setText("" + oldValue2);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue2);
    }
    else if(oldValue2 > tmp.intValue()){
      occPoints += oldValue2 - tmp.intValue();
    }
    oldValue2 = new Integer(occTF2.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[1] = tmp.intValue();
  }

  void occTF3_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF3.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF3.setText("" + oldValue);
      return;
    }

    if(oldValue3 == tmp.intValue())
      return;
    if(oldValue3 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue3)) < 0){
        occTF1.setText("" + oldValue3);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue3);
    }
    else if(oldValue3 > tmp.intValue()){
      occPoints += oldValue3 - tmp.intValue();
    }
    oldValue3 = new Integer(occTF1.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[2] = tmp.intValue();
  }

  void occTF4_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF4.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF4.setText("" + oldValue);
      return;
    }

    if(oldValue4 == tmp.intValue())
      return;
    if(oldValue4 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue4)) < 0){
        occTF4.setText("" + oldValue4);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue4);
    }
    else if(oldValue4 > tmp.intValue()){
      occPoints += oldValue4 - tmp.intValue();
    }
    oldValue4 = new Integer(occTF4.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[3] = tmp.intValue();
  }

  void occTF5_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF5.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF5.setText("" + oldValue);
      return;
    }

    if(oldValue5 == tmp.intValue())
      return;
    if(oldValue5 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue5)) < 0){
        occTF5.setText("" + oldValue5);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue5);
    }
    else if(oldValue5 > tmp.intValue()){
      occPoints += oldValue5 - tmp.intValue();
    }
    oldValue5 = new Integer(occTF5.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[4] = tmp.intValue();
  }

  void occTF6_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF6.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF6.setText("" + oldValue);
      return;
    }

    if(oldValue6 == tmp.intValue())
      return;
    if(oldValue6 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue6)) < 0){
        occTF6.setText("" + oldValue6);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue6);
    }
    else if(oldValue6 > tmp.intValue()){
      occPoints += oldValue6 - tmp.intValue();
    }
    oldValue6 = new Integer(occTF6.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[5] = tmp.intValue();
  }

  void occTF7_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF7.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF7.setText("" + oldValue);
      return;
    }

    if(oldValue7 == tmp.intValue())
      return;
    if(oldValue7 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue7)) < 0){
        occTF7.setText("" + oldValue7);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue7);
    }
    else if(oldValue7 > tmp.intValue()){
      occPoints += oldValue7 - tmp.intValue();
    }
    oldValue7 = new Integer(occTF7.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[6] = tmp.intValue();
  }

  void occTF8_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF8.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF8.setText("" + oldValue);
      return;
    }

    if(oldValue8 == tmp.intValue())
      return;
    if(oldValue8 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue8)) < 0){
        occTF8.setText("" + oldValue8);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue8);
    }
    else if(oldValue8 > tmp.intValue()){
      occPoints += oldValue8 - tmp.intValue();
    }
    oldValue8 = new Integer(occTF8.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[7] = tmp.intValue();
  }

  void occTF9_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF9.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF9.setText("" + oldValue);
      return;
    }

    if(oldValue9 == tmp.intValue())
      return;
    if(oldValue9 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue9)) < 0){
        occTF9.setText("" + oldValue9);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue9);
    }
    else if(oldValue9 > tmp.intValue()){
      occPoints += oldValue9 - tmp.intValue();
    }
    oldValue9 = new Integer(occTF9.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[8] = tmp.intValue();
  }

  void occTF10_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF10.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF10.setText("" + oldValue);
      return;
    }

    if(oldValue10 == tmp.intValue())
      return;
    if(oldValue10 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue10)) < 0){
        occTF10.setText("" + oldValue10);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue10);
    }
    else if(oldValue10 > tmp.intValue()){
      occPoints += oldValue10 - tmp.intValue();
    }
    oldValue10 = new Integer(occTF10.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[9] = tmp.intValue();
  }

  void occTF11_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF11.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF11.setText("" + oldValue);
      return;
    }

    if(oldValue11 == tmp.intValue())
      return;
    if(oldValue11 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue11)) < 0){
        occTF11.setText("" + oldValue11);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue11);
    }
    else if(oldValue11 > tmp.intValue()){
      occPoints += oldValue11 - tmp.intValue();
    }
    oldValue11 = new Integer(occTF11.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[10] = tmp.intValue();
  }

  void occTF12_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF12.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF12.setText("" + oldValue);
      return;
    }

    if(oldValue12 == tmp.intValue())
      return;
    if(oldValue12 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue12)) < 0){
        occTF12.setText("" + oldValue12);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue12);
    }
    else if(oldValue12 > tmp.intValue()){
      occPoints += oldValue12 - tmp.intValue();
    }
    oldValue12 = new Integer(occTF12.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[11] = tmp.intValue();
  }

  void occTF13_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF13.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF13.setText("" + oldValue);
      return;
    }

    if(oldValue13 == tmp.intValue())
      return;
    if(oldValue13 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue13)) < 0){
        occTF13.setText("" + oldValue13);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue13);
    }
    else if(oldValue13 > tmp.intValue()){
      occPoints += oldValue13 - tmp.intValue();
    }
    oldValue13 = new Integer(occTF13.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[12] = tmp.intValue();
  }

  void occTF14_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF14.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF14.setText("" + oldValue);
      return;
    }

    if(oldValue14 == tmp.intValue())
      return;
    if(oldValue14 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue14)) < 0){
        occTF14.setText("" + oldValue14);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue14);
    }
    else if(oldValue14 > tmp.intValue()){
      occPoints += oldValue14 - tmp.intValue();
    }
    oldValue14 = new Integer(occTF14.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[13] = tmp.intValue();
  }

  void occTF15_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF15.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF15.setText("" + oldValue);
      return;
    }

    if(oldValue15 == tmp.intValue())
      return;
    if(oldValue15 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue15)) < 0){
        occTF15.setText("" + oldValue15);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue15);
    }
    else if(oldValue15 > tmp.intValue()){
      occPoints += oldValue15 - tmp.intValue();
    }
    oldValue15 = new Integer(occTF15.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[14] = tmp.intValue();
  }

  void occTF16_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF16.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF16.setText("" + oldValue);
      return;
    }

    if(oldValue16 == tmp.intValue())
      return;
    if(oldValue16 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue16)) < 0){
        occTF16.setText("" + oldValue16);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue16);
    }
    else if(oldValue16 > tmp.intValue()){
      occPoints += oldValue16 - tmp.intValue();
    }
    oldValue16 = new Integer(occTF16.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[15] = tmp.intValue();
  }

  void occTF17_actionPerformed(ActionEvent e) {
    Integer tmp = new Integer(occTF17.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF17.setText("" + oldValue);
      return;
    }

    if(oldValue17 == tmp.intValue())
      return;
    if(oldValue17 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue17)) < 0){
        occTF17.setText("" + oldValue17);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue17);
    }
    else if(oldValue17 > tmp.intValue()){
      occPoints += oldValue17 - tmp.intValue();
    }
    oldValue17 = new Integer(occTF17.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[16] = tmp.intValue();
  }

  void occTF1_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF1.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF1.setText("" + oldValue);
      return;
    }

    if(oldValue == tmp.intValue())
      return;
    if(oldValue < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue)) < 0){
        occTF1.setText("" + oldValue);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue);
    }
    else if(oldValue > tmp.intValue()){
      occPoints += oldValue - tmp.intValue();
    }
    oldValue = new Integer(occTF1.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[0] = tmp.intValue();
  }

  void occTF2_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF2.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF2.setText("" + oldValue);
      return;
    }

    if(oldValue2 == tmp.intValue())
      return;
    if(oldValue2 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue2)) < 0){
        occTF2.setText("" + oldValue2);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue2);
    }
    else if(oldValue2 > tmp.intValue()){
      occPoints += oldValue2 - tmp.intValue();
    }
    oldValue2 = new Integer(occTF2.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[1] = tmp.intValue();
  }

  void occTF3_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF3.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF3.setText("" + oldValue);
      return;
    }

    if(oldValue3 == tmp.intValue())
      return;
    if(oldValue3 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue3)) < 0){
        occTF1.setText("" + oldValue3);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue3);
    }
    else if(oldValue3 > tmp.intValue()){
      occPoints += oldValue3 - tmp.intValue();
    }
    oldValue3 = new Integer(occTF1.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[2] = tmp.intValue();
  }

  void occTF4_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF4.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF4.setText("" + oldValue);
      return;
    }

    if(oldValue4 == tmp.intValue())
      return;
    if(oldValue4 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue4)) < 0){
        occTF4.setText("" + oldValue4);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue4);
    }
    else if(oldValue4 > tmp.intValue()){
      occPoints += oldValue4 - tmp.intValue();
    }
    oldValue4 = new Integer(occTF4.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[3] = tmp.intValue();
  }

  void occTF5_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF5.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF5.setText("" + oldValue);
      return;
    }

    if(oldValue5 == tmp.intValue())
      return;
    if(oldValue5 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue5)) < 0){
        occTF5.setText("" + oldValue5);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue5);
    }
    else if(oldValue5 > tmp.intValue()){
      occPoints += oldValue5 - tmp.intValue();
    }
    oldValue5 = new Integer(occTF5.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[4] = tmp.intValue();
  }

  void occTF6_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF6.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF6.setText("" + oldValue);
      return;
    }

    if(oldValue6 == tmp.intValue())
      return;
    if(oldValue6 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue6)) < 0){
        occTF6.setText("" + oldValue6);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue6);
    }
    else if(oldValue6 > tmp.intValue()){
      occPoints += oldValue6 - tmp.intValue();
    }
    oldValue6 = new Integer(occTF6.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[5] = tmp.intValue();
  }

  void occTF7_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF7.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF7.setText("" + oldValue);
      return;
    }

    if(oldValue7 == tmp.intValue())
      return;
    if(oldValue7 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue7)) < 0){
        occTF7.setText("" + oldValue7);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue7);
    }
    else if(oldValue7 > tmp.intValue()){
      occPoints += oldValue7 - tmp.intValue();
    }
    oldValue7 = new Integer(occTF7.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[6] = tmp.intValue();
  }

  void occTF8_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF8.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF8.setText("" + oldValue);
      return;
    }

    if(oldValue8 == tmp.intValue())
      return;
    if(oldValue8 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue8)) < 0){
        occTF8.setText("" + oldValue8);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue8);
    }
    else if(oldValue8 > tmp.intValue()){
      occPoints += oldValue8 - tmp.intValue();
    }
    oldValue8 = new Integer(occTF8.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[7] = tmp.intValue();
  }

  void occTF9_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF9.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF9.setText("" + oldValue);
      return;
    }

    if(oldValue9 == tmp.intValue())
      return;
    if(oldValue9 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue9)) < 0){
        occTF9.setText("" + oldValue9);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue9);
    }
    else if(oldValue9 > tmp.intValue()){
      occPoints += oldValue9 - tmp.intValue();
    }
    oldValue9 = new Integer(occTF9.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[8] = tmp.intValue();
  }

  void occTF10_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF10.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF10.setText("" + oldValue);
      return;
    }

    if(oldValue10 == tmp.intValue())
      return;
    if(oldValue10 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue10)) < 0){
        occTF10.setText("" + oldValue10);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue10);
    }
    else if(oldValue10 > tmp.intValue()){
      occPoints += oldValue10 - tmp.intValue();
    }
    oldValue10 = new Integer(occTF10.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[9] = tmp.intValue();
  }

  void occTF11_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF11.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF11.setText("" + oldValue);
      return;
    }

    if(oldValue11 == tmp.intValue())
      return;
    if(oldValue11 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue11)) < 0){
        occTF11.setText("" + oldValue11);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue11);
    }
    else if(oldValue11 > tmp.intValue()){
      occPoints += oldValue11 - tmp.intValue();
    }
    oldValue11 = new Integer(occTF11.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[10] = tmp.intValue();
  }

  void occTF12_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF12.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF12.setText("" + oldValue);
      return;
    }

    if(oldValue12 == tmp.intValue())
      return;
    if(oldValue12 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue12)) < 0){
        occTF12.setText("" + oldValue12);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue12);
    }
    else if(oldValue12 > tmp.intValue()){
      occPoints += oldValue12 - tmp.intValue();
    }
    oldValue12 = new Integer(occTF12.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[11] = tmp.intValue();
  }

  void occTF13_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF13.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF13.setText("" + oldValue);
      return;
    }

    if(oldValue13 == tmp.intValue())
      return;
    if(oldValue13 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue13)) < 0){
        occTF13.setText("" + oldValue13);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue13);
    }
    else if(oldValue13 > tmp.intValue()){
      occPoints += oldValue13 - tmp.intValue();
    }
    oldValue13 = new Integer(occTF13.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[12] = tmp.intValue();
  }

  void occTF14_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF14.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF14.setText("" + oldValue);
      return;
    }

    if(oldValue14 == tmp.intValue())
      return;
    if(oldValue14 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue14)) < 0){
        occTF14.setText("" + oldValue14);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue14);
    }
    else if(oldValue14 > tmp.intValue()){
      occPoints += oldValue14 - tmp.intValue();
    }
    oldValue14 = new Integer(occTF14.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[13] = tmp.intValue();
  }

  void occTF15_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF15.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF15.setText("" + oldValue);
      return;
    }

    if(oldValue15 == tmp.intValue())
      return;
    if(oldValue15 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue15)) < 0){
        occTF15.setText("" + oldValue15);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue15);
    }
    else if(oldValue15 > tmp.intValue()){
      occPoints += oldValue15 - tmp.intValue();
    }
    oldValue15 = new Integer(occTF15.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[14] = tmp.intValue();
  }

  void occTF16_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF16.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF16.setText("" + oldValue);
      return;
    }

    if(oldValue16 == tmp.intValue())
      return;
    if(oldValue16 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue16)) < 0){
        occTF16.setText("" + oldValue16);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue16);
    }
    else if(oldValue16 > tmp.intValue()){
      occPoints += oldValue16 - tmp.intValue();
    }
    oldValue16 = new Integer(occTF16.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[15] = tmp.intValue();
  }

  void occTF17_focusLost(FocusEvent e) {
    Integer tmp = new Integer(occTF17.getText());

    if(tmp.intValue() > 100){
      errorDlg.setLocation(this.getX()+(this.getWidth()/2)-(errorDlg.getWidth()/2), this.getY()+(this.getHeight()/2)-(errorDlg.getHeight()/2));
      errorDlg.show();
      occTF17.setText("" + oldValue);
      return;
    }

    if(oldValue17 == tmp.intValue())
      return;
    if(oldValue17 < tmp.intValue()){
      if((occPoints - (tmp.intValue() - oldValue17)) < 0){
        occTF17.setText("" + oldValue17);
        return;
      }
      occPoints -= (tmp.intValue() - oldValue17);
    }
    else if(oldValue17 > tmp.intValue()){
      occPoints += oldValue17 - tmp.intValue();
    }
    oldValue17 = new Integer(occTF17.getText()).intValue();
    spTextField.setText("" + occPoints);
    character.skillValues[16] = tmp.intValue();
  }
}