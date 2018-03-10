/**
 * @file app_config.c
 * @author OpenFeeder Team <https://github.com/orgs/OpenFeeder/people>
 * @version 1.0
 * @date 
 */

#include "app.h"
#include "app_config.h"


bool config_set( void )
{

    INI_READ_STATE read_ini_status;
    char buf[50];

    /* Search for the CONFIG.INI file. */
    if ( FILEIO_RESULT_FAILURE == config_find_ini( ) )
    {
        strcpy( appError.message, "CONFIG.INI not found" );
        appError.currentLineNumber = __LINE__;
        sprintf( appError.currentFileName, "%s", __FILE__ );
        appError.number = ERROR_INI_FILE_NOT_FOUND;
        return false;
    }
    
    if ( true == appDataLog.log_events )
    {
        store_event(OF_FIND_INI);
    }

    /* Read the CONFIG.INI file. */
    read_ini_status = config_read_ini( );

    if ( INI_READ_OK != read_ini_status )
    {
        
        getIniPbChar( read_ini_status, buf, sizearray( buf ));

        sprintf( appError.message, "Wrong parameters in CONFIG.INI: %s (%d)", buf, read_ini_status );
        appError.currentLineNumber = __LINE__;
        sprintf( appError.currentFileName, "%s", __FILE__ );
        appError.number = ERROR_INI_FILE_READ;
        return false;
    }

    if ( true == appDataLog.log_events )
    {
        store_event(OF_READ_INI);
    }
    
//    if ( appDataPitTag.numPitTagDeniedOrColorA > 0 || appDataPitTag.numPitTagAcceptedOrColorB > 0 )
    if ( ( GO_NO_GO == appData.scenario_number && appDataAttractiveLeds.pattern_number > ALL_LEDS) || appData.scenario_number > GO_NO_GO )
    {
        if ( FILEIO_RESULT_FAILURE == read_PIT_tags( ) )
        {
            return false;
        }
        
        if ( true == appDataLog.log_events )
        {
            store_event(OF_READ_PIT_TAGS);
        }
    }

    return true;
}


FILEIO_RESULT read_PIT_tags( void )
{

    FILEIO_OBJECT file;
    FILEIO_ERROR_TYPE errF;
    uint16_t i, j, s;
    char buf[13];

#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
    printf( "Read PIT tags files\n" );
#endif 

    
    appDataPitTag.numPitTagStored = 0;
        
    if ( appData.scenario_number > GO_NO_GO )
    {
        if ( appDataPitTag.numPitTagDeniedOrColorA > 0 )
        {

            if ( FILEIO_RESULT_FAILURE == FILEIO_Open( &file, "PTDENIED.TXT", FILEIO_OPEN_READ ) )
            {
                errF = FILEIO_ErrorGet( 'A' );
#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                printf( "unable to open PIT tags denied file (%u)", errF );
#endif 
                sprintf( appError.message, "Unable to open PIT tags denied file (%u)", errF );
                appError.currentLineNumber = __LINE__;
                sprintf( appError.currentFileName, "%s", __FILE__ );
                FILEIO_ErrorClear( 'A' );
                appError.number = ERROR_PIT_TAGS_DENIED_FILE_OPEN;
                return FILEIO_RESULT_FAILURE;
            }

            for ( i = 0; i < appDataPitTag.numPitTagDeniedOrColorA; i++ )
            {
                FILEIO_Read( appDataPitTag.pit_tags_list[i], 1, 10, &file );
                appDataPitTag.pit_tags_list[i][11] = '\0';
                appDataPitTag.numPitTagStored += 1;
            }

            if ( FILEIO_RESULT_FAILURE == FILEIO_Close( &file ) )
            {
                errF = FILEIO_ErrorGet( 'A' );
#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                printf( "unable to close PIT tags denied file (%u)", errF );
#endif 
                sprintf( appError.message, "Unable to close PIT tags denied file (%u)", errF );
                appError.currentLineNumber = __LINE__;
                sprintf( appError.currentFileName, "%s", __FILE__ );
                FILEIO_ErrorClear( 'A' );
                appError.number = ERROR_PIT_TAGS_DENIED_FILE_CLOSE;
                return FILEIO_RESULT_FAILURE;
            }

        }

        if ( appDataPitTag.numPitTagAcceptedOrColorB > 0 )
        {

            if ( FILEIO_RESULT_FAILURE == FILEIO_Open( &file, "PTACCEPT.TXT", FILEIO_OPEN_READ ) )
            {
                errF = FILEIO_ErrorGet( 'A' );
#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                printf( "unable to open PIT tags accepted file (%u)", errF );
#endif 
                sprintf( appError.message, "Unable to open PIT tags accepted file (%u)", errF );
                appError.currentLineNumber = __LINE__;
                sprintf( appError.currentFileName, "%s", __FILE__ );
                FILEIO_ErrorClear( 'A' );
                appError.number = ERROR_PIT_TAGS_ACCEPTED_FILE_OPEN;
                return FILEIO_RESULT_FAILURE;
            }

            for ( i = 0; i < appDataPitTag.numPitTagAcceptedOrColorB; i++ )
            {
                FILEIO_Read( appDataPitTag.pit_tags_list[i + appDataPitTag.numPitTagDeniedOrColorA], 1, 10, &file );
                appDataPitTag.pit_tags_list[i + appDataPitTag.numPitTagDeniedOrColorA][11] = '\0';
                appDataPitTag.numPitTagStored += 1;
            }

            if ( FILEIO_RESULT_FAILURE == FILEIO_Close( &file ) )
            {
                errF = FILEIO_ErrorGet( 'A' );
#if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                printf( "unable to close PIT tags accepted file (%u)", errF );
#endif 
                sprintf( appError.message, "Unable to close PIT tags accepted file (%u)", errF );
                appError.currentLineNumber = __LINE__;
                sprintf( appError.currentFileName, "%s", __FILE__ );
                FILEIO_ErrorClear( 'A' );
                appError.number = ERROR_PIT_TAGS_ACCEPTED_FILE_CLOSE;
                return FILEIO_RESULT_FAILURE;
            }
        }
    }
    else if ( GO_NO_GO == appData.scenario_number )
    {
        if ( LEFT_RIGHT_LEDS == appDataAttractiveLeds.pattern_number )
        {
           if ( appDataPitTag.numPitTagDeniedOrColorA > 0 )
            {

                if ( FILEIO_RESULT_FAILURE == FILEIO_Open( &file, "PTLEFT.TXT", FILEIO_OPEN_READ ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to open PIT tags left file (%u)", errF );
    #endif 
                    sprintf( appError.message, "Unable to open PIT tags left file (%u)", errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_DENIED_FILE_OPEN;
                    return FILEIO_RESULT_FAILURE;
                }
                
                for ( i = 0; i < appDataPitTag.numPitTagDeniedOrColorA; i++ )
                {
                    FILEIO_Read( appDataPitTag.pit_tags_list[i], 1, 10, &file );
                    appDataPitTag.pit_tags_list[i][11] = '\0';
                    appDataPitTag.numPitTagStored += 1;
                }
                
                if ( FILEIO_RESULT_FAILURE == FILEIO_Close( &file ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to close PIT tags left file (%u)", errF );
    #endif 
                    sprintf( appError.message, "Unable to close PIT tags left file (%u)", errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_DENIED_FILE_CLOSE;
                    return FILEIO_RESULT_FAILURE;
                }

            }

            if ( appDataPitTag.numPitTagAcceptedOrColorB > 0 )
            {

                if ( FILEIO_RESULT_FAILURE == FILEIO_Open( &file, "PTRIGHT.TXT", FILEIO_OPEN_READ ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to open PIT tags right file (%u)", errF );
    #endif 
                    sprintf( appError.message, "Unable to open PIT tags right file (%u)", errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_ACCEPTED_FILE_OPEN;
                    return FILEIO_RESULT_FAILURE;
                }

                for ( i = 0; i < appDataPitTag.numPitTagAcceptedOrColorB; i++ )
                {
                    FILEIO_Read( appDataPitTag.pit_tags_list[i + appDataPitTag.numPitTagDeniedOrColorA], 1, 10, &file );
                    appDataPitTag.pit_tags_list[i + appDataPitTag.numPitTagDeniedOrColorA][11] = '\0';
                    appDataPitTag.numPitTagStored += 1;
                }

                if ( FILEIO_RESULT_FAILURE == FILEIO_Close( &file ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to close PIT tags right file (%u)", errF );
    #endif 
                    sprintf( appError.message, "Unable to close PIT tags right file (%u)", errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_ACCEPTED_FILE_CLOSE;
                    return FILEIO_RESULT_FAILURE;
                }
            }
        }
        else if ( TOP_BOTTOM_LEDS == appDataAttractiveLeds.pattern_number )
        {
           if ( appDataPitTag.numPitTagDeniedOrColorA > 0 )
            {

                if ( FILEIO_RESULT_FAILURE == FILEIO_Open( &file, "PTTOP.TXT", FILEIO_OPEN_READ ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to open PIT tags top file (%u)", errF );
    #endif 
                    sprintf( appError.message, "Unable to open PIT tags top file (%u)", errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_DENIED_FILE_OPEN;
                    return FILEIO_RESULT_FAILURE;
                }

                for ( i = 0; i < appDataPitTag.numPitTagDeniedOrColorA; i++ )
                {
                    FILEIO_Read( appDataPitTag.pit_tags_list[i], 1, 10, &file );
                    appDataPitTag.pit_tags_list[i][11] = '\0';
                    appDataPitTag.numPitTagStored += 1;
                }

                if ( FILEIO_RESULT_FAILURE == FILEIO_Close( &file ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to close PIT tags top file (%u)", errF );
    #endif 
                    sprintf( appError.message, "Unable to close PIT tags top file (%u)", errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_DENIED_FILE_CLOSE;
                    return FILEIO_RESULT_FAILURE;
                }

            }

            if ( appDataPitTag.numPitTagAcceptedOrColorB > 0 )
            {

                if ( FILEIO_RESULT_FAILURE == FILEIO_Open( &file, "PTBOTTOM.TXT", FILEIO_OPEN_READ ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to open PIT tags bottom file (%u)", errF );
    #endif 
                    sprintf( appError.message, "Unable to open PIT tags bottom file (%u)", errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_ACCEPTED_FILE_OPEN;
                    return FILEIO_RESULT_FAILURE;
                }

                for ( i = 0; i < appDataPitTag.numPitTagAcceptedOrColorB; i++ )
                {
                    FILEIO_Read( appDataPitTag.pit_tags_list[i + appDataPitTag.numPitTagDeniedOrColorA], 1, 10, &file );
                    appDataPitTag.pit_tags_list[i + appDataPitTag.numPitTagDeniedOrColorA][11] = '\0';
                    appDataPitTag.numPitTagStored += 1;
                }

                if ( FILEIO_RESULT_FAILURE == FILEIO_Close( &file ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to close PIT tags bottom file (%u)", errF );
    #endif 
                    sprintf( appError.message, "Unable to close PIT tags bottom file (%u)", errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_ACCEPTED_FILE_CLOSE;
                    return FILEIO_RESULT_FAILURE;
                }
            }
        }    
        else if ( ONE_LED == appDataAttractiveLeds.pattern_number )
        {
            memset(buf, 0, sizeof (buf));

            for ( j=0; j<4; j++ )
            {
//                printf("%d %d\n", j, appDataPitTag.numPitTagGroup[j]);
                    
                if (0 == appDataPitTag.numPitTagGroup[j])
                {
                    if ( 0 == j )
                    {
                        s = 0;
                        appDataAttractiveLeds.pattern_one_led_groups[j] = 0;
                    }
                    else
                    {
                        s += appDataPitTag.numPitTagGroup[j-1];
                        appDataAttractiveLeds.pattern_one_led_groups[j] = appDataAttractiveLeds.pattern_one_led_groups[j-1];
                    }
                    continue;
                }
                
                snprintf(buf, 13, "PTONE%d.TXT", j+1);
                
                if ( FILEIO_RESULT_FAILURE == FILEIO_Open( &file, buf, FILEIO_OPEN_READ ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to open PIT tags group %d file (%u)", j+1, errF );
    #endif 
                    sprintf( appError.message, "Unable to open PIT tags group %d file (%u)", j+1, errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_DENIED_FILE_OPEN;
                    return FILEIO_RESULT_FAILURE;
                }
                
                if ( 0 == j )
                {
                    s = 0;
                    appDataAttractiveLeds.pattern_one_led_groups[j] = appDataPitTag.numPitTagGroup[j];
                }
                else
                {
                    s += appDataPitTag.numPitTagGroup[j-1];
                    appDataAttractiveLeds.pattern_one_led_groups[j] = appDataAttractiveLeds.pattern_one_led_groups[j-1] + appDataPitTag.numPitTagGroup[j];
                }
                
//                printf("\t%d %d\n", s, appDataAttractiveLeds.pattern_one_led_groups[j]);
                
                for ( i = 0; i < appDataPitTag.numPitTagGroup[j]; i++ )
                {
                    FILEIO_Read( appDataPitTag.pit_tags_list[i+s], 1, 10, &file );
                    appDataPitTag.pit_tags_list[i+s][11] = '\0';
                    appDataPitTag.numPitTagStored += 1;
                }
                
                
                if ( FILEIO_RESULT_FAILURE == FILEIO_Close( &file ) )
                {
                    errF = FILEIO_ErrorGet( 'A' );
    #if defined (USE_UART1_SERIAL_INTERFACE) && defined (DISPLAY_LOG_INFO)
                    printf( "unable to close PIT tags group %d file (%u)", j+1, errF );
    #endif 
                    sprintf( appError.message, "Unable to close PIT tags group %d file (%u)", j+1, errF );
                    appError.currentLineNumber = __LINE__;
                    sprintf( appError.currentFileName, "%s", __FILE__ );
                    FILEIO_ErrorClear( 'A' );
                    appError.number = ERROR_PIT_TAGS_ACCEPTED_FILE_CLOSE;
                    return FILEIO_RESULT_FAILURE;
                }
                
            }
        }
    }

    return FILEIO_RESULT_SUCCESS;

}


FILEIO_RESULT config_find_ini( void )
{

    FILEIO_SEARCH_RECORD searchRecord;

    return FILEIO_Find( "CONFIG.INI", FILEIO_ATTRIBUTE_ARCHIVE, &searchRecord, true );
}


INI_READ_STATE config_read_ini( void )
{

    int32_t read_parameter;
    int s, i;
    char str[20];
    bool flag = false;
    
    /* Scenario number */
    read_parameter = ini_getl( "scenario", "num", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_SCENARIO_NUM;
    }
    else
    {
        appData.scenario_number = ( uint8_t ) read_parameter;
    }

    /* Site identification. */
    s = ini_gets( "siteid", "zone", "XXXX", appData.siteid, sizearray( appData.siteid ), "CONFIG.INI" );
    for ( i=s; i<4; i++ )
    {
        appData.siteid[i] = 'X';
    }

    /* Wake up time. */
    read_parameter = ini_getl( "time", "wakeup_hour", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_TIME_WAKEUP_HOUR;
    }
    else
    {
        appDataAlarmWakeup.time.tm_hour = ( int ) read_parameter;
    }
    read_parameter = ini_getl( "time", "wakeup_minute", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_TIME_WAKEUP_MINUTE;
    }
    else
    {
        appDataAlarmWakeup.time.tm_min = ( int ) read_parameter;
    }
    appDataAlarmWakeup.time.tm_sec = 0;

    /* Sleep time. */
    read_parameter = ini_getl( "time", "sleep_hour", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_TIME_SLEEP_HOUR;
    }
    else
    {
        appDataAlarmSleep.time.tm_hour = ( int ) read_parameter;
    }
    read_parameter = ini_getl( "time", "sleep_minute", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_TIME_SLEEP_MINUTE;
    }
    else
    {
        appDataAlarmSleep.time.tm_min = ( int ) read_parameter;
    }
    appDataAlarmSleep.time.tm_sec = 0;

    /* Attractive LEDs Color. */
    appData.flags.bit_value.attractive_leds_status = 0;
    /* Check if "attractiveleds" is present in the INI file */ 
    for (s = 0; ini_getsection(s, str, 20, "CONFIG.INI") > 0; s++)
    {
        if ( 0 == strcmp( str, "attractiveleds" ) )
        {
            appData.flags.bit_value.attractive_leds_status = true;
        }
    }
        
    if (true == appData.flags.bit_value.attractive_leds_status)
    {
        read_parameter = ini_getl( "attractiveleds", "red_a", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_RED_A;
        }
        else
        {
            appDataAttractiveLeds.red[0] = ( uint8_t ) read_parameter;
        }
        read_parameter = ini_getl( "attractiveleds", "green_a", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_GREEN_A;
        }
        else
        {
            appDataAttractiveLeds.green[0] = ( uint8_t ) read_parameter;
        }
        read_parameter = ini_getl( "attractiveleds", "blue_a", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_BLUE_A;
        }
        else
        {
            appDataAttractiveLeds.blue[0] = ( uint8_t ) read_parameter;
        }
        read_parameter = ini_getl( "attractiveleds", "red_b", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_RED_B;
        }
        else
        {
            appDataAttractiveLeds.red[1] = ( uint8_t ) read_parameter;
        }
        read_parameter = ini_getl( "attractiveleds", "green_b", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_GREEN_B;
        }
        else
        {
            appDataAttractiveLeds.green[1] = ( uint8_t ) read_parameter;
        }
        read_parameter = ini_getl( "attractiveleds", "blue_b", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_BLUE_B;
        }
        else
        {
            appDataAttractiveLeds.blue[1] = ( uint8_t ) read_parameter;
        }
        /* Attractive LEDs alternate delay. */
        read_parameter = ini_getl( "attractiveleds", "alt_delay", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_ALT_DELAY;
        }
        else
        {
            appDataAttractiveLeds.alt_delay = ( uint8_t ) read_parameter;
        }
        /* Attractive LEDs wake up time. */
        read_parameter = ini_getl( "attractiveleds", "on_hour", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_ON_HOUR;
        }
        else
        {
            appDataAttractiveLeds.wake_up_time.tm_hour = ( int ) read_parameter;
        }
        read_parameter = ini_getl( "attractiveleds", "on_minute", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_ON_MINUTE;
        }
        else
        {
            appDataAttractiveLeds.wake_up_time.tm_min = ( int ) read_parameter;
        }

        appDataAttractiveLeds.wake_up_time.tm_sec = 0;

        /* Attractive LEDs sleep time. */
        read_parameter = ini_getl( "attractiveleds", "off_hour", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_ATTRACTIVE_LEDS_OFF_HOUR;
        }
        else
        {
            appDataAttractiveLeds.sleep_time.tm_hour = ( int ) read_parameter;
        }
        read_parameter = ini_getl( "attractiveleds", "off_minute", -1, "CONFIG.INI" );
        if ( read_parameter == -1 )
        {
            return INI_PB_ATTRACTIVE_LEDS_OFF_MINUTE;
        }
        else
        {
            appDataAttractiveLeds.sleep_time.tm_min = ( int ) read_parameter;
        }
        appDataAttractiveLeds.sleep_time.tm_sec = 0;
        
        if ( GO_NO_GO == appData.scenario_number )
        {
            read_parameter = ini_getl( "attractiveleds", "pattern", -1, "CONFIG.INI" );
            if ( read_parameter == -1 )
            {
                return INI_PB_ATTRACTIVE_LEDS_PATTERN;
            }
            else
            {
                appDataAttractiveLeds.pattern_number = ( uint8_t ) read_parameter;
            }
            
            if ( ALL_LEDS == appDataAttractiveLeds.pattern_number )
            {
                
                ini_gets( "attractiveleds", "pattern_percent", "1.0", str, sizearray( str ), "CONFIG.INI" );
                appDataAttractiveLeds.pattern_percent = atof(str);
    
            }

        }
    }

    if ( appData.scenario_number > GO_NO_GO )
    {
        /* PIT Tags denied or associated with color A. */
        read_parameter = ini_getl( "pittags", "num_denied", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_PIT_TAG_DENIED;
        }
        else
        {
            appDataPitTag.numPitTagDeniedOrColorA = ( uint16_t ) read_parameter;
        }
        /* PIT Tags accepted or associated with color B. */
        read_parameter = ini_getl( "pittags", "num_accepted", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_PIT_TAG_ACCEPTED;
        }
        else
        {
            appDataPitTag.numPitTagAcceptedOrColorB = ( uint16_t ) read_parameter;
        }

        for (i = 0; i < appDataPitTag.numPitTagDeniedOrColorA; i++)
        {
            appDataPitTag.isPitTagdeniedOrColorA[i] = true;
        }
    }
    else if ( GO_NO_GO == appData.scenario_number )
    {
        if ( LEFT_RIGHT_LEDS == appDataAttractiveLeds.pattern_number )
        {
           /* PIT Tags associated with left LEDs. */
            read_parameter = ini_getl( "pittags", "num_left", -1, "CONFIG.INI" );
            if ( -1 == read_parameter )
            {
                return INI_PB_PIT_TAG_LEFT;
            }
            else
            {
                appDataPitTag.numPitTagDeniedOrColorA = ( uint16_t ) read_parameter;
            }
            /* PIT Tags associated with left LEDs. */
            read_parameter = ini_getl( "pittags", "num_right", -1, "CONFIG.INI" );
            if ( -1 == read_parameter )
            {
                return INI_PB_PIT_TAG_RIGHT;
            }
            else
            {
                appDataPitTag.numPitTagAcceptedOrColorB = ( uint16_t ) read_parameter;
            }

            for (i = 0; i < appDataPitTag.numPitTagDeniedOrColorA; i++)
            {
                appDataPitTag.isPitTagdeniedOrColorA[i] = true;
            } 
        }
        else if ( TOP_BOTTOM_LEDS == appDataAttractiveLeds.pattern_number )
        {
           /* PIT Tags associated with top LEDs. */
            read_parameter = ini_getl( "pittags", "num_top", -1, "CONFIG.INI" );
            if ( -1 == read_parameter )
            {
                return INI_PB_PIT_TAG_TOP;
            }
            else
            {
                appDataPitTag.numPitTagDeniedOrColorA = ( uint16_t ) read_parameter;
            }
            /* PIT Tags associated with bottom LEDs. */
            read_parameter = ini_getl( "pittags", "num_bottom", -1, "CONFIG.INI" );
            if ( -1 == read_parameter )
            {
                return INI_PB_PIT_TAG_BOTTOM;
            }
            else
            {
                appDataPitTag.numPitTagAcceptedOrColorB = ( uint16_t ) read_parameter;
            }

            for (i = 0; i < appDataPitTag.numPitTagDeniedOrColorA; i++)
            {
                appDataPitTag.isPitTagdeniedOrColorA[i] = true;
            } 
        }    
        else if ( ONE_LED == appDataAttractiveLeds.pattern_number )
        {
            appDataPitTag.numPitTagGroup[0] = 0;
            appDataPitTag.numPitTagGroup[1] = 0;
            appDataPitTag.numPitTagGroup[2] = 0;
            appDataPitTag.numPitTagGroup[3] = 0;
                
            /* PIT Tags associated with group LEDs 1. */
            read_parameter = ini_getl( "pittags", "num_led_1", -1, "CONFIG.INI" );
            if ( -1 == read_parameter )
            {
                return INI_PB_PIT_TAG_LED_1;
            }
            else
            {
                appDataPitTag.numPitTagGroup[0] = ( uint16_t ) read_parameter;
            }
            /* PIT Tags associated with group LEDs 2. */
            read_parameter = ini_getl( "pittags", "num_led_2", -1, "CONFIG.INI" );
            if ( -1 == read_parameter )
            {
                return INI_PB_PIT_TAG_LED_2;
            }
            else
            {
                appDataPitTag.numPitTagGroup[1] = ( uint16_t ) read_parameter;
            }
            /* PIT Tags associated with group LEDs 3. */
            read_parameter = ini_getl( "pittags", "num_led_3", -1, "CONFIG.INI" );
            if ( -1 == read_parameter )
            {
                return INI_PB_PIT_TAG_LED_3;
            }
            else
            {
                appDataPitTag.numPitTagGroup[2] = ( uint16_t ) read_parameter;
            }
            /* PIT Tags associated with group LEDs 4. */
            read_parameter = ini_getl( "pittags", "num_led_4", -1, "CONFIG.INI" );
            if ( -1 == read_parameter )
            {
                return INI_PB_PIT_TAG_LED_4;
            }
            else
            {
                appDataPitTag.numPitTagGroup[3] = ( uint16_t ) read_parameter;
            }   
        }
    }
    /* Door/servomotor configuration. */
    read_parameter = ini_getl( "door", "close_position", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_CLOSE_POSITION;
    }
    else
    {
        if ( read_parameter >= SERVO_DEFAULT_MIN_POSITION )
        {
            appDataServo.ton_min = ( uint16_t ) read_parameter;
            appDataServo.ton_min_night = appDataServo.ton_min;
        }
        else
        {
            appDataServo.ton_min = SERVO_DEFAULT_MIN_POSITION;
            appDataServo.ton_min_night = appDataServo.ton_min;
        }
    }

    read_parameter = ini_getl( "door", "open_position", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_OPEN_POSITION;
    }
    else
    {
        if ( read_parameter <= SERVO_DEFAULT_MAX_POSITION )
        {
            appDataServo.ton_max = ( uint16_t ) read_parameter;
        }
        else
        {
            appDataServo.ton_max = SERVO_DEFAULT_MAX_POSITION;
        }
    }
    /* Closing speed */
    read_parameter = ini_getl( "door", "closing_speed", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_CLOSING_SPEED;
    }
    else
    {
        appDataServo.closing_speed = ( uint8_t ) read_parameter;
    }
    /* Opening speed */
    read_parameter = ini_getl( "door", "opening_speed", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_OPENING_SPEED;
    }
    else
    {
        appDataServo.opening_speed = ( uint8_t ) read_parameter;
    }
    /* Door open/closee delays. */
    read_parameter = ini_getl( "door", "open_delay", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_OPEN_DELAY;
    }
    else
    {
        appDataDoor.open_delay = ( uint16_t ) read_parameter * 1000;
    }
    read_parameter = ini_getl( "door", "close_delay", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_CLOSE_DELAY;
    }
    else
    {
        appDataDoor.close_delay = ( uint16_t ) read_parameter * 1000;
    }
    /* Door habituation */
    if (DOOR_HABITUATION == appData.scenario_number)
    {
        read_parameter = ini_getl( "door", "habituation", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_DOOR_HABITUATION;
        }
        else
        {
            appDataDoor.habituation_percent = ( uint8_t ) read_parameter;
//            appDataServo.ton_max = (appDataServo.ton_max-appDataServo.ton_min)/100*appDataDoor.habituation_percent+appDataServo.ton_min;
            appDataServo.ton_min = (appDataServo.ton_max-appDataServo.ton_min)/100*appDataDoor.habituation_percent+appDataServo.ton_min;
        }
    }
    /* Door remain open */
    read_parameter = ini_getl( "door", "remain_open", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_REMAIN_OPEN;
    }
    else
    {
        appDataDoor.remain_open = ( uint8_t ) read_parameter;
    }
    /* Door reward probability */
//    read_parameter = ini_getf( "door", "reward_probability", -1, "CONFIG.INI" );
    ini_gets( "door", "reward_probability", "1.0", str, sizearray( str ), "CONFIG.INI" );
    appDataDoor.reward_probability = atof(str);

    /* Door open time. */
    read_parameter = ini_getl( "door", "open_hour", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_OPEN_HOUR;
    }
    else
    {
        appDataDoor.open_time.tm_hour = ( int ) read_parameter;
    }
    read_parameter = ini_getl( "door", "open_minute", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_OPEN_MINUTE;
    }
    else
    {
        appDataDoor.open_time.tm_min = ( int ) read_parameter;
    }
    appDataDoor.open_time.tm_sec = 0;

    /* Door close time. */
    read_parameter = ini_getl( "door", "close_hour", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_CLOSE_HOUR;
    }
    else
    {
        appDataDoor.close_time.tm_hour = ( int ) read_parameter;
    }
    read_parameter = ini_getl( "door", "close_minute", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_DOOR_CLOSE_MINUTE;
    }
    else
    {
        appDataDoor.close_time.tm_min = ( int ) read_parameter;
    }
    appDataDoor.close_time.tm_sec = 0;

    /* Logs */
    /* Check if "reward" section is present in the INI file */ 
    flag = false;
    for (s = 0; ini_getsection(s, str, 20, "CONFIG.INI") > 0; s++)
    {
        if ( 0 == strcmp( str, "logs" ) )
        {
            flag = true;
        }
    }
    
    if (flag)
    {
        /* Data separator in the log file. */
        ini_gets( "logs", "separator", DEFAULT_LOG_SEPARATOR, appDataLog.separator, sizearray( appDataLog.separator ), "CONFIG.INI" );
//        read_parameter = ini_getl( "logs", "birds", -1, "CONFIG.INI" );
//        if ( -1 == read_parameter )
//        {
//            return INI_PB_LOGS_BIRDS;
//        }
//        else
//        {
//            appDataLog.log_data = ( bool ) read_parameter;
//        }
        read_parameter = ini_getl( "logs", "udid", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_LOGS_UDID;
        }
        else
        {
            appDataLog.log_udid = ( bool ) read_parameter;
        }
        read_parameter = ini_getl( "logs", "events", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_LOGS_EVENTS;
        }
        else
        {
            appDataLog.log_events = ( bool ) read_parameter;
        }
        read_parameter = ini_getl( "logs", "errors", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_LOGS_ERRORS;
        }
        else
        {
            appDataLog.log_errors = ( bool ) read_parameter;
        }
        read_parameter = ini_getl( "logs", "battery", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_LOGS_BATTERY;
        }
        else
        {
            appDataLog.log_battery = ( bool ) read_parameter;
        }
        read_parameter = ini_getl( "logs", "rfid", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_LOGS_RFID;
        }
        else
        {
            appDataLog.log_rfid = ( bool ) read_parameter;
        }
    }
    else
    {
        /* Data separator in the log file. */
        ini_gets( "logfile", "separator", DEFAULT_LOG_SEPARATOR, appDataLog.separator, sizearray( appDataLog.separator ), "CONFIG.INI" );
        appDataLog.log_birds = true;
        appDataLog.log_udid = true;
        appDataLog.log_events = true;
        appDataLog.log_errors = true;
        appDataLog.log_battery = true;
        appDataLog.log_rfid = true;
    }
    /* Reward. */
    /* Check if "reward" section is present in the INI file */ 
    flag = false;
    for (s = 0; ini_getsection(s, str, 20, "CONFIG.INI") > 0; s++)
    {
        if ( 0 == strcmp( str, "reward" ) )
        {
            flag = true;
        }
    }
    
    if (flag)
    {
       /* Reward enable */
        read_parameter = ini_getl( "reward", "enable", -1, "CONFIG.INI" );
        if ( -1 == read_parameter )
        {
            return INI_PB_REWARD_ENABLE;
        }
        else
        {
            appData.reward_enable = ( uint8_t ) read_parameter;
        } 
    }
    else
    {
        appData.reward_enable = 1;
    }
 
    /* Timeout before standby. */
    read_parameter = ini_getl( "timeouts", "sleep", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_TIMEOUTS_SLEEP;
    }
    else
    {
        appData.timeout_standby = ( uint16_t ) read_parameter * 1000;
    }
    /* Timeout before pir. */
    read_parameter = ini_getl( "timeouts", "pir", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_TIMEOUTS_PIR;
    }
    else
    {
        appData.timeout_pir = ( uint16_t ) read_parameter * 1000;
    }
    /* Timeout taking reward. */
    read_parameter = ini_getl( "timeouts", "reward", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_TIMEOUTS_REWARD;
    }
    else
    {
        appData.timeout_taking_reward = ( uint16_t ) read_parameter * 1000;
    }
    /* Timeout guillotine. */
//    ini_gets( "timeouts", "guillotine", "1.0", str, sizearray( str ), "CONFIG.INI" );
//    appData.timeout_guillotine = ( uint16_t ) (atof(str) * 1000);

      appData.timeout_guillotine = (appDataServo.ton_max - appDataServo.ton_min)/appDataServo.closing_speed*(PR3/1000)+500;
        
//    printf("%d %d %d\n", appData.timeout_guillotine, TMR3_Period16BitGet(), (appDataServo.ton_max - appDataServo.ton_min)/appDataServo.closing_speed*(PR3/1000)+500);
    
//    read_parameter = ini_getl( "timeouts", "guillotine", -1, "CONFIG.INI" );
//    if ( -1 == read_parameter )
//    {
//        return INI_PB_TIMEOUTS_GUILLOTINE;
//    }
//    else
//    {
//        appData.timeout_guillotine = ( uint16_t ) read_parameter * 1000;
//    }
    /* Punishment delay. */
    read_parameter = ini_getl( "punishment", "delay", -1, "CONFIG.INI" );
    if ( -1 == read_parameter )
    {
        return INI_PB_PUNISHMENT_DELAY;
    }
    else
    {
        appData.punishment_delay = ( uint16_t ) read_parameter * 1000;
    }

    return INI_READ_OK;
}


void config_print( void )
{
    int i, j;

    printf( "Configuration parameters\n" );

    printf( "\tScenario\n\t\tNumber: %u\n\t\tTitle:",
            getCompletScenarioNumber() );
    switch ( appData.scenario_number )
    {
        case NO_SCENARIO:
            printf( " no scenario\n" );
            break;
        case OPEN_BAR:
            printf( " open-bar\n" );
            break;
        case GO_NO_GO:
            if ( 0 == appDataAttractiveLeds.pattern_number )
            {
                printf( " go-no go, all LEDs (pattern 0)\n" );
            }
            else if ( 1 == appDataAttractiveLeds.pattern_number )
            {
                printf( " go-no go, left/right LEDs (pattern 1)\n" );
            }
            else if ( 2 == appDataAttractiveLeds.pattern_number )
            {
                printf( " go-no go, top/bottom LEDs (pattern 2)\n" );
            }
            else if ( 3 == appDataAttractiveLeds.pattern_number )
            {
                printf( " go-no go, one LED (pattern 3)\n" );
            }            
            break;
        case LONG_TERM_SPATIAL_MEMORY:
            printf( " long term spatial memory\n" );
            break;
        case WORKING_SPATIAL_MEMORY:
            printf( " working spatial memory\n" );
            break;
        case COLOR_ASSOCIATIVE_LEARNING:
            printf( " color associative learning\n" );
            break;
        case DOOR_HABITUATION:
            printf( " door habituation\n" );
            break;
    }

    printf( "\tSite ID\n\t\tZone: %s\n",
            appData.siteid );

    printf( "\tTime\n" );
    printf( "\t\tWake up: %02d:%02d\n",
            appDataAlarmWakeup.time.tm_hour,
            appDataAlarmWakeup.time.tm_min );
    printf( "\t\tSleep: %02d:%02d\n",
            appDataAlarmSleep.time.tm_hour,
            appDataAlarmSleep.time.tm_min );

    
    
    printf( "\tLoggers\n" );
    printf( "\t\tSeparator: %s\n", appDataLog.separator );
    if ( true == appDataLog.log_birds)
    {
        printf( "\t\tBirds: enable - %s\n", appDataLog.filename );   
    }
    else
    {
        printf( "\t\tBirds: disable\n");
    }
    if ( true == appDataLog.log_battery)
    {
        printf( "\t\tBattery: enable - %s\n", "BATTERY.CSV" );   
    }
    else
    {
        printf( "\t\tBattery: disable\n");
    }  
    if ( true == appDataLog.log_rfid)
    {
        printf( "\t\tRfid: enable - %s\n", "RFID.CSV" );   
    }
    else
    {
        printf( "\t\tRfid: disable\n");
    }
    if ( true == appDataLog.log_udid)
    {
        printf( "\t\tUdid: enable - %s\n", "UDID.CSV" );   
    }
    else
    {
        printf( "\t\tUdid: disable\n");
    }
    if ( true == appDataLog.log_errors)
    {
        printf( "\t\tErrors: enable - %s\n", "ERRORS.CSV" );   
    }
    else
    {
        printf( "\t\tErrors: disable\n");
    }
    if ( true == appDataLog.log_events)
    {
        if ( EVENT_FILE_BINARY == appDataEvent.file_type )
        {
            printf( "\t\tEvents: enable - %s\n", appDataEvent.binfilename );
        } 
        else if ( EVENT_FILE_TEXT == appDataEvent.file_type )
        {
            printf( "\t\tEvents: enable - %s\n", appDataEvent.filename );   
        }
        else
        {
           printf( "\t\tEvents: enable - %s - %s\n", appDataEvent.filename, appDataEvent.binfilename ); 
        }
    }
    else
    {
        printf( "\t\tEvents: disable\n");
    }
    if (true == appData.flags.bit_value.attractive_leds_status)
    {

        printf( "\tAttractive LEDs\n" );
        
        printf( "\t\tLEDs order: %u %u %u %u\n", 
               appDataAttractiveLeds.leds_order[0], appDataAttractiveLeds.leds_order[1], 
               appDataAttractiveLeds.leds_order[2], appDataAttractiveLeds.leds_order[3]);
    
        if (COLOR_ASSOCIATIVE_LEARNING == appData.scenario_number)
        {

            printf( "\t\tColor A: RGB(%d, %d, %d)\n",
                    appDataAttractiveLeds.red[0],
                    appDataAttractiveLeds.green[0],
                    appDataAttractiveLeds.blue[0] );
            printf( "\t\tColor B: RGB(%d, %d, %d)\n",
                    appDataAttractiveLeds.red[1],
                    appDataAttractiveLeds.green[1],
                    appDataAttractiveLeds.blue[1] );
            printf( "\t\tAlternate delay: %us\n", appDataAttractiveLeds.alt_delay );
        }
        else if (DOOR_HABITUATION == appData.scenario_number) 
        {
            printf( "\t\tColor: RGB(%d, %d, %d)\n",
                    appDataAttractiveLeds.red[0],
                    appDataAttractiveLeds.green[0],
                    appDataAttractiveLeds.blue[0] );
        }            
        else if (GO_NO_GO == appData.scenario_number)
        {
            printf( "\t\tColor: RGB(%d, %d, %d)\n",
                    appDataAttractiveLeds.red[0],
                    appDataAttractiveLeds.green[0],
                    appDataAttractiveLeds.blue[0] );   
            printf( "\t\tAlternate delay: %us\n", appDataAttractiveLeds.alt_delay );
        }
        
        printf( "\t\tOn time: %02d:%02d\n",
                appDataAttractiveLeds.wake_up_time.tm_hour,
                appDataAttractiveLeds.wake_up_time.tm_min );
        printf( "\t\tOff time: %02d:%02d\n",
                appDataAttractiveLeds.sleep_time.tm_hour,
                appDataAttractiveLeds.sleep_time.tm_min );
    }

    printf( "\tDoor\n" );
    printf( "\t\tOpen delay: %ds\n\t\tClose delay: %ds\n",
            appDataDoor.open_delay / 1000,
            appDataDoor.close_delay / 1000 );
    if ( 0 == appDataDoor.remain_open )
    {
        printf( "\t\tRemain open: no\n" );
    }
    else
    {
        printf( "\t\tRemain open: yes\n" );
    }
    printf( "\t\tOpen time: %02d:%02d\n",
            appDataDoor.open_time.tm_hour,
            appDataDoor.open_time.tm_min );
    printf( "\t\tClose time: %02d:%02d\n",
            appDataDoor.close_time.tm_hour,
            appDataDoor.close_time.tm_min );

    if (DOOR_HABITUATION == appData.scenario_number)
    {
        printf( "\t\tDoor habituation: %d%%\n", appDataDoor.habituation_percent );
    }
    
    printf( "\tServo\n\t\tPosition full closed: %d\n", appDataServo.ton_min_night );
    printf( "\t\tPosition full opened: %d\n", appDataServo.ton_max );
    
    if (DOOR_HABITUATION == appData.scenario_number)
    {
        printf( "\t\tPosition habituation closed: %d\n", appDataServo.ton_min );
    }
    printf( "\t\tFull closing time: %.3fs\n", ((float)(appDataServo.ton_max-appDataServo.ton_min_night))/((float)appDataServo.closing_speed)*0.02 );
    printf( "\t\tFull opening time: %.3fs\n", ((float)(appDataServo.ton_max-appDataServo.ton_min_night))/((float)appDataServo.opening_speed)*0.02 );
    printf( "\t\tClosing speed factor: %d\n", appDataServo.closing_speed );
    printf( "\t\tOpening speed factor: %d\n", appDataServo.opening_speed );
    
    printf( "\tReward\n" );
    if ( 0 == appData.reward_enable )
    {
        printf( "\t\tEnable: no\n" );
    }
    else
    {
        printf( "\t\tEnable: yes\n" );
    }
    printf( "\t\tTimeout: %us\n", appData.timeout_taking_reward / 1000 );
    
    printf( "\tTimeouts\n" );
    printf( "\t\tSleep: %us\n", appData.timeout_standby / 1000 );
    printf( "\t\tPIR: %us\n", appData.timeout_pir / 1000 );    
    printf( "\t\tGuillotine: %.3fs\n", (float)appData.timeout_guillotine * 0.001 ); 
    
    if (true == appData.flags.bit_value.attractive_leds_status)
    {
        printf( "\tPunishment\n" );
        printf( "\t\tDelay: %us\n", appData.punishment_delay / 1000 );
    }
            
    if ( appData.scenario_number > DOOR_HABITUATION )
    {
        
        if ( GO_NO_GO == appData.scenario_number && ONE_LED == appDataAttractiveLeds.pattern_number )
        {        
            
//            for (i=0;i<appDataPitTag.numPitTagStored;i++)
//            {
//              printf( "\t\tSN%d: %s\n", i + 1, appDataPitTag.pit_tags_list[i] );  
//            }
 
            for ( i=0; i<4; i++)
            {
                printf( "\tPIT tags associated with LED %d\n", i+1);
                if (0==i)
                {
                    for (j=0;j<appDataAttractiveLeds.pattern_one_led_groups[0];j++)
                    {
                        printf( "\t\tSN%02d: %s\n", j + 1, appDataPitTag.pit_tags_list[j] );
                    }
                }
                else
                {
                   for (j=appDataAttractiveLeds.pattern_one_led_groups[i-1];j<appDataAttractiveLeds.pattern_one_led_groups[i];j++)
                    {
                        printf( "\t\tSN%02d: %s\n", j + 1, appDataPitTag.pit_tags_list[j] );
                    } 
                }
            }
            
        }
        else
        {        
            if ( COLOR_ASSOCIATIVE_LEARNING == appData.scenario_number)
            {
                printf( "\tPIT tags associated with color A\n" );
            }
            else if ( GO_NO_GO == appData.scenario_number)
            {
                if ( LEFT_RIGHT_LEDS == appDataAttractiveLeds.pattern_number )
                {
                    printf( "\tPIT tags associated with left attractive LEDs\n" );
                }
                else if ( TOP_BOTTOM_LEDS == appDataAttractiveLeds.pattern_number )
                {
                    printf( "\tPIT tags associated with top attractive LEDs\n" );
                }
                else
                {
                    printf( "\tPIT tags associated with pattern 1/2\n" );
                }
            }
            else
            {
                printf( "\tPIT tags denied\n" );
            }

            if ( appDataPitTag.numPitTagDeniedOrColorA > 0 )
            {
                for ( i = 0; i < appDataPitTag.numPitTagDeniedOrColorA; ++i )
                {
                    printf( "\t\tSN%02d: %s\n", i + 1, appDataPitTag.pit_tags_list[i] );
                }
            }
            else
            {
                printf( "\t\tNone\n" );
            }

            if ( COLOR_ASSOCIATIVE_LEARNING == appData.scenario_number)
            {
                printf( "\tPIT Tags associated with color B\n" );
            }
            else if ( GO_NO_GO == appData.scenario_number)
            {
                if ( LEFT_RIGHT_LEDS == appDataAttractiveLeds.pattern_number )
                {
                    printf( "\tPIT tags associated with right attractive LEDs\n" );
                }
                else if ( TOP_BOTTOM_LEDS == appDataAttractiveLeds.pattern_number )
                {
                    printf( "\tPIT Tags associated with bottom attractive LEDs\n" );
                }
                else
                {
                    printf( "\tPIT tags associated with pattern 2/2\n" );
                }
            }
            else
            {
                printf( "\tPIT tags accepted\n" );
            }

            if ( appDataPitTag.numPitTagAcceptedOrColorB > 0 )
            {
                for ( i = appDataPitTag.numPitTagDeniedOrColorA; i < ( appDataPitTag.numPitTagDeniedOrColorA + appDataPitTag.numPitTagAcceptedOrColorB ); ++i )
                {
                    printf( "\t\tSN%02d: %s\n", i + 1 - appDataPitTag.numPitTagDeniedOrColorA, appDataPitTag.pit_tags_list[i] );
                }
            }
            else
            {
                printf( "\t\tNone\n" );
            }
        }
        
    }

    printf( "\n" );
}


void getIniPbChar( INI_READ_STATE state, char *buf, uint8_t n )
{

    switch ( state )
    {

        case INI_PB_SCENARIO_NUM:
            snprintf( buf, n, "Scenario: number" );
            break;
        case INI_PB_SITEID_ZONE:
            snprintf( buf, n, "Site ID: zone" );
            break;
        case INI_PB_TIME_WAKEUP_HOUR:
            snprintf( buf, n, "Wake-up: hour" );
            break;
        case INI_PB_TIME_WAKEUP_MINUTE:
            snprintf( buf, n, "Wake-up: minute" );
            break;
        case INI_PB_TIME_SLEEP_HOUR:
            snprintf( buf, n, "Sleep: hour" );
            break;
        case INI_PB_TIME_SLEEP_MINUTE:
            snprintf( buf, n, "Sleep: minute" );
            break;
        case INI_PB_LOGS_BIRDS:
            snprintf( buf, n, "Logs: birds" );
            break;
        case INI_PB_LOGS_UDID:
            snprintf( buf, n, "Logs: udid" );
            break;
        case INI_PB_LOGS_EVENTS:
            snprintf( buf, n, "Logs: events" );
            break;
        case INI_PB_LOGS_ERRORS:
            snprintf( buf, n, "Logs: errors" );
            break;
        case INI_PB_LOGS_BATTERY:
            snprintf( buf, n, "Logs: battery" );
            break;
        case INI_PB_LOGS_RFID:
            snprintf( buf, n, "Logs: rfid" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_RED_A:
            snprintf( buf, n, "Attractive LEDs: red A" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_GREEN_A:
            snprintf( buf, n, "Attractive LEDs: green A" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_BLUE_A:
            snprintf( buf, n, "Attractive LEDs: blue A" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_RED_B:
            snprintf( buf, n, "Attractive LEDs: red B" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_GREEN_B:
            snprintf( buf, n, "Attractive LEDs: green B" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_BLUE_B:
            snprintf( buf, n, "Attractive LEDs: blue B" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_ALT_DELAY:
            snprintf( buf, n, "Attractive LEDs: alternate delay" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_ON_HOUR:
            snprintf( buf, n, "Attractive LEDs: off minute" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_ON_MINUTE:
            snprintf( buf, n, "Attractive LEDs: on minute" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_OFF_HOUR:
            snprintf( buf, n, "Attractive LEDs: off hour" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_OFF_MINUTE:
            snprintf( buf, n, "Attractive LEDs: off minute" );
            break;
        case INI_PB_ATTRACTIVE_LEDS_PATTERN:
            snprintf( buf, n, "Attractive LEDs: pattern" );
            break;      
        case INI_PB_ATTRACTIVE_LEDS_PATTERN_4_NUM:
            snprintf( buf, n, "Attractive LEDs: pattern LED number" );
            break;
        case INI_PB_PIT_TAG_LEFT:
            snprintf( buf, n, "Attractive LEDs: pattern left LEDs" );
            break;
        case INI_PB_PIT_TAG_RIGHT:
            snprintf( buf, n, "Attractive LEDs: pattern right LEDs" );
            break;
        case INI_PB_PIT_TAG_TOP:
            snprintf( buf, n, "Attractive LEDs: pattern top LEDs" );
            break;
        case INI_PB_PIT_TAG_BOTTOM:
            snprintf( buf, n, "Attractive LEDs: pattern bottom LEDs" );
            break;
        case INI_PB_PIT_TAG_LED_1:
            snprintf( buf, n, "Attractive LEDs: pattern group LEDs 1" );
            break;   
        case INI_PB_PIT_TAG_LED_2:
            snprintf( buf, n, "Attractive LEDs: pattern group LEDs 2" );
            break; 
        case INI_PB_PIT_TAG_LED_3:
            snprintf( buf, n, "Attractive LEDs: pattern group LEDs 3" );
            break; 
        case INI_PB_PIT_TAG_LED_4:
            snprintf( buf, n, "Attractive LEDs: pattern group LEDs 4" );
            break;    
        case INI_PB_DOOR_CLOSE_POSITION:
            snprintf( buf, n, "Door: close position" );
            break;
        case INI_PB_DOOR_OPEN_POSITION:
            snprintf( buf, n, "Door: open position" );
            break;
        case INI_PB_DOOR_CLOSING_SPEED:
            snprintf( buf, n, "Door: closing speed" );
            break;
        case INI_PB_DOOR_OPENING_SPEED:
            snprintf( buf, n, "Door: opening speed" );
            break;
        case INI_PB_DOOR_OPEN_DELAY:
            snprintf( buf, n, "Door: open delay" );
            break;
        case INI_PB_DOOR_CLOSE_DELAY:
            snprintf( buf, n, "Door: close delay" );
            break;
        case INI_PB_DOOR_REMAIN_OPEN:
            snprintf( buf, n, "Door: remain open" );
            break;
        case INI_PB_DOOR_OPEN_HOUR:
            snprintf( buf, n, "Door: open hour" );
            break;
        case INI_PB_DOOR_OPEN_MINUTE:
            snprintf( buf, n, "Door: open minute" );
            break;
        case INI_PB_DOOR_CLOSE_HOUR:
            snprintf( buf, n, "Door: close hour" );
            break;
        case INI_PB_DOOR_CLOSE_MINUTE:
            snprintf( buf, n, "Door: close minute" );
            break;
        case INI_PB_TIMEOUTS_SLEEP:
            snprintf( buf, n, "Timeouts: sleep" );
            break;
        case INI_PB_REWARD_ENABLE:
            snprintf( buf, n, "Reward: enable" );
            break;
        case INI_PB_TIMEOUTS_PIR:
            snprintf( buf, n, "Timeouts: pir" );
            break;
        case INI_PB_TIMEOUTS_REWARD:
            snprintf( buf, n, "Timeouts: reward" );
            break;
        case INI_PB_TIMEOUTS_GUILLOTINE:
            snprintf( buf, n, "Timeouts: guillotine" );
            break;
        case INI_PB_PUNISHMENT_DELAY:
            snprintf( buf, n, "New bird: delay" );
            break;
        case INI_PB_DOOR_HABITUATION:
            snprintf( buf, n, "Door: habituation" );
            break;
        default:
            snprintf( buf, n, "Error not listed" );
            break;
    }


}

/*******************************************************************************
 End of File
 */
