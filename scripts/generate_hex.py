import os
import subprocess, logging

logging.basicConfig(level=logging.INFO)

def generate_gzip_hex(source_file, output_file):
    # Gzip the HTML file
    gzip_file = source_file + '.gz'

    if os.path.exists(gzip_file):
        os.remove(gzip_file)

    if os.name == 'nt':
        subprocess.run(['7z', 'a', '-tgzip', gzip_file, source_file], stdout=subprocess.DEVNULL)
    else:
        subprocess.run(['gzip', '-c', source_file], stdout=open(gzip_file, 'wb'))

    if (not os.path.exists(gzip_file)):
        logging.error('Failed writing into gzip file: %s', gzip_file)
        return

    # Convert the gzip file to hex
    with open(output_file, 'w') as f:
        subprocess.run(['xxd', '-nELEGANT_OTA_HTML', '-i', gzip_file], stdout=f)

    # Clean up the gzip file
    os.remove(gzip_file)

def main():
    source_file = 'include/elegant_ota.html'
    output_file = 'src/elegant_html.cpp'
    logging.info('Generating hex file: %s -> %s', source_file, output_file)
    
    generate_gzip_hex(source_file, output_file)

if __name__ == '__main__':
    main()