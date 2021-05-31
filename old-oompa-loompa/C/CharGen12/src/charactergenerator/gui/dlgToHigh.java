package charactergenerator.gui;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

/**
 * <p>Title: Dialog To High</p>
 * <p>Description: Error dialog, showed if user inputs value larger than 100 in a skill</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * @author Markus Svensson
 * @since 1.2
 * @version 1.2
 */

public class dlgToHigh extends JDialog {
  private JPanel panel1 = new JPanel();
  private BorderLayout borderLayout1 = new BorderLayout();
  private JEditorPane jepText = new JEditorPane();
  private JPanel pnlBottom = new JPanel();
  private JButton jbtOk = new JButton();

  public dlgToHigh(Frame frame, String title, boolean modal) {
    super(frame, title, modal);
    this.jbInit();
    this.setSize(300, 200);
    this.setLocation(frame.getX()+(frame.getWidth()/2)-(this.getWidth()/2), frame.getY()+(frame.getHeight()/2)-(this.getHeight()/2));
  }

  public dlgToHigh() {
    this(null, "", false);
  }
  private void jbInit() {
    panel1.setLayout(borderLayout1);
    this.setModal(true);
    this.setResizable(false);
    this.setTitle("Value to large!");
    jepText.setFont(new java.awt.Font("Serif", 0, 16));
    jepText.setOpaque(false);
    jepText.setEditable(false);
    jepText.setText("\n\n       The max value for skills are 100%.\n           Please choose a lower value.");
    jbtOk.setText("OK");
    jbtOk.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        jbtOk_actionPerformed(e);
      }
    });
    pnlBottom.setBorder(BorderFactory.createEtchedBorder());
    getContentPane().add(panel1);
    panel1.add(jepText, BorderLayout.CENTER);
    panel1.add(pnlBottom, BorderLayout.SOUTH);
    pnlBottom.add(jbtOk, null);
  }

  void jbtOk_actionPerformed(ActionEvent e) {
    this.hide();
  }
}