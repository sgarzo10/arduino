#include "HOME.h"

String HOME::cmdLUCE(String valore){
  String cmd="";
  if (valore == "on" || valore == "off")
    cmd="11";
  if (valore == "stato")
    cmd="10";
  return cmd;
}

String HOME::cmdPIR(String valore){
  String cmd="";
  if (valore == "on" || valore == "off")
    cmd="21";
  if (valore == "buzzOn" || valore == "buzzOff")
    cmd="22";
  if (valore == "stato")
    cmd="20";
  return cmd;
}

String HOME::cmdTV(String valore){
  String cmd="";
  if (valore == "on" || valore == "off")
    cmd="30";
  if (valore == "temp")
    cmd="31";
  if (valore == "hum")
    cmd="32";
  if (valore == "vol+")
    cmd="34";
  if (valore == "vol-")
    cmd="35";
  if (valore == "ch+")
    cmd="36";
  if (valore == "ch-")
    cmd="37";
  return cmd;
}

String HOME::cmdCiabatta(String numero, String valore){
  String cmd="";
  if (valore == "on" || valore == "off")
    cmd="4"+numero+"1";
  if (valore == "stato")
    cmd="4"+numero+"0";
  return cmd;
}

String HOME::decodificaNRF(String p){
  String res="Ricevuto da ";
  if (p.substring(0,1)=="1")
    res+="LUCE";
  if (p.substring(0,1)=="2")
    res+="PIR";
  if (p.substring(0,1)=="3")
    res+="TV";
  if (p.substring(0,1)=="4")
    res+="CIABATTA";
  if (p.substring(0,1)=="0")
    res+="MOD.S";    
  res+=" segnale ";
  /* NANO */
  if (p.substring(0,2)=="00")
    res+="di avvio";
  if (p.substring(0,2)=="01")
    res+="di successo scrittura";
  if (p.substring(0,2)=="02")
    res+="di fallimento scrittura";
  /* LUCE */
  if (p.substring(0,2)=="10")
    res+="di on";
  if (p.substring(0,2)=="11")
    res+="di off";
  if (p.substring(0,2)=="19")
    res+="di comando non riconosciuto";
  /* PIR */
  if (p.substring(0,2)=="22")
    flag_PRESENZA=1;
  if (p.substring(0,3)=="210")
    res+="di pir on buzz off";
  if (p.substring(0,3)=="211")
    res+="di pir on buzz on";
  if (p.substring(0,3)=="201")
    res+="di pir off buzz on";
  if (p.substring(0,3)=="200")
    res+="di pir off buzz off";
  if (p.substring(0,2)=="29")
    res+="di comando non riconosciuto";
  /* TV */
  if (p.substring(0,2)=="31")
    res+=" TEMP: "+p.substring(2,4); 
  if (p.substring(0,2)=="32")
    res+=" HUM: "+p.substring(2,4);
  if (p.substring(0,2)=="30")
    res+=" di comando TV";
  if (p.substring(0,2)=="39")
    res+=" di comando non riconosciuto";
  /*CIABATTA*/
  if (p.substring(0,1)=="4" && p.substring(2,3)=="0")
    res+="di luce "+p.substring(1,2)+" on";
  if (p.substring(0,1)=="4" && p.substring(2,3)=="1")
    res+="di luce "+p.substring(1,2)+" off";
  if (p.substring(0,2)=="49")
    res+=" di comando non riconosciuto";
  return res;
}

