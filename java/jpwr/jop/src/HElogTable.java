package jpwr.jop;

import com.sleepycat.db.*;

import jpwr.rt.MhrEvent;
import jpwr.rt.MhrsEventId;
import jpwr.rt.Mh;
import jpwr.rt.MhData;
import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import java.awt.event.*;
import javax.swing.Timer;
import javax.swing.table.TableColumn;
/**
 *  Description of the Class
 *
 *@author     claes
 *@created    December 10, 2002
 */
public class HElogTable extends JPanel
{
  //public final static int GET_SUBSCRIPTIONS = 41;
  final static int PORT = 4446;
  HElogClientReceiveThread recThread;
  boolean trace = false;
  boolean DEBUG = false;
  Socket mhSocket;
  ObjectOutputStream out;
  ObjectInputStream in;
  JLabel mess;
  MhData mhData = new MhData(100, 100);

  Timer timer;
  int scantime = 5000;
  boolean closingDown = false;
  JSplitPane splitPane;
  
  int maxNrOfAlarms;
  JTable alarmTable;
  //Vector alarmData = new Vector();
  JScrollPane scrollPaneAl;
  String[] columnNamesAlarmTable = {"P",
    "Kv",
    "Tid",
    "Larmtext",
    "Objekt"};
  //JLabel alarmTableLbl = new JLabel("Larmlista");    
  
  int maxNrOfEvents;
  JTable eventTable;
  //Vector eventData = new Vector();
  JScrollPane scrollPaneEv;
  String[] columnNamesEventTable = {"P",
    "Typ",
    "Tid",
    "H�ndelsetext",
    "Objekt"};
  //JLabel eventTableLbl = new JLabel("H�ndelselista");
  Color ALarmColor = Color.red;
  Color BLarmColor = Color.yellow;
  Color CLarmColor = Color.blue;
  Color DLarmColor = Color.cyan;
  Color InfoColor = Color.green;
  /**
   *  Constructor for the HElogTable object
   *
   *@param  root   Description of the Parameter
   *@param  DEBUG  Description of the Parameter
   */
  public HElogTable(Object root, boolean DEBUG, JLabel mess)
  {

    //super("HElogTable");
    this.mess = mess;
    this.DEBUG = DEBUG;
    System.out.println("Ny Version");
    this.setBackground(Color.white);

    AlarmTableModel alModel = new AlarmTableModel();
    alarmTable = new JTable(alModel);
    //alarmTable.setCellEditor(null);
    alarmTable.setShowGrid(false);
    alarmTable.setCellSelectionEnabled(false);
    alarmTable.setColumnSelectionAllowed(false);
    alarmTable.setRowSelectionAllowed(true);
    alarmTable.getTableHeader().setReorderingAllowed(false);
    //Create the scroll pane and add the table to it.
    alarmTable.setBackground(Color.white);
    scrollPaneAl = new JScrollPane(alarmTable);

    scrollPaneAl.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);

    this.initColumnSizes(alarmTable, alModel, true);
    alarmTable.getColumn((Object)columnNamesAlarmTable[0]).setCellRenderer(new AlarmTableCellRender());
    scrollPaneAl.getViewport().setBackground(Color.white);

    EventTableModel evModel = new EventTableModel();
    eventTable = new JTable(evModel);
    eventTable.setCellEditor(null);
    eventTable.getTableHeader().setReorderingAllowed(false);
    //Create the scroll pane and add the table to it.
    scrollPaneEv = new JScrollPane(eventTable);
    scrollPaneEv.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
    //scrollPaneEv.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
    //Add the scroll pane to this window.
    this.initColumnSizes(eventTable, evModel, false);
    scrollPaneEv.getViewport().setBackground(Color.white);
    eventTable.getColumn((Object)columnNamesEventTable[0]).setCellRenderer(new EventTableCellRender());

    //Create a split pane with the two scroll panes in it.
    splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT,
      scrollPaneAl, scrollPaneEv);


    splitPane.setOneTouchExpandable(false);
    splitPane.setDividerLocation(270);
    splitPane.setBackground(Color.white);

    this.init(root);

    this.newKeyboardBindings();
    //se till att allt syns vid start
    ((AlarmTableModel)alarmTable.getModel()).updateTable();
    ((EventTableModel)eventTable.getModel()).updateTable();

    
  }


  /*
     * This method picks good column sizes.
     * If all column heads are wider than the column's cells'
     * contents, then you can just use column.sizeWidthToFit().
     */
  /**
   *  Description of the Method
   *
   *@param  table              Description of the Parameter
   *@param  model              Description of the Parameter
   *@param  isAlarmTableModel  Description of the Parameter
   */
  private void initColumnSizes(JTable table, AbstractTableModel model, boolean isAlarmTableModel)
  {
    TableColumn column = null;
    Component comp = null;
    int headerWidth = 0;
    int cellWidth = 0;
    Object[] longValues;

    if(isAlarmTableModel)
    {
      longValues = ((AlarmTableModel)(model)).longValues;
    }
    else
    {
      longValues = ((EventTableModel)(model)).longValues;
    }

    for(int i = 0; i < longValues.length; i++)
    {
      column = table.getColumnModel().getColumn(i);

      comp = table.getDefaultRenderer(model.getColumnClass(i)).
        getTableCellRendererComponent(
        table, longValues[i],
        false, false, 0, i);
      cellWidth = comp.getPreferredSize().width;

      if(DEBUG)
      {
        System.out.println("Initializing width of column "
           + i + ". "
           + "headerWidth = " + headerWidth
           + "; cellWidth = " + cellWidth);
      }

      //XXX: Before Swing 1.1 Beta 2, use setMinWidth instead.
      column.setPreferredWidth(Math.max(headerWidth, cellWidth));
      if(i == 0)
      {
        column.setMaxWidth(Math.max(headerWidth, cellWidth));
        column.setResizable(false);
      }
      else if(i == 1 && isAlarmTableModel)
      {
        column.setMaxWidth(Math.max(headerWidth, cellWidth));
        column.setResizable(false);
      }
    }
  }

  /**
   *  Description of the Method
   *
   *@param  root  Description of the Parameter
   */
  private void init(Object root)
  {
    recThread = new HElogClientReceiveThread(this);
    /*
    try
    {
      URL url;
      String urlString = "127.0.0.1";
      if(root instanceof JApplet)
      {
        url = ((JApplet)root).getCodeBase();
        urlString = url.getHost();
      }
      if(trace)
      {
        System.out.println("Opening socket to " + urlString);
      }

      mhSocket = new Socket(urlString, PORT);
      mhSocket.setKeepAlive(true);
      recThread = new HElogClientReceiveThread(mhSocket, this);
    }
    catch(UnknownHostException e)
    {
      System.err.println("Don't know about host: taranis.");
      mess.setText("FEL: Kan ej identifiera servern");
      //System.exit(1);
    }
    catch(IOException e)
    {
      System.err.println("Couldn't get I/O for the connection");
      mess.setText("FEL: Kan ej skapa I/O");
      //System.exit(1);
    }
    */
  }


  /**
   *  Description of the Method
   */
  public void close()
  {
    closingDown = true;
    try
    {
      System.out.println("Closing socket");

      recThread.keepRunning = false;
      recThread.out.close();
      recThread.in.close();
      mhSocket.close();
    }
    catch(IOException e)
    {
      System.err.println("Couldn't close I/O connection");
    }
    catch(java.lang.NullPointerException e)
    {
    }
  }



  /**
   *  Description of the Method
   */
  public void newKeyboardBindings()
  {
    InputMap inputMap = new InputMap();
    ActionMap actionMap = new ActionMap();

    // Create some keystrokes and bind them to an action
    inputMap.put(KeyStroke.getKeyStroke("ctrl K"), "ACK");
    inputMap.setParent(this.alarmTable.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW));
    this.alarmTable.setInputMap(JComponent.WHEN_FOCUSED, inputMap);

    // Add the actions to the actionMap
    actionMap.put("ACK",
      new AbstractAction("ACK")
      {
        public void actionPerformed(ActionEvent evt)
        {
          //System.out.println("HElogTable: innan ACK");
          //int row = alarmTable.getSelectedRow();
	  
	  AlarmTableModel al = (AlarmTableModel)alarmTable.getModel();
	  for(int row = 0; row < al.getRowCount();row++)
	  {
	    MhrEvent ev = al.getRowObject(row);
	    if(ev == null)
	    {
	      return;
	    }
	    if((ev.eventStatus & Mh.mh_mEventStatus_NotAck) > 0)
	    {
	      al.ackAlarm(row);
	      break;
	    }
	  }
	  
        }
      }
      );
    actionMap.setParent(this.alarmTable.getActionMap());
    this.alarmTable.setActionMap(actionMap);
  }





  /**
   *  Description of the Method
   *
   *@param  ev  Description of the Parameter
   */
  public void newMess(MhrEvent ev)
  {
  
    mhData.insertNewMess(ev);
    ((AlarmTableModel)alarmTable.getModel()).updateTable();
    ((EventTableModel)eventTable.getModel()).updateTable();
    
    //System.out.println("eventtyp " + ev.eventType + " evestst "+ ev.eventStatus + " NotAck " + Mh.mh_mEventStatus_NotAck + " uttryck " + (ev.eventStatus &
    //Mh.mh_mEventStatus_NotAck));

  }


  /**
   *  Description of the Class
   *
   *@author     claes
   *@created    December 10, 2002
   */
  class AlarmTableModel extends AbstractTableModel
  {

    public final Object[] longValues = {"A", Boolean.TRUE,
      "10-12-31 12:12:12.98",
      "QWERTYUIOP���LK_JHGFDSAZXCVBNM__P",
      "QWERTYUIOP���LK"};


    /**
     *  Constructor for the AlarmTableModel object
     */
    public AlarmTableModel() { }


    /**
     *  Gets the columnCount attribute of the AlarmTableModel object
     *
     *@return    The columnCount value
     */
    public int getColumnCount()
    {
      return columnNamesAlarmTable.length;
    }


    /**
     *  Gets the rowCount attribute of the AlarmTableModel object
     *
     *@return    The rowCount value
     */
    public int getRowCount()
    {
      return mhData.getNrOfAlarms();//alarmData.size();
    }


    /**
     *  Gets the columnName attribute of the AlarmTableModel object
     *
     *@param  col  Description of the Parameter
     *@return      The columnName value
     */
    public String getColumnName(int col)
    {
      return (String)columnNamesAlarmTable[col];
    }


    /**
     *  Gets the valueAt attribute of the AlarmTableModel object
     *
     *@param  row  Description of the Parameter
     *@param  col  Description of the Parameter
     *@return      The valueAt value
     */
    public Object getValueAt(int row, int col)
    {
      try
      {
        MhrEvent ev = mhData.getAlarm(row);//(MhrEvent)alarmData.get(row);
        if(col == 0)
        {
          if(ev.eventPrio == Mh.mh_eEventPrio_A)
          {
            return "A";
          }
          if(ev.eventPrio == Mh.mh_eEventPrio_B)
          {
            return "B";
          }
          if(ev.eventPrio == Mh.mh_eEventPrio_C)
          {
            return "C";
          }
          if(ev.eventPrio == Mh.mh_eEventPrio_D)
          {
            return "D";
          }
          else
          {
            return "U";
          }
        }
        if(col == 1)
        {
          return new Boolean(((ev.eventStatus & Mh.mh_mEventStatus_NotAck) == 0));
        }
        if(col == 2)
        {
          return ev.eventTime;
        }
        if(col == 3)
        {
          return ev.eventText;
        }
        if(col == 4)
        {
          return ev.eventName;
        }

      }
      catch(ArrayIndexOutOfBoundsException e)
      {
        System.out.println(e.toString());
        if(col == 1)
        {
          return new Boolean(false);
        }
      }
      return "FEEELLLL";
    }


    /**
     *  Gets the columnClass attribute of the AlarmTableModel object
     *
     *@param  c  Description of the Parameter
     *@return    The columnClass value
     */
    public Class getColumnClass(int c)
    {
      return longValues[c].getClass();
      //return getValueAt(0, c).getClass();
    }


    /**
     *  Gets the cellEditable attribute of the AlarmTableModel object
     *
     *@param  row  Description of the Parameter
     *@param  col  Description of the Parameter
     *@return      The cellEditable value
     */
    public boolean isCellEditable(int row, int col)
    {
      if(col == 1)
      {
        return true;
      }
      return false;
    }


    /**
     *  Sets the valueAt attribute of the AlarmTableModel object
     *
     *@param  value  The new valueAt value
     *@param  row    The new valueAt value
     *@param  col    The new valueAt value
     */
    public void setValueAt(Object value, int row, int col)
    {
      //System.out.println("i setValueAt");
      try
      {
        MhrEvent ev = mhData.getAlarm(row);//(MhrEvent)alarmData.get(row);
        boolean checked = ((Boolean)value).booleanValue();
        if(col == 1 && checked)
        {
          //ev.eventStatus &= ~ Mh.mh_mEventStatus_NotAck;
          if(recThread != null)
          {
            //recThread.sendMess(ev.eventId);
          }
        }
      }
      catch(Exception e)
      {
        System.out.println(e.toString());
      }
      //fireTableCellUpdated(row, col);

    }


    /**
     *  Gets the rowObject attribute of the AlarmTableModel object
     *
     *@param  row  Description of the Parameter
     *@return      The rowObject value
     */
    public MhrEvent getRowObject(int row)
    {
      try
      {
        return mhData.getAlarm(row);//(MhrEvent)alarmData.get(row);
      }
      catch(ArrayIndexOutOfBoundsException e)
      {
        System.out.println("alarmtable.getRowObject " + e.toString());
      }
      return null;
    }


    /**
     *  Description of the Method
     *
     *@param  row  Description of the Parameter
     *@param  col  Description of the Parameter
     */
    public void cellUpdated(int row, int col)
    {
      fireTableCellUpdated(row, col);
    }


    /**
     *  Description of the Method
     */
    public void rowInserted()
    {
      fireTableRowsInserted(0, 0);
    }


    /**
     *  Description of the Method
     *
     *@param  row  Description of the Parameter
     */
    public void rowRemoved(int row)
    {
      fireTableRowsDeleted(row, row);
      
    }


    /**
     *  Description of the Method
     */
    public void reloadTable()
    {
      fireTableStructureChanged();
    }


    /**
     *  Description of the Method
     */
    public void updateTable()
    {
      fireTableDataChanged();
    }


    /**
     *  Description of the Method
     *
     *@param  row  Description of the Parameter
     */
    public void ackAlarm(int row)
    {
      
      if(row != -1)
      {
        setValueAt(new Boolean(true), row, 1);
      }
    }

  }


  /**
   *  Description of the Class
   *
   *@author     claes
   *@created    December 10, 2002
   */
  class EventTableModel extends AbstractTableModel
  {

    public final Object[] longValues = {"A", "Kvittens",
      "10-12-31 12:12:12.98",
      "QWERTYUIOP���LK_JHGFDSAZXCVBNM__PO�IUYTRQWERTYUIOP���L",
      "QWERTYUIOP���LK"};


    /**
     *  Constructor for the EventTableModel object
     */
    public EventTableModel() { }


    /**
     *  Gets the columnCount attribute of the EventTableModel object
     *
     *@return    The columnCount value
     */
    public int getColumnCount()
    {
      return columnNamesEventTable.length;
    }


    /**
     *  Gets the rowCount attribute of the EventTableModel object
     *
     *@return    The rowCount value
     */
    public int getRowCount()
    {
      return mhData.getNrOfEvents();//eventData.size();
    }


    /**
     *  Gets the columnName attribute of the EventTableModel object
     *
     *@param  col  Description of the Parameter
     *@return      The columnName value
     */
    public String getColumnName(int col)
    {
      return (String)columnNamesEventTable[col];
    }


    /**
     *  Gets the valueAt attribute of the EventTableModel object
     *
     *@param  row  Description of the Parameter
     *@param  col  Description of the Parameter
     *@return      The valueAt value
     */
    public Object getValueAt(int row, int col)
    {
      try
      {
        MhrEvent ev = mhData.getEvent(row);//(MhrEvent)eventData.get(row);
        if(col == 0)
        {
          //System.out.println("col == 0 i eventTable.getValueAt()");
          if(ev.eventPrio == Mh.mh_eEventPrio_A)
          {
            return "A";
          }
          if(ev.eventPrio == Mh.mh_eEventPrio_B)
          {
            return "B";
          }
          if(ev.eventPrio == Mh.mh_eEventPrio_C)
          {
            return "C";
          }
          if(ev.eventPrio == Mh.mh_eEventPrio_D)
          {
            return "D";
          }
          else
          {
            return "U";
          }
        }
        if(col == 1)
        {
	  String returnString = " ";
	  switch (ev.eventType)
	  {
	    case Mh.mh_eEvent_Alarm:
	      returnString = "Larm";
	    break;
	    case Mh.mh_eEvent_Ack:
	      returnString = "Kvittens";
	    break;
	    case Mh.mh_eEvent_Block:
	      returnString = "Block";
	    break;
	    case Mh.mh_eEvent_Cancel:
	      returnString = "Cancel";
	    break;
	    case Mh.mh_eEvent_CancelBlock:
	      returnString = "CancelBlock";
	    break;
	    case Mh.mh_eEvent_Missing:
	      returnString = "Missing";
	    break;
	    case Mh.mh_eEvent_Reblock:
	      returnString = "Reblock";
	    break;
	    case Mh.mh_eEvent_Return:
	      returnString = "Retur";
	    break;
	    case Mh.mh_eEvent_Unblock:
	      returnString = "Unblock";
	    break;
	    case Mh.mh_eEvent_Info:
	      returnString = "Info";
	    break;
	    case Mh.mh_eEvent_:
	      returnString = "?";
	    break;
	    default:
	     returnString = " ";
	    break;
	  }
	  return returnString;
        }
        if(col == 2)
        {
          return ev.eventTime;
        }
        if(col == 3)
        {
          return ev.eventText;
        }
        if(col == 4)
        {
          return ev.eventName;
        }
      }

      catch(ArrayIndexOutOfBoundsException e)
      {
        System.out.println(e.toString());
        if(col == 1)
        {
          return new Boolean(false);
        }
      }
      return "FEEELLLL";
    }


    /**
     *  Gets the columnClass attribute of the EventTableModel object
     *
     *@param  c  Description of the Parameter
     *@return    The columnClass value
     */
    public Class getColumnClass(int c)
    {
      return longValues[c].getClass();
      //return getValueAt(0, c).getClass();
    }


    /**
     *  Sets the valueAt attribute of the EventTableModel object
     *
     *@param  value  The new valueAt value
     *@param  row    The new valueAt value
     *@param  col    The new valueAt value
     */
    public void setValueAt(Object value, int row, int col)
    {
      //alarmData[row][col] = value;
      // fireTableCellUpdated(row, col);

    }


    /**
     *  Gets the rowObject attribute of the EventTableModel object
     *
     *@param  row  Description of the Parameter
     *@return      The rowObject value
     */
    public MhrEvent getRowObject(int row)
    {
      try
      {
        return mhData.getEvent(row);//(MhrEvent)eventData.get(row);
      }
      catch(ArrayIndexOutOfBoundsException e)
      {
        System.out.println("eventable.getRowObject " + e.toString());
      }
      return null;
    }


    /**
     *  Description of the Method
     */
    public void rowInserted()
    {
      fireTableRowsInserted(0, 0);
    }


    /**
     *  Description of the Method
     *
     *@param  row  Description of the Parameter
     */
    public void rowRemoved(int row)
    {
      fireTableRowsDeleted(row, row);
      
    }


    /**
     *  Description of the Method
     */
    public void reloadTable()
    {
      fireTableStructureChanged();
    }


    /**
     *  Description of the Method
     */
    public void updateTable()
    {
      fireTableDataChanged();
    }

  }


  /**
   *  Description of the Class
   *
   *@author     claes
   *@created    December 10, 2002
   */
  private class HElogClientReceiveThread extends Thread
  {
    HElogTable hElogTable;
    Socket socket;
    ObjectInputStream in;
    ObjectOutputStream out;
    Db db;
    boolean keepRunning = true;


    /**
     *  Constructor for the HElogClientReceiveThread object
     *
     *@param  socket   Description of the Parameter
     *@param  hElogTable  Description of the Parameter
     */
    public HElogClientReceiveThread(HElogTable hElogTable)
    {
      this.hElogTable = hElogTable;
/*
      this.socket = socket;
      try
      {
        out = new ObjectOutputStream(socket.getOutputStream());
        out.flush();
        //varf�r???
        in = new ObjectInputStream(socket.getInputStream());
      }
      catch(IOException e)
      {
        System.out.println("IOException vid skapande av str�mmar mot server");
        mess.setText("FEL: Kan ej skapa kontakt med servern");
	//errh.error("DataStream failed");
        return;
      }
*/
      try{
        db = new Db(null, null);
	db.open(null, "/data1/pwrp/ssabt/common/db/rt_eventlog.db", Db.DB_RDONLY, 0);
        start();
      }
      catch(Exception e)
      {
        System.out.println(e.toString());
      }
      
    }


    /**
     *  Main processing method for the HElogClientReceiveThread object
     */
    public void run()
    {
      try{
        Dbc cursor = Db.cursor(null, 0);
	Dbt key = new Dbt();
	Dbt data = new Dbt();
	cursor.get(key, data, Db.DB_FIRST);
	system.out.println("Data: " + data.data);
      }
      catch(Exception e)
      {
        System.out.println(e.toString());
      }
      
    
/*
      try
      {
        mhData.maxNrOfAlarms = in.readInt();
	if(mhData.maxNrOfAlarms == -1)
	{
	  mess.setText("F�r m�nga anslutningar, AVSLUTA");
	  this.keepRunning = false;
	}
	else
	{
	  mhData.maxNrOfEvents = in.readInt();
	  int nrOfAlarms = in.readInt();
	  if(nrOfAlarms > 0)
	  {
	    mhData.alarmVec = (Vector)in.readObject();
            ((AlarmTableModel)alarmTable.getModel()).updateTable();
	  }
	  int nrOfEvents = in.readInt();
	  if(nrOfEvents > 0)
	  {
	    mhData.eventVec = (Vector)in.readObject();
	    ((EventTableModel)eventTable.getModel()).updateTable();
	  }
	}
      }
      catch(Exception e)
      {
        System.out.println(e.toString());
	this.keepRunning = false;
      }

      while(this.keepRunning)
      {
        try
        {
          //h�r skall vi ligga och v�nta p� meddelanden fr�n servern
          //typ alarm och h�ndelser
          //System.out.println("V�ntar p� mess");
          MhrEvent ev = (MhrEvent)in.readObject();
          //System.out.println("F�tt mess");
          hElogTable.newMess(ev);
          //Thread.sleep(1);
        }
        catch(ClassNotFoundException e)
        {
          System.out.println("Exception i mess v�ntan: " + e.toString());
	  mess.setText("FEL: Kan ej tolka meddelande fr�n servern");
        }
        catch(Exception e)
        {
          System.out.println("Exception i mess v�ntan: " + e.toString());
          System.out.println("Avslutar");
          mess.setText("FEL: Fel vid mottagande av meddelande fr�n server, AVSLUTA");
	  this.keepRunning = false;
        }
      }
      try
      {
        in.close();
        out.close();
      }
      catch(Exception e)
      {
      }
*/
    }


    /**
     *  Description of the Method
     *
     *@param  obj  Description of the Parameter
     */
/*
    public void sendMess(MhrsEventId obj)
    {
      try
      {
        out.writeObject(obj);
        out.flush();
      }
      catch(Exception e)
      {
        System.out.println("Exception vid s�ndning av meddelande");
	mess.setText("FEL: Kan ej s�nda meddelande, AVSLUTA");
      }
    }
*/
  }


  //MhClientReciveThread

  /**
   *  Description of the Class
   *
   *@author     claes
   *@created    December 10, 2002
   */
  private class AlarmTableCellRender extends DefaultTableCellRenderer
  {

    /**
     *  Gets the tableCellRendererComponent attribute of the
     *  AlarmTableCellRender object
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
      MhrEvent ev = ((AlarmTableModel)table.getModel()).getRowObject(row);

      this.setBackground(Color.white);

      if(ev == null)
      {
        this.setText(" ");
        return this;
      }

      boolean isNotReturned = ((ev.eventStatus & Mh.mh_mEventStatus_NotRet) != 0);

      if(number.compareTo("A") == 0)
      {
        //if(isNotReturned)
        //{
          this.setBackground(ALarmColor);
        //}
        this.setText("A");
      }
      else if(number.compareTo("B") == 0)
      {
        //if(isNotReturned)
        //{
          this.setBackground(BLarmColor);
        //}
        this.setText("B");
      }
      else if(number.compareTo("C") == 0)
      {
        //if(isNotReturned)
        //{
          this.setBackground(CLarmColor);
        //}
        this.setText("C");
      }
      else if(number.compareTo("D") == 0)
      {
        //if(isNotReturned)
        //{
          this.setBackground(DLarmColor);
        //}
        this.setText("D");
      }
      //annars m�ste det vara ett meddelande qqq??
      else
      {
        //if(isNotReturned)
        //{
        this.setBackground(InfoColor);
        //}
        this.setText(" ");
      }

      //}
      //else
      //  this.setBackground(Color.white);
      //this.setHorizontalTextPosition(SwingConstants.CENTER);
      return this;
    }
  }


  /**
   *  Description of the Class
   *
   *@author     claes
   *@created    December 10, 2002
   */
  private class EventTableCellRender extends DefaultTableCellRenderer
  {

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



  /**
   *  The main program for the HElogTable class
   *
   *@param  args  The command line arguments
   */
  public static void main(String[] args)
  {
    boolean debug = false;
    for(int i = 0; i < args.length; i++)
    {
      if(args[i].equals("-d") || args[i].equals("-D"))
      {
        debug = true;
      }
    }
    JFrame frame = new JFrame();
    JLabel label = new JLabel("hej");
    HElogTable table = new HElogTable((Object)null, debug, label);
    frame.getContentPane().add(table.splitPane, BorderLayout.CENTER);
    frame.addWindowListener(
      new WindowAdapter()
      {
        public void windowClosing(WindowEvent e)
        {
          //close();
          System.exit(0);
        }
      });
    frame.pack();
    frame.setVisible(true);
  }

}

