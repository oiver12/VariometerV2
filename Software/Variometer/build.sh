export IDF_PATH="$HOME/esp/esp-idf"
. ~/esp/esp-idf/export.sh
idf.py build
chmod +x openOCD.sh
gnome-terminal -- ./openOCD.sh
exit