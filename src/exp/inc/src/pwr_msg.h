#ifndef pwr_msg_h
#define pwr_msg_h

#ifdef __cplusplus
extern "C" {
#endif

/* Included from VMS */ 

#define MSG_M_SEVERITY 7
					/* ccccffffffffffff mmmmmmmmmmmmmsss */
#define MSG_M_COND_ID 268435448		/* 0000111111111111 1111111111111000 */
#define MSG_M_CONTROL -268435456	/* 1111000000000000 0000000000000000 */
#define MSG_M_SUCCESS 1
#define MSG_M_MSG_NO 65528		/* 0000000000000000 1111111111111000 */
#define MSG_M_CODE 32760		/* 0000000000000000 0111111111111000 */
#define MSG_M_FAC_SP 32768		/* 0000000000000000 1000000000000000 */
#define MSG_M_CUST_DEF 134217728	/* 0000100000000000 0000000000000000 */
#define MSG_M_INHIB_MSG 268435456	/* 0001000000000000 0000000000000000 */
#define MSG_M_FAC_NO 268369920		/* 0000111111111111 0000000000000000 */
#define MSG_M_CLEAR_CODE 0xffff8007	/* 1111111111111111 1000000000000111 */

#define MSG_K_WARNING 0                 /* WARNING                          */
#define MSG_K_SUCCESS 1                 /* SUCCESSFUL COMPLETION            */
#define MSG_K_ERROR 2                   /* ERROR                            */
#define MSG_K_INFO 3                    /* INFORMATION                      */
#define MSG_K_SEVERE 4                  /* SEVERE ERROR                     */
#define MSG_K_FATAL 4			/* SEVERE ERROR                     */

#define MSG_S_CODE       0x0C
#define MSG_S_COND_ID    0x19
#define MSG_S_CONTROL    0x04
#define MSG_S_FAC_NO     0x0C
#define MSG_S_MSG_NO     0x0D
#define MSG_S_SEVERITY   0x03

#define MSG_V_CODE       0x03
#define MSG_V_COND_ID    0x03
#define MSG_V_CONTROL    0x1C
#define MSG_V_CUST_DEF   0x1B
#define MSG_V_FAC_NO     0x10
#define MSG_V_FAC_SP     0x0F
#define MSG_V_INHIB_MSG  0x1C
#define MSG_V_MSG_NO     0x03
#define MSG_V_SEVERITY   0x00
#define MSG_V_SUCCESS    0x00

/* Define MACROS to extract individual fields from a status value */

#define MSG_STATUS_CODE(code) 		(((code) & MSG_M_CODE)	    >> MSG_V_CODE)
#define MSG_STATUS_COND_ID(code) 	(((code) & MSG_M_COND_ID)   >> MSG_V_COND_ID)
#define MSG_STATUS_CONTROL(code) 	(((code) & MSG_M_CONTROL)   >> MSG_V_CONTROL)
#define MSG_STATUS_CUST_DEF(code) 	(((code) & MSG_M_CUST_DEF)  >> MSG_V_CUST_DEF)
#define MSG_STATUS_FAC_NO(code) 	(((code) & MSG_M_FAC_NO)    >> MSG_V_FAC_NO)
#define MSG_STATUS_FAC_SP(code) 	(((code) & MSG_M_FAC_SP)    >> MSG_V_FAC_SP)
#define MSG_STATUS_INHIB_MSG(code)	(((code) & MSG_M_INHIB_MSG) >> MSG_V_INHIB_MSG)
#define MSG_STATUS_MSG_NO(code) 	(((code) & MSG_M_MSG_NO)    >> MSG_V_MSG_NO)
#define MSG_STATUS_SEVERITY(code) 	(((code) & MSG_M_SEVERITY)  >> MSG_V_SEVERITY)
#define MSG_STATUS_SUCCESS(code) 	(((code) & MSG_M_SUCCESS)   >> MSG_V_SUCCESS)

#define MSG_CLEAR_CODE(code)		((code) & MSG_M_CLEAR_CODE)

#define MSG_SET_WARNING(code)		(((code) & ~MSG_M_SEVERITY) | MSG_K_WARNING)
#define MSG_SET_SUCCESS(code)		(((code) & ~MSG_M_SEVERITY) | MSG_K_SUCCESS)
#define MSG_SET_ERROR(code)		(((code) & ~MSG_M_SEVERITY) | MSG_K_ERROR)
#define MSG_SET_INFO(code)		(((code) & ~MSG_M_SEVERITY) | MSG_K_INFO)
#define MSG_SET_SEVERE(code)		(((code) & ~MSG_M_SEVERITY) | MSG_K_SEVERE)
#define MSG_SET_FATAL(code)		(((code) & ~MSG_M_SEVERITY) | MSG_K_FATAL)

#define MSG_NOF(arr) ((int) (sizeof(arr) / sizeof(arr[0])))

typedef enum {
  msg_eSeverity_Warning	= 0,
  msg_eSeverity_Success = 1,
  msg_eSeverity_Error   = 2,
  msg_eSeverity_Info	= 3,
  msg_eSeverity_Fatal	= 4
} msg_eSeverity;


typedef struct {
  char	*MsgName;
  char 	*MsgTxt;
} msg_sMsg;

typedef struct {
  int  	FacNum;
  char *FacName;
  char *Prefix;
  int  NofMsg;
  msg_sMsg *Msg;
} msg_sFacility;

typedef struct {
  int		NofFacility;
  msg_sFacility **Facility;
} msg_sHead;

#endif

#ifdef __cplusplus
}
#endif

