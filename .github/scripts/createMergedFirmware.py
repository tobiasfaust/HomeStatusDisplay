from myUtils import *
import argparse
import os

parser = argparse.ArgumentParser()

# erwartete Parameter
parser.add_argument('-c', '--ChipFamily', type=str, help='chipFamily, ESP8266, ESP32-S3, ...', default='ESP32', required=True)
parser.add_argument('-b', '--BuildDir', type=str, help='Build Verzeichnis welches die fertig gebauten Firmwares enthält', required=True)
parser.add_argument('-p', '--PathOfPartitionsCSV', type=str, help='Pfad wo die partitions.csv liegt', default='partitions.csv', required=True)

# Parsen der Argumente
args = parser.parse_args()
result = None

if(not os.path.isfile(args.PathOfPartitionsCSV)):
    logging.error(f'File {args.PathOfPartitionsCSV} does not exist')
    exit(1)

def readOffsetFromPartitionCSV(path: str, name: str) -> int:
    """
    Reads a CSV file and calculates the offset for a given name.
    This function reads the specified CSV file with a delimiter of ','.
    It fills in the Offset column if it is empty by adding the previous offset
    and the value from the Size column of the previous row. It returns the calculated
    offset from the row where the value in the first column matches the given name as a hexadecimal number.
    If the file does not exist or cannot be read, it returns None.
    
    <b>Args:</b>
        path (str): The path to the CSV file.
        name (str): The name to search for in the first column.

    <b>Returns:</b>
        int: The calculated offset as a hexadecimal number, or None if the file cannot be read.
    """

    try:
        with open(path, 'r') as file:
            lines = file.readlines()
            headers = lines[0].strip().split(',')
            name_index = 0
            offset_index = 3
            size_index = 4

            previous_offset = 0
            previous_size = 0
            for line in lines[1:]:
                columns = line.strip().split(',')
                if columns[offset_index].strip() == '':
                    columns[offset_index] = str(previous_offset + previous_size)
                previous_size = int(columns[size_index], 0)
                previous_offset = int(columns[offset_index], 0)
                if columns[name_index] == name:
                    return hex(previous_offset)
    except (FileNotFoundError, IndexError, ValueError):
        return None

if args.BuildDir and os.path.isdir(args.BuildDir):
    if 'ESP32' in args.ChipFamily:
        result = f'esptool.py --chip {args.ChipFamily} merge_bin \
            --output {args.BuildDir}/merged-firmware.bin \
            --flash_mode dout \
            --flash_freq 80m \
            --flash_size 4MB \
            0x1000 {args.BuildDir}/bootloader.bin \
            0x8000 {args.BuildDir}/partitions.bin \
            {readOffsetFromPartitionCSV("partitions.csv", "app0")} {args.BuildDir}/firmware.bin'
        
        if os.path.isfile(f'{args.BuildDir}/littlefs.bin'):
            result += f' {readOffsetFromPartitionCSV("partitions.csv", "spiffs")} {args.BuildDir}/littlefs.bin'

    elif 'ESP8266' in args.ChipFamily and os.path.isfile(f'{args.BuildDir}/littlefs.bin'):
        result = f'esptool.py --chip {args.ChipFamily} merge_bin \
            --output {args.BuildDir}/merged-firmware.bin \
            --flash_mode dout \
            --flash_freq 40m \
            --flash_size 4MB \
            0x0000 {args.BuildDir}/firmware.bin \
            {readOffsetFromPartitionCSV("partitions.csv", "spiffs")} {args.BuildDir}/littlefs.bin'

print(f'command={result}')