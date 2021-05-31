package charactergenerator.gui;

import javax.swing.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.io.*;
import java.net.*;

/**
 * Title:        Browser
 * Description:  The Broweser class is used for viewing HTML files.
 *               It can connect to webbservers
 *               and to local files. To connect to webbsevers use
 *               http:// and for local files use
 *               file:/ To set a page without the absolute path place
 *               the file in the same directory
 *               as the Browser class file and set the filename as url.
 *               This version also supports hyperlinks in the HTML files.
 * @author Markus Svensson
 * @version 1.3
 */

public class Browser extends JFrame implements HyperlinkListener{

    private JPanel jPanelTop = new JPanel();
    private JPanel jPanelBottom = new JPanel();
    private JTextPane jTextPaneViewer = new JTextPane();
    private JTextField jTextFieldUrl = new JTextField();
    private JTextField jTextFieldStatus = new JTextField();
    private JScrollPane jScrollPaneBrowser = new JScrollPane(jTextPaneViewer);

    /** Creates a new Browser */
    public Browser() {
        initGUI();
        this.setVisible(true);
    }

    /**
     * Creates a new Browser and sets an initial page to be displayed
     * param String initPage The initial page to display
     */
    public Browser(String initPage) {
        initGUI();
        this.setVisible(true);
       	this.jTextFieldUrl.setText(initPage);
        this.jTextFieldUrlActionPerformed(null);
    }

    /** This method is called from within the constructor to initialize the Browser. */
    private void initGUI() {
        this.setTitle("CoC Character Generator Help");
        addWindowListener(
            new java.awt.event.WindowAdapter() {
                public void windowClosing(java.awt.event.WindowEvent evt) {
                    exitForm(evt);
                }
            });
        jScrollPaneBrowser.setAutoscrolls(true);
        setBounds(new java.awt.Rectangle(0, 0, 600, 520));
        getContentPane().add(jScrollPaneBrowser, java.awt.BorderLayout.CENTER);
        getContentPane().add(jPanelBottom, java.awt.BorderLayout.SOUTH);
        getContentPane().add(jPanelTop, java.awt.BorderLayout.NORTH);
        jTextPaneViewer.setText("");
        jTextPaneViewer.setContentType("text/html");
        jTextPaneViewer.setEditable(false);
        jTextFieldUrl.setText("");
        jPanelTop.setLayout(new java.awt.BorderLayout());
        jPanelTop.add(jTextFieldUrl, java.awt.BorderLayout.CENTER);
        jTextFieldStatus.setText("");
        jPanelBottom.setLayout(new java.awt.BorderLayout());
        jPanelBottom.setBorder(javax.swing.BorderFactory.createBevelBorder(javax.swing.border.BevelBorder.LOWERED));
        jPanelBottom.add(jTextFieldStatus, java.awt.BorderLayout.CENTER);
        jTextFieldUrl.setEditable(false);
        jTextFieldStatus.setEditable(false);
        this.jTextPaneViewer.addHyperlinkListener(this);
    }

    /** Exits the Application */
    private void exitForm(WindowEvent evt) {
        this.setVisible(false);
    }

     /**
     * Called when an HyperLink is clicked.
     */
    public void hyperlinkUpdate(HyperlinkEvent e) {
        if (e.getEventType().equals(HyperlinkEvent.EventType.ACTIVATED)) {
            this.setUrl(e.getURL());
        }
    }

    /**
     * Called when the adressbar is clicked.
     */
    private void jTextFieldUrlActionPerformed(ActionEvent e) {
        try {
            this.setUrl(new URL(this.jTextFieldUrl.getText()));
        } catch (MalformedURLException ex) {
            Class browserClass = this.getClass();
			this.setUrl(browserClass.getResource(this.jTextFieldUrl.getText()));
        }
        this.jTextFieldUrl.setText("");
    }

    /**
     * Loads the page to be viewed and displays it.
     * @param URL url The URL to the page to display
     */
    public void setUrl(URL url) {
		if(url == null){
			this.jTextFieldStatus.setText("Could not find page");
            return;
        }
        try {
            this.jTextFieldStatus.setText("Loading page");
            this.jTextPaneViewer.setPage(url);
            this.jTextFieldStatus.setText("Loaded page");
        } catch (IOException ex) {
            this.jTextFieldStatus.setText("Failed to load file: " + ex.getMessage());
        }
    }
}
