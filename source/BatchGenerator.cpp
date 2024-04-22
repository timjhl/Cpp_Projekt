#include "../includes/BatchGenerator.h"

using namespace std;

BatchGenerator::BatchGenerator() : hideshell(false) 
{
    cout << "BatchGenerator created" << endl;
} //end of constructor

bool BatchGenerator::readJsonFile(const std::string& filename) 
{
    bool retFailure = false;
    std::ifstream file(filename);
    Json::Reader reader;
    Json::Value obj;
    size_t line = 0;

    reader.parse(file, obj);
    
    if (!std::filesystem::exists(filename)) 
    {
        std::cerr << "File does not exist: " << filename << std::endl;
        //Failure occured
        retFailure = true;
    }
    else
    {

        std::vector<std::string> expectedKeys = {"outputfile", "hideshell",  "entries" , "application"};
        std::vector<std::string> keys = obj.getMemberNames();

        //Für alle keys in der JSON-Datei
        for (const std::string& key : keys) 
        {
            //Wenn der key nicht in expectedKeys enthalten ist
            if (std::find(expectedKeys.begin(), expectedKeys.end(), key) == expectedKeys.end()) 
            {//Failure occured
                
                //bool keyLineCount(const std::string& filename, const std::string& key, size_t& lineCount);
                retFailure = keyLineCount(filename, key, line);
                if(!retFailure)
                {//Found key in file at position 'line'
                    std::cerr << "Ungültiger Eintrag '" << key << "' in Zeile " << line << " gefunden, die Batch-Datei wird nicht erstellt" << std::endl;
                }
                //Unsupported key found. Return failure.
                retFailure = true;
                //beende Schleife
                break;
            }
            
        }
    }

    if(!retFailure)
    {//Wenn kein Fehler aufgetreten ist
        //Überprüfe ob key "outputfile" vorhanden ist und ob es ein String ist
        if (!obj.isMember("outputfile") || !obj["outputfile"].isString()) 
        {//Fehler
            std::cerr << "Ungültiger oder fehlender 'outputfile'-Eintrag in Zeile " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
            retFailure = true;
        }
    } 
    if(!retFailure)
    {
    cout << "Reading JSON file\n";
        std::string outputfile = obj["outputfile"].asString();
        cout << "Outputfile: " << outputfile << "\n";
        
        if (!obj.isMember("hideshell") || !obj["hideshell"].isBool()) 
        {
            std::cerr << "Ungültiger oder fehlender 'hideshell'-Eintrag in Zeile " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
            retFailure = true;
        }
        else if(!retFailure)
        {
        hideshell = obj["hideshell"].asBool();
        }

        const Json::Value& jsonEntries = obj["entries"];
        
        for (int i = 0; i < jsonEntries.size(); i++) {
            Entry entry;
            entry.type = jsonEntries[i]["type"].asString();
            if (entry.type == "ENV") {
                cout << "ENV entry found\n";
                if (jsonEntries[i].size() != 3 || !jsonEntries[i].isMember("key") || !jsonEntries[i].isMember("value")) {
                    std::cerr << "Ungültiger ENV-Eintrag in Zeile " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
                    retFailure = true;
                    break;
                } 
                entry.key = jsonEntries[i]["key"].asString();
                entry.value = jsonEntries[i]["value"].asString();
            } else if (entry.type == "EXE") {
                cout << "EXE entry found\n";
                if (jsonEntries[i].size() != 2 || !jsonEntries[i].isMember("command")) {
                    std::cerr << "Ungültiger EXE-Eintrag in Zeile " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
                    retFailure = true;
                    break;
                }
                entry.command = jsonEntries[i]["command"].asString();
            } else if (entry.type == "PATH") {
                cout << "PATH entry found\n";
                if (jsonEntries[i].size() != 2 || !jsonEntries[i].isMember("path")) {
                    std::cerr << "Ungültiger PATH-Eintrag in Zeile " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
                    retFailure = true;
                    break;
                }
                entry.path = jsonEntries[i]["path"].asString();
            } else {
                cout << "Unknown entry found\n";
                std::cerr << "Unbekannter Eintrag beim Einlesen der JSON-Datei in Zeile  " << (i+5) << ", die Batch-Datei wird nicht erstellt" << std::endl;
                retFailure = true;
                break;
            }
            entries.push_back(entry);
        } //end of for
        if(!retFailure)
        {
            if (!obj.isMember("application")) {
                std::cerr << "Ungültiger 'application'-Eintrag in Zeile " << line + jsonEntries.size() +2<< ", die Batch-Datei wird nicht erstellt" << std::endl;
                retFailure = true;
            } 
        } 
        else if(!retFailure)
        {   
            application = obj["application"].asString();
        }
        generateBatchFile();
    }
    
    return retFailure;
}//end of readJsonFile

    bool BatchGenerator::parseCommandLine(int argc, char *argv[]) 
{
    bool retFailure = false;
    cout << "Parsing command line arguments\n";
        
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            const string filename = argv[i];
            if (filename == "-h" || filename == "--help" || filename == "-help" || filename == "--h") {
                printHelp();
                exit(EXIT_SUCCESS);
            }

            retFailure = readJsonFile(filename);
            
        }
    } else {
        cerr << "Fehlende Eingabedatei.\n";
        //todo print help
        exit(EXIT_FAILURE);
    }

    return retFailure;
}//end of parseCommandLine


void BatchGenerator::generateBatchFile() 
{
    cout << "Erstelle Batch-Datei " << outputfile << ".\n";
    //Lege Ausgabedatei an
    ofstream outputToBatchFile(outputfile);

        // Ausblenden der Shell
        if (!hideshell) {
            //Schreibe in Batch-Datei
            outputToBatchFile << "@ECHO OFF\n";
        }

        if (hideshell) {
            //Schreibe in Batch-Datei
            outputToBatchFile << "C:\\Windows\\System32\\cmd.exe /c \"";
        }

        if (!hideshell) {
            //Schreibe in Batch-Datei
            outputToBatchFile << "C:\\Windows\\System32\\cmd.exe /k \"";
        }
        

        // Einträge verarbeiten
        for (const auto& entry : entries) {
            if (entry.type == "ENV") {
                // Umgebungsvariable setzen
                outputToBatchFile << "set " << entry.key << "=" << entry.value << "\n";
            } else if (entry.type == "EXE") {
                // Befehl ausführen
                outputToBatchFile << entry.command << "\n";
            } else if (entry.type == "PATH") {
                // Pfad hinzufügen
                outputToBatchFile << "set path=" << entry.path << ";%path%\n";
            }
        }

        // Abschließende Anwendungsausführung, falls angegeben
        if (!application.empty()) {
            outputToBatchFile << application << "\"\n";
        }

        // Einblendung der Shell
        if (!hideshell) {
            outputToBatchFile << "@ECHO ON\n";
        }
        outputToBatchFile << "\r\n";
        //Schliesse ausgabestream
        outputToBatchFile.close();
}//end of generateBatchFile

void BatchGenerator::printHelp() 
{
    cout << "Benutzung: batchgen [option] <inputfile>\n"
        << "Erstellt eine Batch-Datei basierend auf den Einstellungen in der JSON-Datei.\n\n"
        << "Optionen:\n"
        << "  -h, --help       Zeigt diese Hilfeanzeige an.\n\n"
        << "Autorenteam:\n"
        << "  Entwickelt von Team XYZ\n"
        << "  Kontakt:\n";
}//end of printHelp


bool BatchGenerator::keyLineCount(const std::string& filename, const std::string& key, size_t& lineCount)
{
    bool retFailure = true;
    std::ifstream file(filename);
    if (!std::filesystem::exists(filename)) 
    {//Failure occured
        std::cerr << "File does not exist: " << filename << std::endl;
        //Failure occured
        retFailure = true;
    }
    else
    {//File exists
        std::string line;
        size_t lineCounter = 0;
        while (std::getline(file, line)) 
        {//Iterate over lines
            //Increment line counter
            lineCounter++;
            //Search key in line
            size_t keyposition = line.find(key, 0);
            
            //Check if key is found in line
            if (keyposition != string::npos) 
            {//Key found
                //Return result
                lineCount = lineCounter;
                //return success
                retFailure = false;
                break;
            }
                       
        }
    }

    return retFailure;
}//end of lineCounter

