package jpwr.rt;
import java.io.*;

public class RtUtilities {

  // Locate the first occurence of a bytearray in another bytearray
  public static int strStr( char[] buffer, char[] str, int b_len, int s_len) {
    int i, j;
    for ( i = 0; i < b_len - s_len; i++) {
      boolean match = true;
      for ( j = 0; j < s_len; j++) {
	if ( buffer[i+j] != str[j]) {
	  match = false;
	  break;
        }
      }
      if ( match)
	return i;
    }
    return -1;
  }

  //
  // Copy a html-file and insert the value of the instance parameter
  //
  public static int createInstanceFile( String from_name, String to_name,
					String instance) {
    File from_file = new File( from_name);
    String from_parent = from_file.getParent();

    // Check source file
    if ( from_parent == null) {
      System.out.println( "Directory specification is missing: " + from_name);
      return 0;
    }
    if ( !from_file.exists()) {
      System.out.println( "Can't find file: " + from_name);
      return 0;
    }
    if ( !from_file.canRead()) {
      System.out.println( "No permission: " + from_name);
      return 0;
    }

    File to_file = new File( from_parent + "/" + to_name);

    // Check destination file
    if ( to_file.exists()) {
      if ( !to_file.canWrite()) {
	System.out.println( "No write premission: " + to_name);
        return 0;
      }
    }
    else {
      File dir = new File( from_parent);
      if ( !dir.canWrite()) {
	System.out.println( "No write permission: " + from_parent);
        return 0;
      }
    }

    FileReader from = null;
    FileWriter to = null;

    // Copy the file
    try {
      from = new FileReader( from_file);
      to = new FileWriter( to_file);
      char[] buffer = new char[4096];
      String searchString1 = new String("\"instance\" VALUE=\"\"");
      char[] searchBuffer1 = new char[19];
      int searchLen1 = searchString1.length();
      searchString1.getChars( 0, searchLen1, searchBuffer1, 0);
      String searchString2 = new String("instance=\"\"");
      char[] searchBuffer2 = new char[11];
      int searchLen2 = searchString2.length();
      searchString2.getChars( 0, searchLen2, searchBuffer2, 0);
      char[] instanceBuff = new char[120];
      instance.getChars( 0, instance.length(), instanceBuff, 0);

      int len;
      int offs1;
      int offs2;

      // Replace ��� in instance name because mozilla can't pass them as 
      // parameter
      for ( int i = 0; i < instance.length(); i++) {
        if ( instanceBuff[i] == '�' || instanceBuff[i] == '�')
          instanceBuff[i] = '\\';
        if ( instanceBuff[i] == '�' || instanceBuff[i] == '�')
          instanceBuff[i] = '/';
        if ( instanceBuff[i] == '�' || instanceBuff[i] == '�')
          instanceBuff[i] = '@';
      }

      while( (len = from.read( buffer, 0, 4096)) != -1) {
        String s = new String( buffer, 0, len);

        if ( (offs1 = strStr( buffer, searchBuffer1, len, searchLen1)) != -1 &&
             (offs2 = strStr( buffer, searchBuffer2, len, searchLen2)) != -1) {
	  to.write( buffer, 0, offs1 + searchLen1 - 1);
          to.write( instanceBuff, 0, instance.length());
          to.write( buffer, offs1 + searchLen1 - 1, 
		    offs2 + searchLen2 - 1 - (offs1 + searchLen1 - 1));
          to.write( instanceBuff, 0, instance.length());
          to.write( buffer, offs2 + searchLen2 - 1, 
		    len - (offs2 + searchLen2 - 1));
        }
        else {
	  System.out.println( "html-file corrupt: " + from_name);
        }
      }
    }
    catch( IOException e) {
      System.out.println( "IOExeption i file " + from_name + " or " +
				to_name);
      return 0;
    }
    finally {
      try {
	if ( from != null)
          from.close();
        if ( to != null)
          to.close();
      }
      catch ( IOException e) {}
    }
    return 1;
  }

  //
  // Replace all occurences of subStr1 to subStr2 in oldStr
  // The maxsize of the result is 500
  //
  public static String strReplace( String oldStr, String subStr1, String subStr2)
  {
    int oldLen = oldStr.length();
    int subLen1 = subStr1.length();
    int subLen2 = subStr2.length();
    char[] result = new char[1000];
    char[] oldChar = new char[oldLen];
    oldStr.getChars( 0, oldLen, oldChar, 0);
    char[] subChar1 = new char[subLen1];
    subStr1.getChars( 0, subLen1, subChar1, 0);
    char[] subChar2 = new char[subLen2];
    subStr2.getChars( 0, subLen2, subChar2, 0);
    int i, j;
    int resultOffs = 0;
    int oldOffs = 0;

    for ( i = 0; i < oldLen - subLen1 + 1; i++) {
      boolean match = true;
      for ( j = 0; j < subLen1; j++) {
	if ( oldChar[i+j] != subChar1[j]) {
	  match = false;
	  break;
	}
      }
      if ( match) {
	for ( j = 0; j < i - oldOffs; j++)
	  result[resultOffs + j] = oldChar[ oldOffs + j];
	resultOffs += i - oldOffs;
        oldOffs = i + subLen1;
        for ( j = 0; j < subLen2; j++)
	  result[resultOffs + j] = subChar2[j];
        resultOffs += subLen2;
        i += subLen1;
      }
    }
    for ( j = 0; j < oldLen - oldOffs; j++)
      result[resultOffs + j] = oldChar[ oldOffs + j];
    resultOffs += oldLen - oldOffs;

    String resultStr = new String( result, 0, resultOffs);
    return resultStr;
  }
}







