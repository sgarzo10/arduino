#ifndef HOME_H
#define HOME_H

#include "Arduino.h"

class HOME
{
	public:
    String vULTIMA_PRESENZA="";
    int flag_PRESENZA=0;
		//-----
    String cmdLUCE(String v);
    String cmdPIR(String v);
    String cmdTV(String v);
    String cmdCiabatta(String n, String v);
    String decodificaNRF(String p);
};
#endif
