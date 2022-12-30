# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )
set(COMPONENT_SRCS  "TestTools/FlightLogger.c" "TestTools/Simulation.c" "IGCLogger.c" "SoundManager.c" "Sensor/QMC588L.c" "BatteryManager.c" 
                    "SDCard.c" "nmea_parser.c" "QuaternionFilter.c" "Sensor/BMI088.c" "main.c" "Sensor/DPS310.c" "Kalman/cholesky.c" 
                    "Kalman/kalman.c" "Kalman/matrix.c" "Screen/FIFO.c" "Screen/Vario_Screen.c" "Screen/Widget_Vario.c"
                    "Screen/Display_Vario.c" "DeviceContext.c" "Screen/E_Ink/EPD_3in7.c" "Screen/E_Ink/GUI_Paint.c"
                    "Screen/E_Ink/font8.c" "Screen/E_Ink/font12.c" "Screen/E_Ink/font16.c" "Screen/E_Ink/font20.c" "Screen/E_Ink/font24.c"
                    )
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
