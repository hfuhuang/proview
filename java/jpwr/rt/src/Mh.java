/* 
 * Proview   $Id: Mh.java,v 1.4 2007-02-07 08:22:37 claes Exp $
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

/**
 *  Title: Mh.java Description: Klass som fungerar som en port mot
 *  meddelandehanteraren Copyright: <p>
 *
 *  Company SSAB<p>
 *
 *
 *
 *@author     JN
 *@version    1.0
 */

package jpwr.rt;
import java.util.Vector;
/**
 *  Description of the Class
 *
 *@author     Jonas Nylund
 *@created    November 25, 2002
 */
public class Mh
{
  static
  {
    System.loadLibrary("jpwr_rt_gdh");
    initIDs();  
  }

  /**
   *@author                            claes
   *@created                           November 26, 2002
   *@ingroup                           MSGH_DS
   *@brief                             Defines a bit pattern.
   *@param  mh_mEventFlags_Return      Setting this flag enables a return
   *      message associated with this message to be shown in the event list.
   *@param  mh_mEventFlags_Ack         Setting this flag enables an
   *      acknowledgement message associated with this message to be shown in
   *      the event list.
   *@param  mh_mEventFlags_Bell
   *@param  mh_mEventFlags_Force
   *@param  mh_mEventFlags_InfoWindow
   *@param  mh_mEventFlags_Returned
   *@param  mh_mEventFlags_NoObject
   */
  public static final int mh_mEventFlags_Return = 0x01;
  public static final int mh_mEventFlags_Ack = 0x02;
  public static final int mh_mEventFlags_Bell = 0x04;
  public static final int mh_mEventFlags_Force = 0x08;
  public static final int mh_mEventFlags_InfoWindow = 0x10;
  public static final int mh_mEventFlags_Returned = 0x20;
  public static final int mh_mEventFlags_NoObject = 0x40;
  
  
  public static final  int mh_mEventStatus_NotRet = (1 << 0);
  public static final  int mh_mEventStatus_NotAck = (1 << 1);
  public static final  int mh_mEventStatus_Block  = (1 << 2);
  


 /** 
 * @ingroup MSGH_DS
 * @brief Event prio
 *
 * This enumeration defines the priority of the event. 
 * This affects how the message handler treats the generated message. 
 * For A and B priorities the alarm window displays number of alarms, 
 * number of unacknowledged alarms, identities of the alarms, and associated 
 * message texts. For C and D priorities, only number of alarms and number of 
 * unacknowledged alarms are shown. 
 * @param mh_eEventPrio_A Priority A, the highest priority. 
 * Alarm messages of this priority are shown in the upper part of the alarm window. 
 * @param mh_eEventPrio_B Priority B. 
 * These messages are shown in the lower part of the alarm window. 
 * @param mh_eEventPrio_C Priority C. 
 * @param mh_eEventPrio_D Priority D. This is the lowest priority. 
 */
  public static final int mh_eEventPrio__ = 0;   
  public static final int mh_eEventPrio_A = 67;  
  public static final int mh_eEventPrio_B = 66; 
  public static final int mh_eEventPrio_C = 65;
  public static final int mh_eEventPrio_D = 64;
  public static final int mh_eEventPrio_  = 63;
  
  
  public static final int mh_eEvent__		= 0;    
  public static final int mh_eEvent_Ack		= 1;
  public static final int mh_eEvent_Block	= 2;
  public static final int mh_eEvent_Cancel	= 3;
  public static final int mh_eEvent_CancelBlock	= 4;
  public static final int mh_eEvent_Missing	= 5;
  public static final int mh_eEvent_Reblock	= 6;
  public static final int mh_eEvent_Return	= 7;
  public static final int mh_eEvent_Unblock	= 8;
  public static final int mh_eEvent_Info	= 32;
  public static final int mh_eEvent_Alarm	= 64;
  public static final int mh_eEvent_   		= 65;
  
  public static final int EventType_ClearAlarmList = 66;
  //public static final int EventType_Return = 1;
  //public static final int EventType_Ack = 2;
  //public static final int EventType_Alarm = 3;

  static MhrEvent lastMhrEvent;
  static boolean newAlarmArrived = false;
  static boolean newEventArrived = false;
  static boolean clearAlarmList = false;
  static int nrOfAlarmsArrived = 0;
  
  static int maxNoOfAlarms;
  static int maxNoOfEvents;
  
  private boolean keepRunning = true;
  
  private String mess = "dummymess";
  private static boolean initDone = false;

  private static String currentSystemGroup = null;
  private static String currentUser = null;
  private static String currentPassword = null;
  private static int currentPrivilege = Pwr.mAccess_AllPwr;



  /**
   *  Constructor for the Mh object
   *
   *@param  root  Description of the Parameter
   */
  public Mh(Object root, int maxNoOfAlarms, int maxNoOfEvents)
  {
    
    if(!initDone)
    {
      Mh.maxNoOfAlarms = maxNoOfAlarms;
      Mh.maxNoOfEvents = maxNoOfEvents;
      initDone = true;
    }
  }


  /**
   *  Description of the Method
   */
  public void close()
  {
  }


  /**
   *  Description of the Method
   *
   *@param  lockRejected  Description of the Parameter
   */
  public void printStatistics(int lockRejected)
  {
  }


  /**
   *  Description of the Method
   *
   *@param  user      Description of the Parameter
   *@param  password  Description of the Parameter
   *@return           Description of the Return Value
   */
  public int login(String user, String password)
  {
    // Get system group
    String systemGroup = "SSAB";
    CdhrInt ret = RtSecurity.checkUser(systemGroup, user,
      password);
    if(ret.evenSts())
    {
      logout();
      return ret.getSts();
    }
    currentSystemGroup = systemGroup;
    currentUser = user;
    currentPassword = password;
    currentPrivilege = ret.value;
    return 1;
  }


  /**
   *  Description of the Method
   */
  public void logout()
  {
    currentSystemGroup = null;
    currentUser = null;
    currentPassword = null;
    currentPrivilege = Pwr.mAccess_AllPwr;
  }


  /**
   *  Description of the Method
   *
   *@return    Description of the Return Value
   */
  public int checkUser()
  {
    return 1;
  }


  /**
   *  Gets the user attribute of the Mh object
   *
   *@return    The user value
   */
  public String getUser()
  {
    return currentUser;
  }


  /**
   *  Gets the authorized attribute of the Mh object
   *
   *@param  access  Description of the Parameter
   *@return         The authorized value
   */
  public boolean isAuthorized(int access)
  {
    return (access & currentPrivilege) != 0;
  }


  /**
   *  Description of the Method
   *
   *@param  from      Description of the Parameter
   *@param  to        Description of the Parameter
   *@param  instance  Description of the Parameter
   *@return           Description of the Return Value
   */
  public PwrtStatus createInstanceFile(String from, String to,
                                       String instance)
  {
    // Dummy
    return new PwrtStatus(0);
  }


  /**
   *  Description of the Method
   *
   *@param  str  Description of the Parameter
   */
  public void logString(String str)
  {
    // Dummy
  }


  public static void callBack()
  {
    System.out.println("callback funkar ju");
  }
  /**
   *  Anropas av Callbackfunktionerna
   *
   *@param  messType    Meddelandetyp
   *@param  messString  Meddelande
   */
  public static void messReceived(String messString, String nameString, String timeString)
  {
    //System.out.println("Larm " + timeString + " " + messString + " " + nameString);
    //hantera det mottagna meddelandet
    //this.newMessArrived = true;
    //notify();

  }

  public static void messReceived(String messString, 
                                  String nameString, 
				  String timeString, 
				  int flags, 
				  int prio, 
				  int status, 
				  int eventId_nix, 
				  String eventId_birthTime, 
				  int eventId_idx, 
				  int targetId_nix, 
				  String targetId_birthTime, 
				  int targetId_idx, 
				  int eventType, 
				  PwrtObjid object)
  {
    //System.out.println("Larm " + timeString + " " + messString + " " + nameString + " flags " + flags + " prio " + prio + " sts " +
    //status +  " nix " + eventId_nix + " birttime " + eventId_birthTime + " idx " + eventId_idx +
    //" typ " + eventType);
    //hantera det mottagna meddelandet
    //System.out.println("messReceived " + eventType);
    MhrEvent evItem =  new MhrEvent(messString, 
                                    nameString,
                                    timeString, 
				    flags, 
				    prio, 
				    status, 
				    eventId_nix, eventId_birthTime, eventId_idx, 
				    targetId_nix, targetId_birthTime, targetId_idx, 
				    eventType, object);
    lastMhrEvent = evItem;
    newAlarmArrived = true;
    
    nrOfAlarmsArrived++; //beh�vs ej

  }

  public boolean hasNewMessArrived()
  {
    if(!newAlarmArrived)
      return newAlarmArrived;
    newAlarmArrived = false;
    return true; 
    
  }


  /**
   *  Returnerar det nya meddelandet som har kommit. Skall endast anropas om
   *  <tag>hasNewMessArrived()</tag> returnerar true
   *
   *@return    Det nya meddelandet som har kommit
   */
  public MhrEvent getNewMess()
  { 
    return lastMhrEvent;
  }


  /**
   *  Se Programmers reference manual f�r beskrivning
   */
  //public native PwrtStatus outunitBlock(PwrtObjid object, MheEventPrio prio);


  private native static void initIDs();

  /**
   *  S�nder ett kvittensmeddelande till meddelandehanteraren
   *
   *@param  id  Identiteten f�r h�ndelsen som skall kvitteras
   *@return          Status. Koder: %MH-S-SUCCES %MH-S-ACKBUFF
   */
  public native PwrtStatus outunitAck(MhrsEventId id);



  /**
   *  Kopplar en outunit till den lokala meddelandehanteraren
   *
   *@param  outunit  Objektsidentiteten f�r UserObjektet
   *@return          Status. Koder: %MH-S-SUCCES
   */
  public native PwrtStatus outunitConnect(PwrtObjid outunit);


  /**
   *  Kopplar bort en outunit fr�n den lokala meddelandehanteraren
   *
   *@return    Status. Koder: %MH-S-SUCCES
   */
  public native PwrtStatus outunitDisConnect();


  /**
   *  Den h�r funktionen l�ser meddelandek�n f�r en outunit. Om det finns ett
   *  meddelande kommer messReceived att anropas
   *
   *@return    Status. Koder: %MH-S-SUCCES
   */
  public native PwrtStatus outunitReceive();


  /**
   *  Se Programmers reference manual f�r beskrivning
   *
   *@param  object  Identiteten f�r objektet som skall blockeras
   *@return         Status. Koder: %MH-S-SUCCES, %MH-W-NOTBLOCK redan
   *      avblockerad , %GDH-F-NOSUCHOBJ objektet �r inte tillg�ngligt
   */
  public native PwrtStatus outunitUnBlock(PwrtObjid object);


  /**
   *  Se Programmers reference manual f�r beskrivning
   *
   *@return    Status. Koder: %MH-S-SUCCES
   */
  public native PwrtStatus outunitUpdate();



  /**
   *  Description of the Method
   *
   *@param  filename  Description of the Parameter
   *@return           Description of the Return Value
   */
  public native static String translateFilename(String filename);
}


