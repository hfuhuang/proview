package jpwr.rt;
import java.io.Serializable; 
/**
 *  Description of the Class
 *
 *@author     claes
 *@created    November 26, 2002
 */
public class MhrEvent implements Serializable
{
  public String eventTime;
  public String eventText;
  public String eventName;
  public int eventFlags;
  public int eventStatus;
  public int eventPrio;
  public MhrsEventId eventId;
  public int eventType;
  public PwrtObjid object;

  public MhrEvent(String eventText, 
                  String eventName, 
		  String eventTime, 
		  int eventFlags, 
		  int eventPrio, 
		  int eventStatus, 
		  int eventId_nix, 
		  String eventId_birthTime, 
		  int eventId_idx,
		  int eventType,
		  PwrtObjid object)
  {
    this.eventText = eventText;
    this.eventName = eventName;
    this.eventTime = eventTime;
    this.eventFlags = eventFlags;
    this.eventPrio = eventPrio;
    this.eventStatus = eventStatus;
    this.eventId = new MhrsEventId(eventId_nix, eventId_birthTime, eventId_idx); 
    this.eventType = eventType;
    this.object = object;
  }
  public MhrEvent getCopy()
  {
    return new MhrEvent(eventText, 
                        eventName, 
		        eventTime, 
		        eventFlags, 
		        eventPrio, 
		        eventStatus, 
		        eventId.nix, 
		        eventId.birthTime, 
		        eventId.idx,
		        eventType,
		        object);
  }

}

