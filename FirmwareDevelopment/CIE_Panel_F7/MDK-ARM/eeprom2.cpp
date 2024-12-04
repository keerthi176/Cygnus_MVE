
typedef struct
{
	char  adjStart ; 
	char  dayStart ;	
	char  monthStart; 
	char  hourStart;	
	char  dayEnd  	;	
	char  monthEnd ;	
	char  hourEnd 	;	
} ahggh;


typedef struct
{
	ahggh    DSTTimes[ 32 ];	
	char		PanelName[ 40];
	
	int 		TotalFiresCounter;
	int 		VersionNumber; 																	

	int 		CurrentSiteAddress; 				
	int 		PreviousSiteAddress; 				
	int 		NextSiteAddress; 	
	short 	TouchXmin; 							

	short 	TouchYmin; 							
	short 	TouchXmax; 							
	short 	TouchYmax; 							
	short 	GSMCheckPeriod; 
	char 		PanelNetworkAddress; 	
	char 		DSTEnable; 							
	char 		ActiveBrightness; 					
	char 		InactiveBrightness; 				
	char 		BatteryActiveBrightness; 		
	char 		BatteryInactiveBrightness; 	
} feek;
 


/* Globals
**************************************************************************/




 
static feek defaults =  		  {  .PanelName					 = "Bob",
												  .VersionNumber            =	 0   ,
                                      .PanelNetworkAddress      =	 0   ,
                                      .DSTEnable                =	 0   ,
                                      .ActiveBrightness         =	50   ,
                                      .InactiveBrightness       =	30   ,
                                      .BatteryActiveBrightness  =	 5   , 
                                      .BatteryInactiveBrightness=	 0   ,
                                      .CurrentSiteAddress       =	 0   ,
                                      .PreviousSiteAddress      =	-1   ,
                                      .NextSiteAddress          =	 0   ,
                                      .TouchXmin              	= 0x62  ,
                                      .TouchYmin              	= 0x99  ,
                                      .TouchXmax              	=0xFB9  ,
                                      .TouchYmax              	=0xEAB  ,
                                      .GSMCheckPeriod         	=   15  ,
                                      .TotalFiresCounter      	=    0   ,
                                      .DSTTimes = {{ +1, /* at */ 25,3, 01, /* to */ 28,10, 01 },
																	{ +1, /* at */ 31,3, 01, /* to */ 27,10, 01 },
																	{ +1, /* at */ 29,3, 01, /* to */ 25,10, 01 },
																	{ +1, /* at */ 28,3, 01, /* to */ 31,10, 01 },
																	{ +1, /* at */ 27,3, 01, /* to */ 30,10, 01 },
																	{ +1, /* at */ 26,3, 01, /* to */ 29,10, 01 },
																	{ +1, /* at */ 31,3, 01, /* to */ 27,10, 01 },
																	{ +1, /* at */ 30,3, 01, /* to */ 26,10, 01 },
																	{ +1, /* at */ 29,3, 01, /* to */ 25,10, 01 },
																	{ +1, /* at */ 28,3, 01, /* to */ 31,10, 01 },
																	{ +1, /* at */ 26,3, 01, /* to */ 29,10, 01 },
																	{ +1, /* at */ 25,3, 01, /* to */ 28,10, 01 },
																	{ +1, /* at */ 31,3, 01, /* to */ 27,10, 01 },
																	{ +1, /* at */ 30,3, 01, /* to */ 26,10, 01 },
																	{ +1, /* at */ 28,3, 01, /* to */ 31,10, 01 },
																	{ +1, /* at */ 27,3, 01, /* to */ 30,10, 01 }, 
																	{ +1, /* at */ 26,3, 01, /* to */ 29,10, 01 },	
																	{ +1, /* at */ 25,3, 01, /* to */ 28,10, 01 },	
																	{ +1, /* at */ 30,3, 01, /* to */ 26,10, 01 },	
																	{ +1, /* at */ 29,3, 01, /* to */ 25,10, 01 },	
																	{ +1, /* at */ 28,3, 01, /* to */ 31,10, 01 },	
																	{ +1, /* at */ 27,3, 01, /* to */ 30,10, 01 },	
																	{ +1, /* at */ 25,3, 01, /* to */ 28,10, 01 },	
																	{ +1, /* at */ 31,3, 01, /* to */ 27,10, 01 },	
																	{ +1, /* at */ 30,3, 01, /* to */ 26,10, 01 },	
																	{ +1, /* at */ 29,3, 01, /* to */ 25,10, 01 },	
																	{ +1, /* at */ 27,3, 01, /* to */ 30,10, 01 },	
																	{ +1, /* at */ 26,3, 01, /* to */ 29,10, 01 },	
																	{ +1, /* at */ 25,3, 01, /* to */ 28,10, 01 },	
																	{ +1, /* at */ 31,3, 01, /* to */ 27,10, 01 },	
																	{ +1, /* at */ 29,3, 01, /* to */ 25,10, 01 },	
																	{ +1, /* at */ 28,3, 01, /* to */ 31,10, 01 }}};

