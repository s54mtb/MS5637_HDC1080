/**
  ******************************************************************************
  * @file    command.c
  * @brief   Command line interpreter.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 S54MTB</center></h2>
  *
  ******************************************************************************
  */ 


#include "stm32f0xx.h"                  // Device header

#include <stdio.h>
#include <string.h>  // strcmp
#include <ctype.h>   // toupper
#include <stdlib.h>
#include <string.h>


#define MAX_CMD_LEN 128

/** Globals */
static char cmdstr_buf [1 + MAX_CMD_LEN];
static char argstr_buf [1 + MAX_CMD_LEN];
char newline [1 + MAX_CMD_LEN];


///* Flash storage for settings at absolute address */
//const settings_t settings_Store[SETTINGS_CATLEN] __attribute__((at(FLASH_STORE_ADR)));
//const settings_t backup_settings_Store[SETTINGS_CATLEN] __attribute__((at(FLASH_BACKUP_ADR)));

extern void uart_puts(char *str);

/**
 *
 * Command identifiers	
 */
enum
  {
		CMD_TEMPERATURE,
		CMD_HUMIDITY,
		CMD_THERMISTOR,
		CMD_IDENT,
		CMD_ADDRESS,
		// Add more 
		CMD_LAST
  };


// command table
struct cmd_st
  {
  const char *cmdstr;
  int id;
  };


	/**
   * @brief Help text 
   */
const char helptext[] = 
"Commands:\n\r"
"---------\n\r"
"TE{MPERATURE}\n\r"
"HU{MIDITY}\n\r"
"TH{ERMISTOR}\n\r"
"ID{ENT}\n\r"
"AD{DRESS}\n\r"
;

/**
 *	Command strings - match command with command ID 
 */
const struct cmd_st cmd_tbl [] =
  {
		{	"TEMPERATURE",	CMD_TEMPERATURE, },
		{ "HUMIDITY",     CMD_HUMIDITY,   },
		{ "THERMISTOR",   CMD_THERMISTOR,  },
		{ "IDENT",        CMD_IDENT,   },		
		{	"TE",				 		CMD_TEMPERATURE, },
		{ "HU",        		CMD_HUMIDITY,   },
		{ "TH",       		CMD_THERMISTOR,  },
		{ "ID",        		CMD_IDENT,   },		
		{ "ADDRESS",      CMD_ADDRESS,   },		
		{ "AD",        		CMD_ADDRESS,   },		
  };
	
#define CMD_TBL_LEN (sizeof (cmd_tbl) / sizeof (cmd_tbl [0]))
	
/********** Command functions ***********/
void cmd_temperature(char *argstr_buf);
void cmd_humidity(char *argstr_buf);
void cmd_thermistor(char *argstr_buf);
void cmd_ident(char *argstr_buf);
void cmd_address(char *argstr_buf);

void cmd_unknown(char *argstr_buf);

/*********************************************************************
 * Function:        static unsigned char cmdid_search
 * PreCondition:    -
 * Input:           command string  
 * Output:          command identifier
 * Side Effects:    -
 * Overview:        This function searches the cmd_tbl for a specific 
 *					command and returns the ID associated with that 
 *					command or CID_LAST if there is no matching command.
 * Note:            None
 ********************************************************************/
static int cmdid_search (char *cmdstr) {
	const struct cmd_st *ctp;

	for (ctp = cmd_tbl; ctp < &cmd_tbl [CMD_TBL_LEN]; ctp++) {
		if (strcmp (ctp->cmdstr, cmdstr) == 0) return (ctp->id);
	}

	return (CMD_LAST);
}


/*********************************************************************
 * Function:        char *strupr ( char *src)
 * PreCondition:    -
 * Input:           string  
 * Output:          Uppercase of string
 * Side Effects:    -
 * Overview:        change to uppercase
 * Note:            None
 ********************************************************************/
char *strupr (char *src) {
	char *s;

	for (s = src; *s != '\0'; s++)
		*s = toupper (*s);

	return (src);
}


/*********************************************************************
 * Function:        void cmd_proc (const char *cmd)
 * PreCondition:    -
 * Input:           command line  
 * Output:         	None
 * Side Effects:    Depends on command
 * Overview:        This function processes the cmd command.
 * Note:            The "big case" is here
 ********************************************************************/
void cmd_proc (char *cmd)
{
	char *argsep;
	unsigned int id;
//	char tmpstr_buf[MAX_CMD_LEN];

/*------------------------------------------------
First, copy the command and convert it to all
uppercase.
------------------------------------------------*/
	strncpy (cmdstr_buf, cmd, sizeof (cmdstr_buf) - 1);
	cmdstr_buf [sizeof (cmdstr_buf) - 1] = '\0';
	strupr (cmdstr_buf);
	//skip empty commands
  if (cmdstr_buf[0] == '\0')
		return;
/*------------------------------------------------
Next, find the end of the first thing in the
buffer.  Since the command ends with a space,
we'll look for that.  NULL-Terminate the command
and keep a pointer to the arguments.
------------------------------------------------*/
	argsep = strchr (cmdstr_buf, ' ');
	
	if (argsep == NULL) {
	  argstr_buf [0] = '\0';
	} else {
	  strcpy (argstr_buf, argsep + 1);
	  *argsep = '\0';
	}

/*------------------------------------------------
Search for a command ID, then switch on it.  Each
function invoked here.
------------------------------------------------*/
	id = cmdid_search (cmdstr_buf);
	
	switch (id)
	{
		case CMD_TEMPERATURE:
			cmd_temperature(argstr_buf);	
		break;
				
		case CMD_HUMIDITY:
			cmd_humidity(argstr_buf);	
		break;
				
		case CMD_THERMISTOR:
			cmd_thermistor(argstr_buf);	
		break;
				
		case CMD_IDENT:
			cmd_ident(argstr_buf);	
		break;
		
		case CMD_ADDRESS:
			cmd_address(argstr_buf);	
		break;
				
		case CMD_LAST:
			cmd_unknown(cmdstr_buf);
		break;
	}
}


/**
   * @brief Read temperature from Si7013 
   * @param Arguments string from command
   * @param None
   * @retval None
   */
void cmd_temperature(char *argstr_buf)
{	

	
//	char *argsep;
//	char param[32];
//	int x;

//	argsep = strchr (argstr_buf, ' ');

//	if (argsep != NULL) 
//	{
//		strcpy (param, argsep + 1);
//    *argsep = '\0';
//  }	else argstr_buf[0] = 'D';

//	switch 	(argstr_buf[0])
//	{
//		case 'V' : // Volume
//			x = atoi(param);
//			if ((x<=100) & (x>0)) settings.audio.volume = x;	
//				Audio_Init(settings);
//		break;
//		
//		case 'R' : // Run
//			x = atoi(param);
//			Audio_Run(x);
//		break;
//		
//		case 'F' : // Frequency
//			x = atoi(param);
//			if ((x<10000) & (x>100)) settings.audio.frequency = x;	
//				Audio_Init(settings);
//		break;
//		
//		case 'C' : // CW
//			x = atoi(param);
//			if (x>0) settings.audio.cw = 1;	else settings.audio.cw = 0;
//		break;
//		
//		case 'D' :
//			snprintf(param, 32, "Audio.Volume = %d%%\n\r", settings.audio.volume);
//			USB_write((uint8_t *)param, strlen(param));
//			snprintf(param, 32, "Audio.Frequency = %dHz\n\r", settings.audio.frequency);
//			USB_write((uint8_t *)param, strlen(param));
//			snprintf(param, 32, "Audio.CW = %d\n\r", settings.audio.cw);
//			USB_write((uint8_t *)param, strlen(param));
//		break;
//		
//	}
	
}


/**
   * @brief Read humidity from Si7013 
   * @param Arguments string from command
   * @param None
   * @retval None
   */
void cmd_humidity(char *argstr_buf)
{
//	char *argsep;
//	char param[32];
//	int x;

//	argsep = strchr (argstr_buf, ' ');

//	if (argsep != NULL) 
//	{
//		strcpy (param, argsep + 1);
//    *argsep = '\0';
//  }	else argstr_buf[0] = 'D';

//	switch 	(argstr_buf[0])
//	{
//		case 'N' : // Normal --- WPM
//			x = atoi(param);
//		if ((x>0) & (x<61)) settings.cw_message.dottime = 60000U/(50*x);
//		break;
//		
//		case 'S' : // Slow --- dot duration in seconds
//			x = atoi(param);
//		  if ((x>999) & (x<100000)) settings.cw_message.dottime = x;
//		  //  settings............
//		break;
//				
//		case 'D' :
//			snprintf(param, 32, "CWmessage.DotTime = %dms\n\r", settings.cw_message.dottime);
//			USB_write((uint8_t *)param, strlen(param));
//		  snprintf(param, 32, "CWmessage.Message:");
//			USB_write((uint8_t *)param, strlen(param));
//			USB_write((uint8_t *)settings.cw_message.message, strlen(settings.cw_message.message));
//		  snprintf(param, 32, "\n\r");
//			USB_write((uint8_t *)param, strlen(param));
//		break;
//		
//	}
	
}


/**
   * @brief Read thermistor voltage from Si7013 
   * @param Arguments string from command
   * @param None
   * @retval None
   */
void cmd_thermistor(char *argstr_buf)
{
//	char *argsep;
//	char param[32];
//	int x;

//	argsep = strchr (argstr_buf, ' ');

//	if (argsep != NULL) 
//	{
//		strcpy (param, argsep + 1);
//    *argsep = '\0';
//  }	else argstr_buf[0] = 'D';

//	switch 	(argstr_buf[0])
//	{
//		case 'N' : // Normal --- WPM
//			x = atoi(param);
//		if ((x>0) & (x<61)) settings.cw_message.dottime = 60000U/(50*x);
//		break;
//		
//		case 'S' : // Slow --- dot duration in seconds
//			x = atoi(param);
//		  if ((x>999) & (x<100000)) settings.cw_message.dottime = x;
//		  //  settings............
//		break;
//				
//		case 'D' :
//			snprintf(param, 32, "CWmessage.DotTime = %dms\n\r", settings.cw_message.dottime);
//			USB_write((uint8_t *)param, strlen(param));
//		  snprintf(param, 32, "CWmessage.Message:");
//			USB_write((uint8_t *)param, strlen(param));
//			USB_write((uint8_t *)settings.cw_message.message, strlen(settings.cw_message.message));
//		  snprintf(param, 32, "\n\r");
//			USB_write((uint8_t *)param, strlen(param));
//		break;
//		
//	}
	
}




/**
   * @brief Define settings ident 
   * @param Message string
   * @param None
   * @retval None
   */
void cmd_ident(char *argstr_buf)
{
//	char izp[32];
//	
//	if ((argstr_buf != NULL) & (strlen(argstr_buf)>0))
//	{		
//		strupr(argstr_buf);
//		strncpy(settings.Ident, argstr_buf, 8);
//  }
//	else
//	{
//		USB_write((uint8_t *)settings.Ident, 
//		           strlen(settings.Ident));	
//		snprintf(izp, 32, "\n\r"); 
//		USB_write((uint8_t *)izp, strlen(izp));
//		
//	}
}

///**
//   * @brief Store settinfgs to specified position 
//   * @param Message string
//   * @param del - 0 = normal write, 1 - delete 
//   * @retval None
//   */
//void cmd_store(char *argstr_buf, uint8_t del, uint8_t echo)
//{
//	char izp[32];
//	int x,i;
//	HAL_StatusTypeDef FLstatus;
//	FLASH_EraseInitTypeDef eraseinit;
//	uint32_t PageError;
//	uint32_t stadr;
//	uint64_t buf;
//	uint32_t fladr;
//	uint8_t *adr;
//	settings_t 	tmp_settings;
//	
//	if ((argstr_buf != NULL) & (strlen(argstr_buf)>0))
//	{		
//    x = atoi(argstr_buf);
//		if ((x>0) & (x<=SETTINGS_CATLEN))
//		{
//			x--; // x = 1...n 
//			FLstatus = HAL_FLASH_Unlock();  

//			// Erase backup sector
//			eraseinit.NbPages = 1;
//			eraseinit.PageAddress = FLASH_BACKUP_ADR;
//			eraseinit.TypeErase = TYPEERASE_PAGES;
//			FLstatus = HAL_FLASHEx_Erase(&eraseinit, &PageError);   
//			
//			// copy store to backup
//			for (i = 0; i< SETTINGS_CATLEN; i++)
//			{
//				adr = (uint8_t *)&tmp_settings;
//				memcpy(adr, &settings_Store[i], sizeof(settings_t));
//				fladr = FLASH_BACKUP_ADR+sizeof(settings_t)*(i);
//				for (stadr = 0; stadr<(sizeof(settings_t)/8); stadr++)
//				{
//					memcpy(&buf, adr, 8); 
//					FLstatus =  HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, fladr, buf);
//					adr+=8;
//					fladr+=8;
//				}

//		  }
//			if (echo)
//			{
//			  snprintf(izp, 32, "Backup... status %d\n\r", FLstatus); 
//		    USB_write((uint8_t *)izp, strlen(izp));
//			}
//			// erase store 
//			eraseinit.NbPages = 1;
//			eraseinit.PageAddress = FLASH_STORE_ADR;
//			eraseinit.TypeErase = TYPEERASE_PAGES;
//			FLstatus = HAL_FLASHEx_Erase(&eraseinit, &PageError);   
//			
//			// and write to it from new data and backup

//			for (i = 0; i< SETTINGS_CATLEN; i++)
//			{
//        if (i == x) // actual location is written with new data
//				{
//					if (del == 1) // delete
//					{
//						// do nothing, keep erased flash
//					}
//					else
//					{							
//						adr = (uint8_t *)&settings;
//						fladr = FLASH_STORE_ADR+sizeof(settings_t)*x;
//						for (stadr = 0; stadr<(sizeof(settings_t)/8); stadr++)
//						{
//							memcpy(&buf, adr, 8); 
//							FLstatus =  HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, fladr, buf);
//							adr+=8;
//							fladr+=8;
//						}
//					}
//				}  // if i==x
//				else
//				{
//					adr = (uint8_t *)&tmp_settings;
//					memcpy(adr, &backup_settings_Store[i], sizeof(settings_t));
//					fladr = FLASH_STORE_ADR+sizeof(settings_t)*(i);
//					for (stadr = 0; stadr<(sizeof(settings_t)/8); stadr++)
//					{
//						memcpy(&buf, adr, 8); 
//						FLstatus =  HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, fladr, buf);
//						adr+=8;
//						fladr+=8;
//					}
//					
//				}
//			}  /*  for ... */
//			if (echo)
//			{
//				snprintf(izp, 32, "Writing... status %d\n\r", FLstatus); 
//				USB_write((uint8_t *)izp, strlen(izp));
//			}


//			FLstatus = HAL_FLASH_Lock();  
//		}
//  }
//}



///**
//   * @brief Load settinfgs from specified position 
//   * @param Message string
//   * @param del - 0 = normal write, 1 - delete 
//   * @retval None
//   */
//void cmd_load(char *argstr_buf)
//{
//	int x;
//	uint8_t *adr;
//	char izp[32];
//	
//	if ((argstr_buf != NULL) & (strlen(argstr_buf)>0))
//	{		
//    x = atoi(argstr_buf);
//		if ((x>0) & (x<=SETTINGS_CATLEN))
//		{
//			x--; // x = 1...n 
//			adr = (uint8_t *)&settings; 
//			memcpy(adr, &settings_Store[x], sizeof(settings_t));
//			snprintf(izp, 32, "Loaded %d: ", x+1); 
//		  USB_write((uint8_t *)izp, strlen(izp));			
//			DisplaySettings(&settings);
//			snprintf(izp, 32, "\n\r"); 
//			USB_write((uint8_t *)izp, strlen(izp));		
//    }
//  }
//}





///**
//   * @brief Catalog stored settings
//   * @param None
//   * @param None
//   * @retval None
//   */
//void cmd_cat(void)
//{
//	int i;
//	char izp[32];
//	uint8_t firstchar;
//	
//	for (i=0; i<SETTINGS_CATLEN; i++)
//	{
//		// Print number
//		snprintf(izp, 32, "%x: ",i+1);  // avoid 0
//		USB_write((uint8_t *)izp, strlen(izp));
//		// Print out settings ID
//		firstchar = settings_Store[i].Ident[0];
//		if ((strlen(settings_Store[i].Ident) > 0) & (firstchar!=0xff))
//		{
//			DisplaySettings((settings_t *)&settings_Store[i]);
//		}
//		else
//		{
//			snprintf(izp, 32, " <Empty> ");
//			USB_write((uint8_t *)izp, strlen(izp));
//		}

//		snprintf(izp, 32, "\n\r"); 
//		USB_write((uint8_t *)izp, strlen(izp));
//		
//	}
//	
//}


/**
   * @brief Set/display device address
   * @param  
   * @param None
   * @retval None
   */
void cmd_address(char *argstr_buf)
{
//	int x;

//	if ((argstr_buf != NULL) & (strlen(argstr_buf)>0))
//	{		
//    x = atoi(argstr_buf);
//		if ((x>=0) & (x<=2))
//		{
//			settings.autorun = x;
//			
////			x--; // x = 1...n 
////			memcpy(&tmpsettings, &settings, sizeof(settings_t));  // temp. storage
////			for (n=0; n<SETTINGS_CATLEN; n++)
////			{
////				snprintf(izp, 32, "%d", n+1); 
////				cmd_load(izp);
////				if (x<100)
////				{
////				  settings.autorun = (n==x) ? 1 : 0;
////				}
////				else
////				{
////				  settings.autorun = ((n+100)==x) ? 100 : settings.autorun;					
////				}
////				cmd_store(izp, 0, 0);		// normal write
////			} // for
////			memcpy(&settings, &tmpsettings, sizeof(settings_t));  // temp. storage
//    }
//  }
}



void cmd_unknown(char *argstr_buf)
{
	uart_puts((char *)helptext);
}

