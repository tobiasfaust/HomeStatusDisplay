from myUtils import *
import argparse

parser = argparse.ArgumentParser()

# erwartete Parameter
parser.add_argument('-m', '--ManifestDir', type=str, help='The Path where all manifest:_all.json are located', default='manifests')
parser.add_argument('-v', '--VersionDir', type=str, help='Verzeichnis, wo releases.json geschrieben werden soll', default='web-installer')

# Parsen der Argumente
args = parser.parse_args()

if args.ManifestDir:
    # lade die Versionen erneut
    extracted_data = search_manifests_and_extract_version(args.ManifestDir, True)

    # Wenn Daten extrahiert wurden, speichere sie in der JSON-Datei 'versions.json'
    if extracted_data:
        save_results_to_json(extracted_data, os.path.join(args.VersionDir, 'firmware/releases.json'))
    else:
        print("Keine relevanten 'manifest_all.json' Dateien gefunden.")
    