

#include "MM_Events.h"


int func( Event e )
{
	switch (e )
	{
		case EVENT_EEPROM_FULL_RESET : return 5;
			break;
		case EVENT_EEPROM_RESET : return 9;
		break;
	}
	return sizeof(Event) + (e == EVENT_EEPROM_FULL_RESET) -  (e == EVENT_EEPROM_RESET);
}
	
	