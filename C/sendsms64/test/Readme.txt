/**
* RIL_UNSOL_RESPONSE_NEW_SMS
*
* Called when new SMS is received.
*
* "data" is const char *
* This is a pointer to a string containing the PDU of an SMS-DELIVER
* as an ascii string of hex digits. The PDU starts with the SMSC address
* per TS 27.005 (+CMT:)
*
* Callee will subsequently confirm the receipt of thei SMS with a
* RIL_REQUEST_SMS_ACKNOWLEDGE
*
* No new RIL_UNSOL_RESPONSE_NEW_SMS
* or RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT messages should be sent until a
* RIL_REQUEST_SMS_ACKNOWLEDGE has been received
*/

#define RIL_UNSOL_RESPONSE_NEW_SMS 1003

/**
* RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT
*
* Called when new SMS Status Report is received.
*
* "data" is const char *
* This is a pointer to a string containing the PDU of an SMS-STATUS-REPORT
* as an ascii string of hex digits. The PDU starts with the SMSC address
* per TS 27.005 (+CDS:).
*
* Callee will subsequently confirm the receipt of the SMS with a
* RIL_REQUEST_SMS_ACKNOWLEDGE
*
* No new RIL_UNSOL_RESPONSE_NEW_SMS
* or RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT messages should be sent until a
* RIL_REQUEST_SMS_ACKNOWLEDGE has been received
*/

#define RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT 1004
