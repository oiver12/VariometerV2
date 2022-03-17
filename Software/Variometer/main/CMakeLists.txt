# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )
set(COMPONENT_SRCS "nmea_parser.c" "QuaternionFilter.c" "BMI088.c" "main.c" "MS5611.c" "Kalman/cholesky.c" "Kalman/kalman.c" "Kalman/matrix.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
