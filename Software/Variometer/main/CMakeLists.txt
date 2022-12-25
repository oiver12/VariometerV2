# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )
set(COMPONENT_SRCS  "TestTools/FlightLogger.c" "TestTools/Simulation.c" "IGCLogger.c" "SoundManager.c" "Sensor/QMC588L.c" "BatteryManager.c" 
                    "SDCard.c" "nmea_parser.c" "QuaternionFilter.c" "Sensor/BMI088.c" "main.c" "Sensor/DPS310.c" "Kalman/cholesky.c" 
                    "Kalman/kalman.c" "Kalman/matrix.c" "Screen/FIFO.c" "Screen/Vario_Screen.c" "Screen/Widget_Vario.c"
                    "Screen/assets/arrow.c" "Screen/assets/Doppelpfeil_Vario.c" "Screen/assets/gps_an.c" "Screen/assets/gps_aus.c"
                    "Screen/assets/Kompass.c" "Screen/assets/Kreis_4px.c" "Screen/assets/lautsprecher_an.c" "Screen/assets/lautsprecher_stumm.c"
                    "Screen/assets/sd_card_error.c" "Screen/assets/sd_card_good.c" "Screen/Display_Vario.c" "DeviceContext.c" "Screen/E_Ink/EPD_3in7.c"
                    )
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
