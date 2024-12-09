from myUtils import *
"""
This script creates the manifest_all.json as release asset.

Arguments:
    -r, --ReleaseURL (str): The URL of the release. Its an mandatory argument.
    -f, --ReleaseDir (str): Release Verzeichnis für die Firmwares. Default: 'releases'
Usage:
    python createReleaseAssetManifest.py -a <ReleaseURL> -f <ReleaseDir>
"""
import argparse, os

parser = argparse.ArgumentParser()

# erwartete Parameter
parser.add_argument('-r', '--ReleaseURL', type=str, help='The URL of the release. Its an mandatory argument', required=True)
parser.add_argument('-f', '--ReleaseDir', type=str, help='Release Verzeichnis für die Firmwares', default='releases')
parser.add_argument('-t', '--ReleaseTagName', type=str, help='The tag name of the release. Its an mandatory argument', required=True)

# Parsen der Argumente
args = parser.parse_args()

if args.ReleaseURL:
    # ändere in allen manifest.json-Dateien unterhalb args.ReleaseDir den URL-Pfad in allen 'path' variablen 
    # im array 'parts' zur Release-URL
    changeURL(args.ReleaseDir, args.ReleaseURL, args.ReleaseTagName)

    # erstelle das manifestAll.json
    process_manifests(args.ReleaseDir)

