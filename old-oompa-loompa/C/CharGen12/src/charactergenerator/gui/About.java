package charactergenerator.gui;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

/**
 * Title:        About
 * Description:  About dialog box.
 * Copyright:    Copyright (c) 2002
 * Company:
 * @author Markus Svensson
 * @version 1.2
 */

public class About extends JDialog {
  private JPanel aboutPanel = new JPanel();
  private BorderLayout borderLayout1 = new BorderLayout();
  private JPanel southPanel = new JPanel();
  private JButton okButton = new JButton();
  private JPanel eastPanel = new JPanel();
  private JPanel northPanel = new JPanel();
  private JPanel westPanel = new JPanel();
  private JTextArea textArea = new JTextArea();

  /**
   * Constructor
   * @param The parent frame
   * @param The title
   * @param Modal?
   */
  public About(Frame frame, String title, boolean modal){
    super(frame, title, modal);
    jbInit();
    pack();
    this.setLocation(frame.getX()+(frame.getWidth()/2)-(this.getWidth()/2), frame.getY()+(frame.getHeight()/2)-(this.getHeight()/2));
    this.setVisible(true);
  }

  /**
   * Init the dialog
   */
  void jbInit(){
    aboutPanel.setLayout(borderLayout1);
    aboutPanel.setBackground(Color.lightGray);
    southPanel.setBackground(Color.lightGray);
    southPanel.setBorder(BorderFactory.createEtchedBorder());
    okButton.setBackground(Color.lightGray);
    okButton.setText("Dismiss");
    okButton.addMouseListener(new java.awt.event.MouseAdapter() {
      public void mouseReleased(MouseEvent e) {
        okButton_mouseReleased(e);
      }
    });
    westPanel.setBackground(Color.lightGray);
    northPanel.setBackground(Color.lightGray);
    eastPanel.setBackground(Color.lightGray);
    textArea.setText("\n\tCoC Character Generator V1.2\n\n");
    textArea.append("Call of Cthulhu is a Registred Trademark of Chaosium Inc.\n\n");
    textArea.append("This program was written by Markus Svensson, 2003\n");
    textArea.append("Send comments to: markus.svensson@linux.nu\n");
    getContentPane().add(aboutPanel);
    aboutPanel.add(southPanel, BorderLayout.SOUTH);
    southPanel.add(okButton, null);
    aboutPanel.add(eastPanel,  BorderLayout.EAST);
    aboutPanel.add(northPanel, BorderLayout.NORTH);
    aboutPanel.add(westPanel, BorderLayout.WEST);
    aboutPanel.add(textArea, BorderLayout.CENTER);
  }

  /**
   * Event handler for the dismiss button
   * @param The event
   */
  void okButton_mouseReleased(MouseEvent e) {
    this.setVisible(false);
  }
}
