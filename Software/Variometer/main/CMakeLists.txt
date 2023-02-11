# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )
set(COMPONENT_SRCS  
                    "main.c" "IGCLogger.c" "SoundManager.c" "BatteryManager.c" "SDCard.c" "nmea_parser.c" "QuaternionFilter.c" "DeviceContext.c" "ButtonInput.c"
                    "Sensor/QMC588L.c" "Sensor/BMI088.c" "Sensor/DPS310.c"
                    "Screen/Display_Vario.c" "Screen/FIFO.c" "Screen/Vario_Screen.c" "Screen/Widget_Vario.c" "Screen/Settings_Widget_vario.c" "Screen/RenderSystem.c"
                    "Screen/E_Ink/EPD_3in7.c" "Screen/E_Ink/GUI_Paint.c"
                    "Screen/E_Ink/font8.c" "Screen/E_Ink/font12.c" "Screen/E_Ink/font16.c" "Screen/E_Ink/font20.c" "Screen/E_Ink/font24.c"
                    "Kalman/cholesky.c" "Kalman/kalman.c" "Kalman/matrix.c" 
                    "TestTools/FlightLogger.c" "TestTools/Simulation.c" 
                    )
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
