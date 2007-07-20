/* 
 * Proview   $Id: MhServer.java,v 1.7 2007-07-20 08:45:16 claes Exp $
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


package jpwr.rt;
import java.net.*;
import java.io.*;
import java.util.*;
//for test
import java.sql.Timestamp;
import java.util.Date;
import javax.swing.*;
//end for test
/**
 *  Description of the Class
 *
 *@author     claes, Jonas
 *@created    November 25, 2002
 *@version    0.1 beta: F�rsta testversionen
 */
public class MhServer
{
  public final static int PORT = 4446;
  
  public final static int __IO_EXCEPTION = 2000;

  static int eventCount = 0;

  static int threadCount = 0;
  static int totalThreadCount = 0;

  Mh mh;
  Gdh gdh;
  Errh errh;
  int maxConnections;
  String currentConnectionsStr;
  MhThread mhThread[];
  MhData mhData;
  int maxEvents = 100;
  int maxAlarms = 10;
  static boolean ignoreHandler = false;
  static boolean log = false;
  static boolean logStatistics = false;
  boolean keepRunning = true;

  /**
   *  The main program for the MhServer class
   *
   *@param  args  The command line arguments
   */
  public static void main(String[] args)
  {
    for(int i = 0; i < args.length; i++)
    {
      if(args[i].equals("-i"))
      {
        ignoreHandler = true;
      }
      else if(args[i].equals("-l"))
      {
        log = true;
      }
      else if(args[i].equals("-s"))
      {
        logStatistics = true;
      }
    }
    if(log)
    {
      System.out.println("MhServer starting");
    }

    MhServer mhServer = new MhServer();
    mhServer.openServerSocket();
    for(int i=0;i < mhServer.mhThread.length;i++)
    {
      mhServer.mhThread[i] = null;
    }
    try{
      mhServer.finalize();
    }
    catch(Throwable e) {
    }
    mhServer = null;
    System.out.println("MhServer exiting");
    System.exit(0);
    
  }


  /**
   *  Description of the Method
   */
  public MhServer()
  {
  }


  /**
   *  Gets the handlerObject attribute of the MhServer object
   *
   *@return    The handlerObject value
   */
  private int getHandlerObject()
  {
    if(log)
    {
      System.out.println("i getHandlerObject");
    }
    
    if(ignoreHandler)
    {
      maxConnections = 50;
      return 1;
    }
    CdhrObjid cdhrObjid;
    CdhrString cdhrString;
    CdhrString cdhrStringUser;
    CdhrInt cdhrMaxCon;
    CdhrInt cdhrMaxAlarms;
    CdhrInt cdhrMaxEvents;
    
    cdhrObjid = gdh.getClassList(Pwrb.cClass_WebHandler);
    if(cdhrObjid.evenSts())
    {
      errh.info("No WebHandler is configured, GdhServer terminating");
      return cdhrObjid.getSts();
    }
    cdhrString = gdh.objidToName(cdhrObjid.objid, Cdh.mName_volumeStrict);
    if(cdhrString.evenSts())
    {
      return cdhrString.getSts();
    }
    String userObjectString = cdhrString.str + ".UserObject";
    cdhrStringUser = gdh.getObjectInfoString(userObjectString);
    cdhrUserObj = gdh.nameToObjid(cdhrStringUser.str);
    
    if(cdhrUserObj.evenSts())
    {
      return cdhrUserObj.getSts();
    }
    String maxAlarmsAttr = cdhrStringUser.str + ".MaxNoOfAlarms";
    String maxEventsAttr = cdhrStringUser.str + ".MaxNoOfEvents";
    String maxConAttr = cdhrString.str + ".MaxConnections";
    cdhrMaxAlarms = gdh.getObjectInfoInt(maxAlarmsAttr);
    cdhrMaxEvents = gdh.getObjectInfoInt(maxEventsAttr);
    cdhrMaxCon = gdh.getObjectInfoInt(maxConAttr);
    System.out.println( "MaxNoOfAlarms: " + cdhrStringUser.str + " " + cdhrMaxAlarms.value + " " + cdhrMaxAlarms.evenSts());
    if(cdhrMaxCon.evenSts())
    {
      return cdhrMaxCon.getSts();
    }
    if(cdhrMaxAlarms.evenSts())
    {
      return cdhrMaxAlarms.getSts();
    }
    if(cdhrMaxEvents.evenSts())
    {
      return cdhrMaxEvents.getSts();
    }

    maxConnections = cdhrMaxCon.value;
    maxAlarms = cdhrMaxAlarms.value;
    maxEvents = cdhrMaxEvents.value;
    //pga bugg i proview, MaxNoOfAlarms syns ej i arbetsb�nken
    if(maxAlarms == 0) maxAlarms = 100;
    if(maxEvents == 0) maxEvents = 200;
    //currentConnectionsStr = cdhrString.str + ".CurrentConnections";
    //setCurrentConnections(threadCount);
    errh.info("MhServer started, MaxConnections: " + maxConnections);
    return 1;
  }
  


  /**
   *  Sets the currentConnections attribute of the GdhServer object
   *
   *@param  connections  The new currentConnections value
   */
  private void setCurrentConnections(int connections)
  {
    PwrtStatus sts;

    if(!ignoreHandler)
    {
      sts = gdh.setObjectInfo(currentConnectionsStr, connections);
    }
  }

  CdhrObjid cdhrUserObj;


  /**
   *  Description of the Method
   */
  public void openServerSocket()
  {
    ServerSocket serverSocket = null;
    try
    {
      serverSocket = new ServerSocket(PORT);
      serverSocket.setSoTimeout(1000);
    }

    catch(IOException e)
    {
      System.out.println("IOException i openServerSocket");
      errh.fatal("Could not listen on port " + PORT);
      System.exit(1);
    }

    gdh = new Gdh((Object)null);
    errh = new Errh("MhServer", Errh.eAnix_webmonmh);
    errh.setStatus( Errh.PWR__SRVSTARTUP);
    int sts = getHandlerObject();
    if(sts % 2 == 0)
    {
      errh.setStatus(0);
      return;
    }
    if(log)
    {
      System.out.println("innan mh-skapande");
    }

    mh = new Mh((Object)null, maxAlarms, maxEvents);
    mhThread = new MhThread[maxConnections];
    mhData = new MhData(maxAlarms, maxEvents);
     
    PwrtStatus stsM = mh.outunitConnect(cdhrUserObj.objid);
    if(stsM.evenSts())
    {
      System.out.println("Fel vid outunitConnect");
      errh.setStatus( Errh.PWR__SRVTERM);
      return;
    }

    MhSendThread mhSendThread = new MhSendThread(mh);

     
    errh.setStatus( Errh.PWR__SRUN);
     

    Qcom qcom = new Qcom();
    QcomrCreateQ qque = qcom.createIniEventQ("MhServer");
    if(qque.evenSts())
    {
      System.out.println("MH:Error during qreateque");
      errh.fatal("MH:Error during qcom.createIniEventQ");
      return;
    }

    QcomrGetIniEvent qrGetIniEv;


    while(keepRunning)
    {
      Socket cliSocket = null;
      try
      {
        if(log)
	{
	  System.out.println(" Wait for accept ");
        }
	cliSocket = serverSocket.accept();

      }
      catch(InterruptedIOException e)
      {
	qrGetIniEv = qcom.getIniEvent(qque.qix, qque.nid, 0);
        if(qrGetIniEv.timeout)
	{
	  //do nothing
          continue;
	}
	else if(qrGetIniEv.terminate)
	{
	  //Time to die
          System.out.println("MhServer received killmess from QCom");
	  return;
	}
	else 
        {
	  //All other messages is qurrently ignored
	  //But perhaps we should reinitialize when we get
	  //swapdone
	  continue;
	}

      }
      catch(IOException e)
      {
        errh.error("Accept failed.");
        continue;
      }
      if(log)
      {
        System.out.println("threadcount " + threadCount);
      } 
      
      if(threadCount <= maxConnections)
      {
        
        //setCurrentConnections(threadCount);
        for(int i = 0;i < mhThread.length;i++)
	{
	  if(log)
	  {
	    System.out.println("letar efter mhThread");
	  }
	  if(mhThread[i] == null || !mhThread[i].keepRunning)
	  {
	    mhThread[i] = null;
	    mhThread[i] = new MhThread(cliSocket, mh);
	    // Create a new thread
            threadCount++;
            totalThreadCount++;
	    if(log)
	    {
	      System.out.println("Ny klient index " + i);
	    }
	    break;
	  }
	}
      }
      else
      {
        //errh.warning("Connection dismissed, max number of connections exceeded");
        try
        {
	  ObjectOutputStream out = new ObjectOutputStream(cliSocket.getOutputStream());
          out.writeInt(-1);
	  out.flush();
	  out.close();
	  cliSocket.close();
        }
        catch(IOException e2)
        {
          //errh.error("Connection close failed");
        }
      }
    }

  }

  protected void finalize() throws Throwable
  {
      super.finalize();
    
      /* Qcom might no be alive...
      System.out.println("innan disconnect");
      PwrtStatus stsM = mh.outunitDisConnect();
      System.out.println("Status disconnect " + stsM.getSts());
      */
  }
    
  private class MhSendThread extends Thread
  {
    Mh mh;
    boolean keepRunning = true;
    public MhSendThread(Mh mh)
    {
      this.mh = mh;
      start();
    }
    public void run()
    {
       PwrtStatus stsM = new PwrtStatus(0);
       if(log)
       {
         System.out.println(totalThreadCount + " " + mhThread.length);
       }
       while(this.keepRunning)
       {
         try
	 {
           if(mh.hasNewMessArrived())
           {
	     //System.out.println("messReceiveSts " + stsM.sts);
             MhrEvent newMess = mh.getNewMess();
             synchronized(mhData)
             {
	       mhData.insertNewMess(newMess);
	     }
	     
	     //System.out.println("efter insertNewMess");
	     for(int i = 0;i < totalThreadCount && i < mhThread.length;i++)
	     {
	       //System.out.println("Nu skall sendMess anropas");
	       //det kan ju vara s� att n�gon klient har avslutats 
	       if(mhThread[i] == null)
	       {
	         if(log)
		 {
		   System.out.println("mhThread[" + i + "] �r null");
	         }
		 continue;
	       }
	       if(mhThread[i].initDone && mhThread[i].keepRunning)
	       {
	         mhThread[i].sendMess(newMess);
	       }
	     }
           }
	  
           stsM = mh.outunitReceive();
       
           Thread.sleep(1);
         }
         catch(Exception e)
         {
	   if(log)
	   {
	     System.out.println("Exception i run mhSendThread " + e.toString());
	   }
	 }
      }
    }
  }


  
  private class MhThread extends Thread
  {
    Socket socket;
    ObjectInputStream in;
    ObjectOutputStream out;
    public boolean initDone = false;
    Mh mh;
    boolean keepRunning = true;
    public MhThread(Socket socket, Mh mh)
    {
      this.socket = socket;
      this.mh = mh;
      try
      {
        this.socket.setKeepAlive(true);
        out = new ObjectOutputStream(socket.getOutputStream());
	in = new ObjectInputStream(socket.getInputStream());
      }
      catch(IOException e)
      {
        this.keepRunning = false;
	System.out.println("IOException vid skapande av str�mmar mot klient");
	errh.error("DataStream failed");
        threadCount--;
        //setCurrentConnections(threadCount);
        return;
      }
      start();
    }
    public void run()
    {
      try
      {
        try
        {
	  out.writeInt(maxAlarms);
	  out.writeInt(maxEvents);
	  synchronized(mhData)
	  {
	    int nrOfAlarms = mhData.getNrOfAlarms();
	    out.writeInt(nrOfAlarms);
	    if(log)
	    {
	      System.out.println("Larmstorlek: " + nrOfAlarms);
	    }
	    if(nrOfAlarms > 0)
	    {
	      out.writeObject(mhData.alarmVec);
	    }
	    int nrOfEvents = mhData.getNrOfEvents();
	    out.writeInt(nrOfEvents);
	    if(log)
	    {
	      System.out.println("H�ndelsestorlek: " + nrOfEvents);
	    }
	    if(nrOfEvents > 0)
	    {
	      out.writeObject(mhData.eventVec);
	    
	    }
	    out.flush();
	    out.reset();
	  }
        }
        catch(Exception e)
        {
          System.out.println(e.toString());
	  this.keepRunning = false;
        }

	initDone = true;
        
	while(this.keepRunning)
        {
	  //h�r skall vi ligga och v�nta p� meddelanden fr�n klienterna
	  //typ alarmkvittens och dylikt
	  try
	  {
	    
	    MhrsEventId id = (MhrsEventId)in.readObject();
	    if(log)
	    {
	      System.out.println("f�tt meddelande" + id.idx + " " + id.nix + " " +
	                          id.birthTime);
	    }
	    PwrtStatus sts = mh.outunitAck(id);
	  }
	  catch(Exception e)
	  {
	    System.out.println("Exception i receivemesstr�d " + e.toString());
	    this.keepRunning = false;
	  }
	  Thread.sleep(3);
        }
      }
      catch(Exception e)
      {
        System.out.println("exception i run MhThread");
	System.out.println("avslutar tr�den");
      }
      finally
      {
        System.out.println("finally");
	this.keepRunning = false;
        try
	{
	  out.close();
	}
	catch(Exception e)
	{
	}
	try
	{
	  in.close();
	}
	catch(Exception e)
	{
	  System.out.println("Closing client socket");
	}
	try
	{
	  socket.close();
	}
	catch(Exception e)
	{
	}

      }
    }
    /**
    * Anropas var g�ng ett nytt meddelande skall skickas till klienterna
    */
    public void sendMess(MhrEvent event)
    {
      //System.out.println("Skickar nytt meddelande");
      try
      {
        out.writeObject(event);
	out.flush();
	out.reset();
      }
      catch(IOException e)
      {
        System.out.println("IOException vid skrivande till klient");
      }
    } 
  }//MhThread

}//MhServer

