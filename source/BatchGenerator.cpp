#include "../includes/BatchGenerator.h"

using namespace std;

BatchGenerator::BatchGenerator(){}

bool BatchGenerator::readJsonFile(const std::string& filename) 
{
    bool retFailure = false;
    std::ifstream file(filename);
    Json::Reader reader;
    Json::Value obj;
    size_t line = 0;
    entries.clear();

    reader.parse(file, obj);
    
    if (!std::filesystem::exists(filename)) 
    {
        std::cerr << "Datei existiert nicht: " << filename << std::endl;
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
        outputfile = obj["outputfile"].asString();
        
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
        Entry entry;
        
        for (int i = 0; i < jsonEntries.size(); i++) {
            entry.type = jsonEntries[i]["type"].asString();
            if (entry.type == "ENV") {
                if (jsonEntries[i].size() != 3 || !jsonEntries[i].isMember("key") || !jsonEntries[i].isMember("value")) {
                    std::cerr << "Ungültiger ENV-Eintrag in Zeile " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
                    retFailure = true;
                    break;
                } 
                entry.key = jsonEntries[i]["key"].asString();
                entry.value = jsonEntries[i]["value"].asString();
            } else if (entry.type == "EXE") {
                if (jsonEntries[i].size() != 2 || !jsonEntries[i].isMember("command")) {
                    std::cerr << "Ungültiger EXE-Eintrag in Zeile " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
                    retFailure = true;
                    break;
                }
                entry.command = jsonEntries[i]["command"].asString();
            } else if (entry.type == "PATH") {
                if (jsonEntries[i].size() != 2 || !jsonEntries[i].isMember("path")) {
                    std::cerr << "Ungültiger PATH-Eintrag in Zeile " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
                    retFailure = true;
                    break;
                }
                entry.path = jsonEntries[i]["path"].asString();
            } else {
                std::cerr << "Unbekannter Eintrag beim Einlesen der JSON-Datei in Zeile  " << line << ", die Batch-Datei wird nicht erstellt" << std::endl;
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
        if(!retFailure)
        {   
            application = obj["application"].asString();
        }
        if(!retFailure)
        {
            generateBatchFile();
        }
    }
    
    return retFailure;
}//end of readJsonFile

bool BatchGenerator::parseCommandLine(int argc, char *argv[]) 
{
    bool retFailure = false;

    struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;
    
    if (argc == 1) {
    std::cerr << "Fehler: Keine Datei angegeben. Verwenden Sie -h oder --help für Hilfe." << std::endl;
    return true;
    }


    while ((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                printHelp();
                retFailure = true;
                break;
            case '?':
                // getopt_long already printed an error message.
                break;
            default:
                abort();
        }
    }

    if (!retFailure) {
        while (optind < argc) {
            retFailure = readJsonFile(argv[optind++]);
            if (retFailure) {
                break;
            }
        }
    }

    return retFailure;
}

void BatchGenerator::generateBatchFile() 
{
    cout << "Erstelle Batch-Datei: " << outputfile << "\n";
    //Lege Ausgabedatei an
    ofstream outputToBatchFile(outputfile);

        // Ausblenden der Shell
            //Schreibe in Batch-Datei
        outputToBatchFile << "@ECHO OFF\n";
      

        if (hideshell) 
        {
            //Schreibe in Batch-Datei
            outputToBatchFile << "C:\\Windows\\System32\\cmd.exe /c \"";
        }

        if (!hideshell) {
            //Schreibe in Batch-Datei
            outputToBatchFile << "C:\\Windows\\System32\\cmd.exe /k \"";
        }
        
        // Einträge verarbeiten
        string lastPath;
        for (const auto& entry : entries) {
            if (entry.type == "ENV") {
                // Umgebungsvariable setzen
                outputToBatchFile << " && set " << entry.key << "=" << entry.value;
            } else if (entry.type == "EXE") {
                // Befehl ausführen
                outputToBatchFile << entry.command;
            } else if (entry.type == "PATH") {
                if (firstIt){
                    lastPath = entry.path;
                    outputToBatchFile << " && set path=" << entry.path;
                    firstIt = false;
                } 
                else {
                    lastPath = entry.path;
                    outputToBatchFile << ";" << entry.path;
                }
            }
        }
        firstIt = true;
            if (!lastPath.empty())
            {
            outputToBatchFile << ";%path%";
            }

        // Abschließende Anwendungsausführung, falls angegeben
        if (!application.empty()) {

            std::string delimiter = ".";
            size_t pos = outputfile.find(delimiter);
            std::string extractedString = outputfile.substr(0, pos);
            
            outputToBatchFile << " && start \"" << extractedString <<"\" "<< application;
        }

        // Einblendung der Shell
            outputToBatchFile << "\"\n@ECHO ON\n";


        outputToBatchFile << "\r\n";
        //Schliesse ausgabestream
        outputToBatchFile.close();
}//end of generateBatchFile

void BatchGenerator::printHelp() 
{
    cout << "-Helptext-\n\n"
        << "Funktion des Programms:\n"
        << "  Das Programm liest JSON-Dateien ein und erstellt daraus Batch-Dateien.\n"
        << "  Die Batch-Dateien führen die in der JSON-Datei definierten Befehle aus.\n"
        << "  Durch anhängen von den Dateipfaden der JSON-Dateien als Argumente\n"
        << "  beim Start des Programms, können mehrere JSON-Dateien nacheinander eingelesen werden.\n"
        << "  Die Batch-Dateien werden im selben Verzeichnis wie die JSON-Datei erstellt.\n"
        << "  Die Batch-Dateien werden so benannt, wie es in der JSON-Datei definiert ist.\n"
        << "  Beispiel(Aufruf): ./genbatch datei1.json datei2.json datei3.json\n\n"
        << "Optionen:\n"
        << "  -h, --help       Zeigt diesen Helptext an.\n\n"
        << "Autorenteam:\n"
        << "  Entwickelt von Fabio Behmüller, Mika Urban, Niklas Scheffold und Tim Johler\n"
        << "  Kontakt:\n"
        << "    n.scheffold09@gmail.com\n"
        << "    mika.urban24@gmail.com\n";
}//end of printHelp


bool BatchGenerator::keyLineCount(const std::string& filename, const std::string& key, size_t& lineCount)
{
    bool retFailure = true;
    std::ifstream file(filename);
    if (!std::filesystem::exists(filename)) 
    {//Failure occured
        std::cerr << "-Fehler- Datei existiert nicht: " << filename << std::endl;
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

