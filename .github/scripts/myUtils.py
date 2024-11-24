import os, shutil
import json
import logging
 
# Konfigurieren des Loggings 
logging.basicConfig(level=logging.INFO)

def read_json_file(file) -> dict:
    """
    Liest eine JSON-Datei und gibt die Daten zurück.

    <b>Parameter:</b>
        file_path (str): Der Pfad zur JSON-Datei.

    <b>Rückgabewert:</b>
        dict: Die gelesenen JSON-Daten.
    """
    if os.path.isfile(file):
        try:
            with open(file, 'r', encoding='utf-8') as file:
                return json.load(file)
        except json.JSONDecodeError:
            logging.error(f"Warnung: Kann die JSON-Datei nicht lesen: {file}")
        except Exception as e:
            logging.error(f"Fehler beim Verarbeiten von {file}: {e}")
        return None
    else:
        logging.error(f"Warnung: Datei {file} nicht gefunden.")
        return None



def renameDirs(root: str) -> None:
    """
    Iteriert über alle Verzeichnisse im angegebenen Zielverzeichnis 
    und benennt alle Verzeichnisse um, die mit '.zip' enden.
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
         
    <b>Rückgabewert:</b>
        keiner
    """
    for dirpath, dirnames, filenames in os.walk(root):
        for dirname in dirnames:
            # Überprüfen, ob das Verzeichnis mit '.zip' endet
            if dirname.endswith('.zip'):
                # Neuer Name ohne '.zip' Endung
                new_dirname = dirname[:-4]
                # Erstelle den vollständigen Pfad zum alten und neuen Verzeichnis
                old_dirpath = os.path.join(dirpath, dirname)
                new_dirpath = os.path.join(dirpath, new_dirname)
                # Benenne das Verzeichnis um
                os.rename(old_dirpath, new_dirpath)
                logging.info(f"Verzeichnis umbenannt: {old_dirpath} -> {new_dirpath}")


def process_manifests(root: str) -> None:
    """
    Funktion zum Suchen der 'manifest.json', Extrahieren der relevanten Daten und Erstellen der neuen JSON-Datei "manifest_all.json"
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
         
    <b>Rückgabewert:</b>
        keiner
    """
    
    headerIsWritten = False
    new_manifest_data = {}

    # Durchlaufe alle Unterverzeichnisse und Dateien im Verzeichnis
    for dirpath, dirnames, filenames in os.walk(root):
        # Prüfe, ob 'manifest.json' im aktuellen Verzeichnis existiert
        if 'manifest.json' in filenames:
            manifest_path = os.path.join(dirpath, 'manifest.json')

            try:
                # Öffne und lade die JSON-Daten aus der 'manifest.json' Datei
                manifest_data = read_json_file(manifest_path)
                if manifest_data is not None:
                    if headerIsWritten is False:
                        # Extrahiere die relevanten Informationen: 'name', 'chipFamily', 'version', 'stage' und 'parts'
                        name = manifest_data.get('name')
                        version = manifest_data.get('version')
                        stage = manifest_data.get('stage')
                        build = manifest_data.get('build')
                        
                        # Wenn die erforderlichen Felder vorhanden sind, erstelle die neue 'manifest_all.json' Datei
                        if name and version and stage:
                            headerIsWritten = True
                            # Das neue Dictionary für 'manifest_all.json'
                            new_manifest_data = {
                                "name": name,
                                "version": version,
                                "stage": stage,
                                "build": build, 
                                "builds": []  # Wir werden die "builds" später mit chipFamily und parts füllen
                            }

                    # Füge 'chipFamily' und 'parts' als Array zu 'builds' hinzu
                    chipFamily = manifest_data.get('chipFamily')
                    parts = manifest_data.get('parts', [])

                    if chipFamily:
                        new_manifest_data["builds"].append({
                            "chipFamily": chipFamily,
                            "parts": parts
                        })
                        
            except json.JSONDecodeError:
                logging.warning(f"Warnung: Fehler beim Parsen der JSON-Datei {manifest_path}.")
            except Exception as e:
                logging.error(f"Fehler beim Verarbeiten der Datei {manifest_path}: {e}")
    
    if manifest_path :
        # Der Pfad für die neue 'manifest_all.json' Datei
        parent_dir = os.path.dirname(os.path.dirname(manifest_path))
        new_manifest_path = os.path.join(parent_dir, 'manifest_all.json')
                            
        # Schreibe die neue JSON-Datei
        save_results_to_json(new_manifest_data, new_manifest_path)
                            
        logging.info(f"Manifest-Daten erfolgreich in {new_manifest_path} gespeichert.")
            

def search_manifests_and_extract_version(root: str, keepPath: bool) -> list :
    """
    Funktion zum Suchen und Extrahieren der relevanten Informationen aus manifest_all.json-Dateien
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
        keepPath (bool): ob der originale Pfad aus manifest_all.json behalten werden soll, anosnsten wird der lokale Pfad gesetzt

    <b>Rückgabewert:</b>
        Liste von JsonObjekten
    """
    results = []  # Liste, um die Ergebnisse zu speichern
    
    # Durchlaufe alle Unterverzeichnisse im angegebenen Verzeichnis
    for dirpath, dirnames, filenames in os.walk(root):
        # Prüfe, ob eine Datei existiert die mit 'manifest_all.json' aufhört
        for filename in filenames:

            manifest_path = os.path.join(dirpath, filename)
            if filename.endswith('manifest_all.json'):

                try:
                    # Öffne und lade die JSON-Daten aus der Datei
                    manifest_data = read_json_file(manifest_path)
                    if manifest_data is not None:
                        
                        # Extrahiere 'version' und 'stage' falls vorhanden
                        version = manifest_data.get('version', None)
                        stage = manifest_data.get('stage', None)
                        build = manifest_data.get('build', None)

                        # Extrahiere den ersten 'path' aus 'parts' falls vorhanden, 
                        # extrahiere daraus den Pfad
                        try:
                            path = manifest_data.get('builds', [])[0].get('parts', [])[0].get('path')
                            path = os.path.join(os.path.dirname(path), 'manifest_all.json')
                        except:
                            path = None
                        
                        if keepPath is False or path is None:
                           # entferne den root-folder "web-installer" aus dem Pfad
                           path =  os.sep.join(manifest_path.strip(os.sep).split(os.sep)[1:])

                        # Falls sowohl 'version', 'build' und 'stage' vorhanden sind, füge sie zum Ergebnis hinzu
                        if version is not None and stage is not None:
                            results.append({
                                'path': path ,
                                'version': version,
                                'stage': stage,
                                'build': int(build) if build else 0
                            })
                
                except json.JSONDecodeError:
                    logging.warning(f"Warnung: Kann die JSON-Datei nicht lesen: {manifest_path}")
                except Exception as e:
                    logging.error(f"Fehler beim Verarbeiten von {manifest_path}: {e}")
    
    return results


# Funktion zum Speichern der extrahierten Daten in einer neuen JSON-Datei
def save_results_to_json(results, output_file):
    """
    Speichert die extrahierten Daten in einer neuen JSON-Datei.

    <b>Parameter:</b>
        results (dict): Die zu speichernden Ergebnisse.
        output_file (str): Der Pfad zur Ausgabedatei.

    <b>Rückgabewert:</b>
        keiner
    """
    try:
        with open(output_file, 'w', encoding='utf-8') as outfile:
            json.dump(results, outfile, indent=4, ensure_ascii=False)
        logging.info(f"Ergebnisse wurden in {output_file} gespeichert.")
    except Exception as e:
        logging.error(f"Fehler beim Speichern der Ergebnisse: {e}")


def deleteVersions(root: str, keepVersions: int, versions: list = None) -> None:
    """
    lädt das json 'versions.json' in 'root' und durchsucht das array darin. Lädt die Attribute 'build' und 'path'. 
    Sortiert alle aufsteigend nach 'build' und entfernt daraus die ersten 'keepVersions' einträge.
    Die 'path' variable zeigt auf die manifest_all.json datei. Dessen gesamter Ordner wird nun für alle übrigen Einträge gelöscht.

    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
        keepVersions (int): die Anzahl der Build´s, die behalten werden sollen
        
    <b>Rückgabewert:</b>
        keiner
    """
    logging.info(f"Lösche alle Versionen, behalte die letzten {keepVersions} Versionen")
    
    if versions is None:
        # Lade die 'versions.json' Datei
        versions_file = os.path.join(root, 'versions.json')
        versions_data = read_json_file(versions_file)
        if versions_data is None:
            logging.error(f"Fehler beim Veraarbeiten der Datei {versions_file}")
            return
    else:
        versions_data = versions
    
    # Dictionary zum Speichern der 'build' Nummern und der Pfadangabe
    versions = {}
    
    # Durchlaufe alle Einträge in der 'versions.json' Datei
    for entry in versions_data:
        # Extrahiere 'build' und 'path' falls vorhanden
        build = entry.get('build', None)
        path = entry.get('path', None)
        stage = entry.get('stage', None)

        # TODO: stage wird aktuell nicht berücksichtigt

        # Wenn 'build' und 'path' vorhanden sind, füge sie zum Dictionary hinzu
        if build is not None and path is not None and stage is not None:
            path = os.path.dirname(path)
            if stage not in versions:
                versions[stage] = {}
            if build not in versions[stage]:
                versions[stage][build] = []
                versions[stage][build].append(path)
            if build not in versions[stage]:
                versions[stage][build] = []
                versions[stage][build].append(path)
    
    # Sortiere die 'build' Nummern aufsteigend pro stage
    for stage in versions:
        sorted_builds = sorted(versions[stage].keys())
        logging.info(f"Folgende Build Nummern wurden für Stage {stage} gefunden: {sorted_builds}")
        # Lösche alle 'build' Nummern, die nicht in den ersten 'keepVersions' enthalten sind
        for build in sorted_builds[:-keepVersions]:
            for path in versions[stage][build]:
                # Lösche den Ordner
                shutil.rmtree(f'web-installer/{path}')
                logging.info(f"{path} gelöscht")
    

# das manifest.json sieht folgendermassen aus:
#{
#    "name": "test (v2.5.1-development)",
#    "chipFamily": "ESP32-C3",
#    "stage": "development",
#    "build": 254,
#    "version": "v2.5.1",
#    "parts": [
#        {
#            "path": "https://tobiasfaust.github.io/test/firmware/v2.5.1-254-development/firmware_ESP32-C3/merged-firmware.ESP32-C3.v2.5.1-254.development.bin",
#            "offset": 0
#        }
#    ]
#}

def changeURL(root: str, url: str) -> None:
    """
    Ändert den URL-Pfad in allen 'manifest.json'-Dateien unterhalb des angegebenen Verzeichnisses 
    in allen 'path' Variablen im Array 'parts' zur angegebenen URL. Speichert die Änderungen in den Dateien.
         
    <b>Parameter:</b>
        root (string): das Root verzeichnis über welches iteriert werden soll
        url (string): die URL, die in den 'path' Variablen geändert werden soll
         
    <b>Rückgabewert:</b>
        keiner
    """
    for dirpath, dirnames, filenames in os.walk(root):
        if 'manifest.json' in filenames:
            manifest_path = os.path.join(dirpath, 'manifest.json')
            try:
                manifest_data = read_json_file(manifest_path)
                if manifest_data is not None:
                    parts = manifest_data.get('parts', [])
                    for part in parts:
                        if 'path' in part:
                            old_url = part['path']
                            new_url = os.path.join(url, os.path.basename(old_url))
                            part['path'] = new_url
                    save_results_to_json(manifest_data, manifest_path)
                    logging.info(f"URLs in {manifest_path} geändert.")
            except json.JSONDecodeError:
                logging.warning(f"Warnung: Fehler beim Parsen der JSON-Datei {manifest_path}.")
            except Exception as e:
                logging.error(f"Fehler beim Verarbeiten der Datei {manifest_path}: {e}")