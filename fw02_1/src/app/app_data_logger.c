/**
 * @file app_data_logger.c
 * @author OpenFeeder Team <https://github.com/orgs/OpenFeeder/people>
 * @version 1.0
 * @date 08/09/2016
 */

#include "app.h"
#include "app_data_logger.h"

#include  "ctype.h"

// *****************************************************************************
// PopulateBuffer - Ajout de donnees dans le buffer
// *****************************************************************************


static int populateLogBuffer(void)
{
    int flag;
    char line[MAX_CHAR_PER_LINE];   
    unsigned long delayS;

    if (0 == strcmp(appDataLog.bird_pit_tag_str, "XXXXXXXXXX"))
    {
        appDataLog.is_reward_taken = false;
    }

    if (true == appDataLog.is_pit_tag_denied)
    {
        appDataLog.is_reward_taken = false;
    }

    /* Dur�e stationnement oiseau en secondes */
    if (true == appDataLog.is_reward_taken)
    {
        delayS = (appDataLog.bird_quit_time.tm_hour - appDataLog.bird_arrived_time.tm_hour)*60 * 60 +
            (appDataLog.bird_quit_time.tm_min - appDataLog.bird_arrived_time.tm_min)*60 +
            appDataLog.bird_quit_time.tm_sec - appDataLog.bird_arrived_time.tm_sec;
    }
    else
    {
        delayS = 0;
    }
    
    flag = sprintf(line, "%02d/%02d/%02d%s%02d:%02d:%02d%s"
                   /* siteid               */ "%c%c"
                   /* separator            */ "%s"
                   /* OF                   */ "OF%c%c"
                   /* separator            */ "%s"
                   /* scenario_number      */ "%u"
                   /* separator            */ "%s"
                   /* bird_pit_tag_str     */ "%s"
                   /* separator            */ "%s"
                   /* is_pit_tag_denied    */ "%u"
                   /* separator            */ "%s"
                   /* is_reward_taken      */ "%u"
                   /* separator            */ "%s"
                   /* attractive LED red   */ "%d"
                   /* separator            */ "%s"
                   /* attractive LED green */ "%d"
                   /* separator            */ "%s"
                   /* attractive LED blue           */ "%d"
                   /* separator                     */ "%s"
                   /* door_status_when_bird_arrived */ "%d"
                   /* separator                     */ "%s"
                   /* delayS                        */ "%lu\n",
                   appDataLog.bird_arrived_time.tm_mday,
                   appDataLog.bird_arrived_time.tm_mon,
                   appDataLog.bird_arrived_time.tm_year,
                   appDataLog.separator,
                   appDataLog.bird_arrived_time.tm_hour,
                   appDataLog.bird_arrived_time.tm_min,
                   appDataLog.bird_arrived_time.tm_sec,
                   appDataLog.separator,
                   appData.siteid[0],
                   appData.siteid[1],
                   appDataLog.separator,
                   appData.siteid[2],
                   appData.siteid[3],
                   appDataLog.separator,
                   getCompletScenarioNumber(),
                   appDataLog.separator,
                   appDataLog.bird_pit_tag_str,
                   appDataLog.separator,
                   appDataLog.is_pit_tag_denied,
                   appDataLog.separator,
                   appDataLog.is_reward_taken,
                   appDataLog.separator,
                   appDataAttractiveLeds.red[appDataLog.attractive_leds_current_color_index],
                   appDataLog.separator,
                   appDataAttractiveLeds.green[appDataLog.attractive_leds_current_color_index],
                   appDataLog.separator,
                   appDataAttractiveLeds.blue[appDataLog.attractive_leds_current_color_index],
                   appDataLog.separator,
                   appDataLog.door_status_when_bird_arrived,
                   appDataLog.separator,
                   delayS);

    if (flag > 0)
    {
        /* Concatenation de chaines de caracteres dans le buffer. */
        strcat(appDataLog.buffer, line);
    }

    return flag;
}

// *****************************************************************************
// LogWrite - Ecriture dans le fichier LOG.CSV
// *****************************************************************************


static int writeLogFile(void)
{
    FILEIO_OBJECT file;
    FILEIO_ERROR_TYPE errF;
    size_t numDataWritten;

    if (FILEIO_RESULT_FAILURE == FILEIO_Open(&file, appDataLog.filename, FILEIO_OPEN_WRITE | FILEIO_OPEN_CREATE | FILEIO_OPEN_APPEND))
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to open log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_LOG_FILE_OPEN;
        return FILEIO_RESULT_FAILURE;
    }

    numDataWritten = FILEIO_Write(appDataLog.buffer, 1, appDataLog.nCharBuffer, &file);

    if (numDataWritten < appDataLog.nCharBuffer)
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to write data in log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_LOG_FILE_WRITE;
        return FILEIO_RESULT_FAILURE;
    }

    if (FILEIO_RESULT_FAILURE == FILEIO_Close(&file))
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to close log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_LOG_FILE_CLOSE;
        return FILEIO_RESULT_FAILURE;
    }

    clearLogBuffer();

#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
    printf("\tWrite data to log file success\n");
#endif 

    return FILEIO_RESULT_SUCCESS;
}

bool dataLog(bool newData)
{
    unsigned int nChar = 0;

    /* Check if new data need to be added to the log buffer */
    if (true == newData)
    {

        nChar = populateLogBuffer();

        if (nChar < 0)
        {
            sprintf(appError.message, "Unable to populate log buffer");
            appError.currentLineNumber = __LINE__;
            sprintf(appError.currentFileName, "%s", __FILE__);
            appError.number = ERROR_POPULATE_DATA_BUFFER;
            return false;
        }
        else
        {
            appDataLog.nCharBuffer += nChar;
            appDataLog.numDataStored += 1;
#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO )
            printf("\tPopulate data to buffer (%u/%u)\n", appDataLog.numDataStored, MAX_NUM_DATA_TO_STORE);
#endif 
        }

    }

    /* If buffer is full then write log file on the USB device */
    if (appDataLog.numDataStored == MAX_NUM_DATA_TO_STORE)
    {
        setLedsStatusColor(LED_USB_ACCESS);

        if (USB_DRIVE_NOT_MOUNTED == usbMountDrive())
        {
            return false;
        }

        /* Ecriture fichier LOG. */
        if (FILEIO_RESULT_FAILURE == writeLogFile())
        {
            usbUnmountDrive();
            //            CMD_VDD_APP_V_USB_SetLow( );
            return false;
        }
        appDataLog.numDataStored = 0;
        usbUnmountDrive();
        //        CMD_VDD_APP_V_USB_SetLow( );
        setLedsStatusColor(LEDS_OFF);
    }

    return true;
}

void clearLogBuffer(void)
{
    /* Vidage du buffer. */
    memset(appDataLog.buffer, '\0', sizeof ( appDataLog.buffer));
    appDataLog.nCharBuffer = 0;

}

void clearRfidFreqBuffer(void)
{   
    appDataLog.numRfidFreqStored = 0;
}

void clearBatteryBuffer(void)
{
    appDataLog.numBatteryLevelStored = 0;
}

bool setLogFileName(void)
{

    /* Clear filename buffer */
    memset(appDataLog.filename, 0, sizeof (appDataLog.filename));

    /* Get current date */
    while (!RTCC_TimeGet(&appData.current_time))
    {
        Nop();
    }

    /* Set log file name => 20yymmdd.CSV */
    if (snprintf(appDataLog.filename, 13, "20%02d%02d%02d.CSV",
                 appData.current_time.tm_year,
                 appData.current_time.tm_mon,
                 appData.current_time.tm_mday) <= 0)

    {
#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO )
        printf("Unable to set log file name\n");
#endif 
        sprintf(appError.message, "Unable to set log file name");
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        appError.number = ERROR_LOG_FILE_SET_NAME;
        return false;
    }

    return true;
}


FILEIO_RESULT logBatteryLevel(void)
{
    FILEIO_OBJECT file;
    FILEIO_ERROR_TYPE errF;
    char buf[35];
    struct tm currentTime;
    int flag, i;
    size_t numDataWritten;

    getDateTime(&currentTime);

    if (USB_DRIVE_NOT_MOUNTED == usbMountDrive())
    {
        return FILEIO_RESULT_FAILURE;
    }

    if (FILEIO_RESULT_FAILURE == FILEIO_Open(&file, "BATTERY.CSV", FILEIO_OPEN_WRITE | FILEIO_OPEN_CREATE | FILEIO_OPEN_APPEND))
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to open battery log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_BATTERY_FILE_OPEN;
        return FILEIO_RESULT_FAILURE;
    }

    memset(buf, '\0', sizeof ( buf));

    for (i = 0; i < appDataLog.numBatteryLevelStored; i++)
    {
        flag = sprintf(buf, "%c%c,OF%c%c,%u,%02d/%02d/%02d,%02d:00,%2.3f\n",
                       appData.siteid[0],
                       appData.siteid[1],
                       appData.siteid[2],
                       appData.siteid[3],
                       getCompletScenarioNumber(),
                       currentTime.tm_mday,
                       currentTime.tm_mon,
                       currentTime.tm_year,
                       appDataLog.battery_level[i][0],
                       appDataLog.battery_level[i][1] * BATTERY_VOLTAGE_FACTOR);

        if (flag > 0)
        {
            numDataWritten = FILEIO_Write(buf, 1, flag, &file);

            if (numDataWritten < flag)
            {
                errF = FILEIO_ErrorGet('A');
                sprintf(appError.message, "Unable to write battery level in log file (%u)", errF);
                appError.currentLineNumber = __LINE__;
                sprintf(appError.currentFileName, "%s", __FILE__);
                FILEIO_ErrorClear('A');
                appError.number = ERROR_BATTERY_FILE_WRITE;
                return FILEIO_RESULT_FAILURE;
            }
        }
    }

    if (FILEIO_RESULT_FAILURE == FILEIO_Close(&file))
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to close battery log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_BATTERY_FILE_CLOSE;
        return FILEIO_RESULT_FAILURE;
    }

    if (USB_DRIVE_MOUNTED == usbUnmountDrive())
    {
        return FILEIO_RESULT_FAILURE;
    }

#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
    printf("\tWrite battery level to file success\n");
#endif 

    clearBatteryBuffer( );
        
    return FILEIO_RESULT_SUCCESS;
}

FILEIO_RESULT logUDID(void)
{
    FILEIO_OBJECT file;
    FILEIO_ERROR_TYPE errF;
    char buf[35];
    size_t numDataWritten;
    int flag;
    
    if (USB_DRIVE_NOT_MOUNTED == usbMountDrive())
    {
        return FILEIO_RESULT_FAILURE;
    }
    
    if (FILEIO_RESULT_FAILURE == FILEIO_Open(&file, "UDID.CSV", FILEIO_OPEN_WRITE | FILEIO_OPEN_CREATE | FILEIO_OPEN_APPEND))
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to open UDID log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_UDID_FILE_OPEN;
        return FILEIO_RESULT_FAILURE;
    }
    
    flag = sprintf(buf, "%06lX %06lX %06lX %06lX %06lX\n",
                        appData.udid.words[0],
                        appData.udid.words[1],
                        appData.udid.words[2],
                        appData.udid.words[3],
                        appData.udid.words[4]);

    if (flag > 0)
    {
        numDataWritten = FILEIO_Write(buf, 1, flag, &file);
        
        if (numDataWritten < flag)
        {
            errF = FILEIO_ErrorGet('A');
            sprintf(appError.message, "Unable to write UDID frequency in log file (%u)", errF);
            appError.currentLineNumber = __LINE__;
            sprintf(appError.currentFileName, "%s", __FILE__);
            FILEIO_ErrorClear('A');
            appError.number = ERROR_UDID_FILE_WRITE;
            return FILEIO_RESULT_FAILURE;
        }
    }
    
    if (FILEIO_RESULT_FAILURE == FILEIO_Close(&file))
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to close UDID log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_UDID_FILE_CLOSE;
        return FILEIO_RESULT_FAILURE;
    }
    
    if (USB_DRIVE_MOUNTED == usbUnmountDrive())
    {
        return FILEIO_RESULT_FAILURE;
    }
    
    return FILEIO_RESULT_SUCCESS;
    
}

FILEIO_RESULT logRfidFreq(void)
{
    FILEIO_OBJECT file;
    FILEIO_ERROR_TYPE errF;
    char buf[35];
    struct tm currentTime;
    int flag, i;
    size_t numDataWritten;

    getDateTime(&currentTime);

    if (USB_DRIVE_NOT_MOUNTED == usbMountDrive())
    {
        return FILEIO_RESULT_FAILURE;
    }

    if (FILEIO_RESULT_FAILURE == FILEIO_Open(&file, "RFIDFREQ.CSV", FILEIO_OPEN_WRITE | FILEIO_OPEN_CREATE | FILEIO_OPEN_APPEND))
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to open RFID log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_RFID_FILE_OPEN;
        return FILEIO_RESULT_FAILURE;
    }

    memset(buf, '\0', sizeof ( buf));
        
    for (i = 0; i < appDataLog.numRfidFreqStored; i++)
    {
        flag = sprintf(buf, "%c%c,OF%c%c,%u,%02d/%02d/%02d,%02d:%02d,%ld\n",
                       appData.siteid[0],
                       appData.siteid[1],
                       appData.siteid[2],
                       appData.siteid[3],
                       getCompletScenarioNumber(),
                       currentTime.tm_mday,
                       currentTime.tm_mon,
                       currentTime.tm_year,
                       appDataLog.rfid_freq[i][0],
                       appDataLog.rfid_freq[i][1],
                       (long) appDataLog.rfid_freq[i][2]*10);

        if (flag > 0)
        {
            numDataWritten = FILEIO_Write(buf, 1, flag, &file);

            if (numDataWritten < flag)
            {
                errF = FILEIO_ErrorGet('A');
                sprintf(appError.message, "Unable to write RFID frequency in log file (%u)", errF);
                appError.currentLineNumber = __LINE__;
                sprintf(appError.currentFileName, "%s", __FILE__);
                FILEIO_ErrorClear('A');
                appError.number = ERROR_RFID_FILE_WRITE;
                return FILEIO_RESULT_FAILURE;
            }
        }
    }

    if (FILEIO_RESULT_FAILURE == FILEIO_Close(&file))
    {
        errF = FILEIO_ErrorGet('A');
        sprintf(appError.message, "Unable to close RFID log file (%u)", errF);
        appError.currentLineNumber = __LINE__;
        sprintf(appError.currentFileName, "%s", __FILE__);
        FILEIO_ErrorClear('A');
        appError.number = ERROR_RFID_FILE_CLOSE;
        return FILEIO_RESULT_FAILURE;
    }

    if (USB_DRIVE_MOUNTED == usbUnmountDrive())
    {
        return FILEIO_RESULT_FAILURE;
    }

#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
    printf("\tWrite RFID frequency to file success\n");
#endif 

    clearRfidFreqBuffer( );
    
    return FILEIO_RESULT_SUCCESS;
}

// Placeholder function to get the timestamp for FILEIO operations

void GetTimestamp(FILEIO_TIMESTAMP * timestamp)
{
    /* help_mla_fileio.pdf 
     * 1.7.1.3.32 FILEIO_TimestampGet Type */

    while (!RTCC_TimeGet(&appData.current_time))
    {
        Nop();
    }

    timestamp->date.bitfield.day = appData.current_time.tm_mday;
    timestamp->date.bitfield.month = appData.current_time.tm_mon;
    timestamp->date.bitfield.year = appData.current_time.tm_year + 20; // 2000-1980 = 20
    timestamp->time.bitfield.hours = appData.current_time.tm_hour;
    timestamp->time.bitfield.secondsDiv2 = appData.current_time.tm_sec / 2;
    timestamp->time.bitfield.minutes = appData.current_time.tm_min;
    timestamp->timeMs = 0;
}


/*******************************************************************************
 End of File
 */
