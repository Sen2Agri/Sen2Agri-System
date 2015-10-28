INPUT_DIR="/mnt/Imagery_S2A/L2A/MACCS/L1/S2"
TILE="15SVD"
OUTPUT_DIR="/var/maccs/S2"
MODE="L2INIT"
CONF_DIR="UserConfiguration"
STYLESHEET="styleSheet.xsl"

echo "Launching $MODE on input..."

/opt/maccs/core/4.4/bin/maccs                         \
"--input"                   "$INPUT_DIR"              \
"--TileId"                  "$TILE"                   \
"--output"                  "$OUTPUT_DIR"             \
"--mode"                    "$MODE"                   \
"--loglevel"                "DEBUG"                   \
"--enableTest"              "false"                   \
"--CheckXMLFilesWithSchema" "false"                   \
"--conf"                    "$CONF_DIR"               \
                            > log_$MODE.txt 2>&1

#echo "Launching L2 Checktool on output..."

#/opt/maccs/core/4.4/bin/maccs                         \
#"--input"                   "$OUTPUT_DIR"             \
#"--output"                  "$OUTPUT_DIR/L2CHECKTOOL" \
#"--mode"                    "L2CHECKTOOL"             \
#"--loglevel"                "DEBUG"                   \
#"--enableTest"              "false"                   \
#                            > log_CHCKTL.txt 2>&1

echo "Execution Completed."

