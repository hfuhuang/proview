/* 
 * Proview   $Id: MhClient.java,v 1.2 2005-09-01 14:57:51 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

package jpwr.jop;
import jpwr.rt.*;
//import Logg;
import java.awt.*;
import java.awt.event.*;
import java.awt.font.*;
import java.awt.geom.*;
import java.awt.image.*;
import javax.swing.event.*;
import java.io.*;
import java.net.*;
import java.util.*;
import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import java.awt.event.*;


/**
 *  Description of the Class
 *
 *@author     JN3920
 *@created    November 12, 2002
 */
public class MhClient extends JApplet
{
  /**  Description of the Field */
  BorderLayout borderLayout1 = new BorderLayout();

  /**  Description of the Field */
  BorderLayout borderLayout5 = new BorderLayout();

  /**  Description of the Field */
  JPanel contentPane;
  /**  Description of the Field */
  JLabel labelMessage = new JLabel("MhClient version 1.0");

  /**  Description of the Field */
  JPanel messagePanel = new JPanel();

  /**  Description of the Field */
  boolean sim = false;

  /**  Description of the Field */
  Dimension size;

  /**  Description of the Field */
  JPanel userPanel = new JPanel();


  /**  Description of the Field */
  MhTable mhTable;


  /**  Constructor for the MhClient object */
  public MhClient() { }


  /**
   *  Sets the size attribute of the MhClient object
   *
   *@param  width   The new size value
   *@param  height  The new size value
   */
  public void setSize(int width, int height)
  {
    //Logg.logg("MhClient: setSize()", 1);

    super.setSize(width, height);

    validate();
  }



  /**
   *  Description of the Method
   *
   *@return    Description of the Return Value
   */
/*
  public String GetAppletInfo()
  {
    return "Applet information";
  }
*/

  /**
   *  Gets the parameterInfo attribute of the MhClient object
   *
   *@return    The parameterInfo value
   */
  public String[][] getParameterInfo()
  {
    return null;
  }




  /**  Description of the Method */
  public void destroy()
  {
    if(mhTable != null)
      mhTable.close();
    if(!sim)
    {
      super.destroy();
    }
  }


  /**  Description of the Method */
  public void init()
  {
    super.init();
    size = new Dimension(570, 570);
    contentPane = (JPanel)this.getContentPane();
    contentPane.setLayout(borderLayout1);


    Dimension d = messagePanel.getSize();
    d.height += 20;
    messagePanel.setPreferredSize(d);
    contentPane.add(messagePanel, BorderLayout.SOUTH);

    contentPane.setOpaque(true);
    userPanel.setLayout(new GridBagLayout());
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.anchor = GridBagConstraints.WEST;
    gbc.insets = new Insets(2, 2, 2, 2);

    messagePanel.setLayout(borderLayout5);
    contentPane.setOpaque(true);
    contentPane.setBackground(Color.white);
    this.setSize(size);

    messagePanel.add(labelMessage, BorderLayout.NORTH);
    //Logg.logg("MhClient: F�re XttTree-skapande", 6);
    this.mhTable = new MhTable(this, false, this.labelMessage);
    //Logg.logg("MhClient: mhTable-skapande klart", 6);
    this.contentPane.add(this.mhTable.splitPane, BorderLayout.CENTER);

  }
  
  public void setMess(String mess, boolean error)
  {
    /*
    if(error)
    {
      this.labelMessage.setColor(Color.red);
    }
    else
    {
      this.labelMessage.setColor(Color.blue);
    }
    */
    this.labelMessage.setText(mess);
  }


  /**  Description of the Method */
  public void start()
  {
  }


  /**  Description of the Method */
  public void stop()
  {
  }
}

