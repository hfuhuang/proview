package jpwr.jop;
import java.awt.*;
import javax.swing.*;
import javax.swing.table.DefaultTableCellRenderer;
import jpwr.rt.Mh;
import jpwr.rt.MhrEvent;
/* This class is used to make sure an EventTableModel in a JaTable is presented 
 * in a graphically appealing manner. */

public class EventTableCellRender extends DefaultTableCellRenderer
  {
        
  Color ALarmColor = Color.red;
  Color BLarmColor = Color.yellow;
  Color CLarmColor = Color.blue;
  Color DLarmColor = Color.cyan;
  Color InfoColor = Color.green;

    /**
     *  Gets the tableCellRendererComponent attribute of the
     *  EventTableCellRender object
     *
     *@param  table       Description of the Parameter
     *@param  value       Description of the Parameter
     *@param  isSelected  Description of the Parameter
     *@param  hasFocus    Description of the Parameter
     *@param  row         Description of the Parameter
     *@param  column      Description of the Parameter
     *@return             The tableCellRendererComponent value
     */
    public Component getTableCellRendererComponent(JTable table, Object value,
                                                   boolean isSelected, boolean hasFocus, int row, int column)
    {
      //I will assume that the number is stored as a string. if it is stored as an Integer, you can change this easily.
      //if(column == 0)
      //{
      String number = (String)value;
      //cast to a string
      //int val = Integer.parseInt(number);//convert to an integer
      MhrEvent ev = ((EventTableModel)table.getModel()).getRowObject(row);

      this.setBackground(Color.white);

      if(ev == null)
      {
        this.setText(" ");
        return this;
      }

      boolean setColor = false;
      if( ev.eventType == Mh.mh_eEvent_Alarm||ev.eventType == Mh.mh_eEvent_Info )
        setColor = true;

      //System.out.println("i eventTable.getTableCellRendererComponent(row " + row + "value" + number + ")");
      if(number.compareTo("A") == 0)
      {
        if(setColor)
        {
          this.setBackground(ALarmColor);
        }
        this.setText("A");
      }
      else if(number.compareTo("B") == 0)
      {
        if(setColor)
        {
          this.setBackground(BLarmColor);
        }
        this.setText("B");
      }
      else if(number.compareTo("C") == 0)
      {
        if(setColor)
        {
          this.setBackground(CLarmColor);
        }
        this.setText("C");
      }
      else if(number.compareTo("D") == 0)
      {
        if(setColor)
        {
          this.setBackground(DLarmColor);
        }
        this.setText("D");
      }
      //annars m�ste det vara ett meddelande qqq??
      else
      {
        if(setColor)
        {
          this.setBackground(InfoColor);
        }
        this.setText(" ");
      }

      //}
      //else
      //  this.setBackground(Color.white);

      return this;
    }
  }
