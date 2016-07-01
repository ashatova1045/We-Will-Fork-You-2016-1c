#ifndef LOG_UMC_H_
#define LOG_UMC_H_
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>//Incluyo commons

t_log* logUMC;
t_log* logDump;

t_log* crearLog();
t_log* crearLogReemplazoClock(int proceso);
t_log* crearLogReemplazoClockM(int proceso);
t_log* crearLogDump();




#endif /* LOG_UMC_H_ */
