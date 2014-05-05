#ifndef DEF_Pipeline
#define DEF_Pipeline

// General Librairies
#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <vector>
#include <utility>
#include <sys/stat.h>

// Qt Librairies
#include <QDir>
#include <QStringList>
#include <QProcess>
#include <QTime>
#include <QPlainTextEdit>

// My Specific Librairies
#include "PipelineParameters.h"
#include "PreProcessingData.h"
#include "DtiRegistration.h"
#include "AtlasPopulationRegistration.h"
#include "AtlasRegistration.h"
#include "AtlasGeneration.h"
#include "NeosegExecution.h"


class Pipeline
{

   public:

   // Constructor
   Pipeline();

   // Destructor
   ~Pipeline(); 

   // Parameters
   void setPipelineParameters(PipelineParameters* parameters);
   void setPlainTextEdit(QPlainTextEdit* plainTextEdit); 

   // Process
   QProcess* getMainScriptProcess();

   // Write Pipeline 
   void writePipeline(); 

   // Run Pipeline
   void runPipeline();

   private: 

   // Create Directories
   void createProcessingDirectory();
   QString createModuleDirectory(QString directory_name);

   // Write Neoseg Pipeline Steps
   void writePreProcessingData();
   void writeAtlasRegistration();
   void writeAtlasPopulationRegistration();
   void writeDTIRegistration();   
   void writeAtlasGeneration();
   void writeNeosegExecution();

   // Write Main Script 
   void initializeMainScript();
   void initializeLogging();
   void defineSignalHandler();
   void writeMainScript();
   void executeMainScript();
   void copySegmentations();
   
   // Clean-Up
   void cleanUp(); 




   // Neoseg Pipeline Steps
   PreProcessingData* m_preProcessingData;
   DtiRegistration* m_dtiRegistration; 
   AtlasRegistration* m_atlasRegistration; 
   AtlasPopulationRegistration* m_atlasPopulationRegistration; 
   AtlasGeneration* m_atlasGeneration; 
   NeosegExecution* m_neosegExecution; 

   PipelineParameters* m_parameters; 

   QString m_processing_name;
   QString m_processing_path;

   QString m_script;
   QString m_importingModules;
   QString m_runningModules;

   QString m_main_path;

   QString m_log_path; 

   // QProcess
   QProcess* m_mainScriptProcess;

   // PlainTextEdit
   QPlainTextEdit* m_plainTextEdit; 
 
   // Timer
   QTime m_timer; 
};

#endif


