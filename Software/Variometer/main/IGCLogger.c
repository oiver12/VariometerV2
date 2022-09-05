#include "IGCLogger.h"
#include <string.h>
#include <esp_log.h>
#include <math.h>

#define IGC_HEADER_DATE			(2)
#define IGC_HEADER_PILOT		(4)
#define IGC_HEADER_CLASS		(6)
#define IGC_HEADER_GLIDER		(8)
#define IGC_HEADER_GPSDATUM		(10)

static const char *TAG = "IGCLogger";

const char *directory = "/Tracklogs";
const char *sdcard_Mount_Point = "/sdcard";

const char *nameManufacture = "MIN";
const char *serialNumber = "XXX";
const char *tailOfFileName = "01.igc";

igc_logger_t logger;

// Baumgartner Variometer
// HFDTExxxx
// HFPLTPILOT:xxxxxx
// HFCCLCOMPETITION CLASS:FAI-3 (PG)
// HFGTYGLIDERTYPE:xxxxx
// HFDTM100GPSDATUM:WGS-84
const char * igcHeader[] =
{
	"Baumgartner Variometer", 
	"\r\nHFDTE",					// ex) DDMMYY
	NULL,
	"\r\nHFPLTPILOT:",				// ex) akkdong
	NULL,
	"\r\nHFCCLCOMPETITION CLASS:",	// ex) FAI-3 (PG)
	NULL,
	"\r\nHFGTYGLIDERTYPE:", 		// ex) NIVIUK IcePeak6  {MANUFACTURE} {MODEL}
	NULL,
	"\r\nHFDTM100GPSDATUM:WGS-84",
	NULL,
	"\r\n"
};

static const char * makeFileName(char *buf, size_t buflength, sdcard_t *sdcard)
{
	// name format : YYYY-MM-DD-NRC-XXX-nn.igc
	// ...
	int pos = 0;
	int i;
	const char * ptr;
	
	// year
	snprintf(buf, buflength, "%04d-%02d-%02d-", logger.gps_data.date.year, logger.gps_data.date.month, logger.gps_data.date.day);
	pos += 11;

	// manufacture code
	for (i = 0, ptr = nameManufacture; i < 3; i++)
		buf[pos++] = (*ptr) ? *ptr++ : 'X';
	buf[pos++] = '-';
		
	// FR serial number
	ptr = serialNumber;
	for (i = 0; i < 3; i++)
		buf[pos++] = ptr[i];
	buf[pos++] = '-';
	
	// flight number of the day & file extension
	for (ptr = tailOfFileName; *ptr; ptr++)
		buf[pos++] = *ptr;
	buf[pos] = '\0'; // null-terminate
	
    char pathBuf[64];

	for (i = 2; i < 101; i++) // valid number : 01 ~ 99
	{
		strcpy(pathBuf, directory);
		strcat(pathBuf, "/");
		strcat(pathBuf, buf);

		if (!fileExists_sdcard(sdcard, pathBuf))
			return buf;
		
		buf[19] = (i / 10) + '0';
		buf[20] = (i % 10) + '0';
	}
	return NULL;
}

static void gps_event_handler_igcLogger(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	//TODO: GNZDA kann ignoriert werden
    gps_t *gps = NULL;
    switch (event_id) 
	{
		case GPS_UPDATE:
			gps = (gps_t *)event_data;
			logger.gps_data.altitude = gps->altitude;
			logger.gps_data.date = gps->date;
			logger.gps_data.latitude = gps->latitude;
			logger.gps_data.longitude = gps->longitude;
			logger.gps_data.time = gps->tim;
			logger.gps_data.fix3D = gps->fix_mode == GPS_MODE_3D;
			ESP_LOGI(TAG, "GPS Update: %lli ms", (int64_t)((esp_timer_get_time() - logger.gpsLastTimeUpdate)/1000));
			logger.gpsLastTimeUpdate = esp_timer_get_time();
			break;
		default:
			break;
    }
}

void updatePressureAltitude_IGCLogger(float height)
{
	logger.pressureAltitude = height;
}

static void logICGSentence(void *arg)
{
	fprintf(logger.file, "B");
	fprintf(logger.file, "%02d%02d%02d", logger.gps_data.time.hour, logger.gps_data.time.minute, logger.gps_data.time.second); //log Time HHMMSS
	int degree = floor(logger.gps_data.latitude); //DD
	int minutes = (logger.gps_data.latitude - degree) * 60 * 1000; //MMmmm
	fprintf(logger.file, "%02d%05d", abs(degree), abs(minutes));
	fprintf(logger.file, logger.gps_data.latitude > 0 ? "N" : "S");
	degree = floor(logger.gps_data.longitude); //DDD
	minutes = (logger.gps_data.longitude - degree) * 60 * 1000; //MMmmm
	fprintf(logger.file, "%03d%05d", abs(degree), abs(minutes));
	fprintf(logger.file, logger.gps_data.longitude > 0 ? "E" : "W");
	fprintf(logger.file, logger.gps_data.fix3D ? "A" : "V");
	fprintf(logger.file, "%05d", (int)logger.pressureAltitude); //PPPPP
	fprintf(logger.file, "%05d", (int)logger.gps_data.altitude); //GGGGG
	fprintf(logger.file, "\r\n");
}

esp_err_t start_IGCLogger(sdcard_t *card, nmea_parser_handle_t nmea_handler)
{
	nmea_parser_add_handler(nmea_handler, gps_event_handler_igcLogger, NULL);
	while(logger.gpsLastTimeUpdate == 0)
	{
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	ESP_LOGI(TAG, "IGC Logging started");
	char buffer[26 + strlen(directory)];
	init_sdcard(card, sdcard_Mount_Point);
	const char* FileName = makeFileName(buffer, 26 + strlen(directory), card);
	ESP_LOGI(TAG, "File Name: %s", FileName);
	FILE *f = openFile_sdcard(card, "/Tracklogs", FileName);
	logger.sdcard = card;
	logger.file = f;
	if(f)
	{
		for (int i = 0; i < sizeof(igcHeader) / sizeof(igcHeader[0]); i++)
		{
			switch (i)
			{
				case IGC_HEADER_DATE:
					fprintf(f, "%02d%02d%02d", logger.gps_data.date.day, logger.gps_data.date.month, logger.gps_data.date.year);				
					break;
				case IGC_HEADER_PILOT:
					break;
				case IGC_HEADER_CLASS:
					break;
				case IGC_HEADER_GLIDER:
					break;
				case IGC_HEADER_GPSDATUM:
					// leave it empty!!
					break;
				default:
					fprintf(f, igcHeader[i]);
					break;
			}
		}
		/* Start Timer, to log IGC every Second */
		const esp_timer_create_args_t periodic_timer_args = {
			.callback = &logICGSentence,
			.name = "periodicIGC"
		};
		esp_timer_handle_t periodic_timer;
		ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
		ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000000));
		return ESP_OK;
	}
	else
	{
		return ESP_ERR_INVALID_RESPONSE;
	}
}

esp_err_t stop_IGCLogger()
{
	fclose(logger.file);
	logger.file = NULL;
	return ESP_OK;
}


