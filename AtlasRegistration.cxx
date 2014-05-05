#include "AtlasRegistration.h"

AtlasRegistration::AtlasRegistration(QString module) : Script(module)
{

}

// SET // 
void AtlasRegistration::setLog(QString log_path)
{
   m_log_path = log_path; 
}
void AtlasRegistration::setAntsParameters(AntsParameters* parameters) 
{
   m_parameters=parameters;
}

void AtlasRegistration::initializeScript()
{
   definePython();
   importGeneralModules();

   defineExecutable("ANTS");
   defineExecutable("ResampleVolume2");
 
   m_script += "logger = None\n";

   m_script += "runningProcess = None\n";  
   m_script += "logFile = None\n\n";  
}

void AtlasRegistration::initializeLogging()
{
   m_script += "def initializeLogging(log):\n";

   m_script += "\tglobal logger\n";   
   m_script += "\tlogger = logging.getLogger('NeosegPipelineRegistrations')\n"; 
   m_script += "\tlogger.setLevel(logging.DEBUG)\n";

   m_script += "\tfileHandler = logging.FileHandler(log)\n";
   m_script += "\tfileHandler.setLevel(logging.DEBUG)\n";
   m_script += "\tfileFormatter = logging.Formatter('%(message)s')\n";
   m_script += "\tfileHandler.setFormatter(fileFormatter)\n";

   m_script += "\tlogger.addHandler(fileHandler)\n\n";
}

void AtlasRegistration::implementRegisterAtlas(bool probabilistic)
{   
   if(probabilistic)
   {
      m_script += "def main(name, T1Atlas, T2Atlas, whiteAtlas, grayAtlas, csfAtlas, outbase, log):\n\n";
   } 
   else
   {
      m_script += "def main(name, T1Atlas, T2Atlas, segAtlas, outbase, log):\n\n";
   }

   m_script += "\tsignal.signal(signal.SIGINT, stop)\n";
   m_script += "\tsignal.signal(signal.SIGTERM, stop)\n\n";

   m_script += "\tlogFile = open(log, \"w\")\n";
   m_script += "\tinitializeLogging(log)\n";

   m_script += "\tfinalT1 = outbase + '-T1.nrrd'\n";
   m_script += "\tfinalT2 = outbase + '-T2.nrrd'\n\n";

   if(probabilistic)
   {
      m_script += "\tfinalWhite = outbase + '-white.nrrd'\n";
      m_script += "\tfinalGray = outbase + '-gray.nrrd'\n";
      m_script += "\tfinalCsf = outbase + '-csf.nrrd'\n";
      m_script += "\tif checkFileExistence(finalT1)==True or checkFileExistence(finalT2) or checkFileExistence(finalWhite) or checkFileExistence(finalGray) or checkFileExistence(finalCsf) :\n";
   }
   else
   {
      m_script += "\tfinalSeg = outbase + '-csf.nrrd'\n";
      m_script += "\tif checkFileExistence(finalT1)==True or checkFileExistence(finalT2) or checkFileExistence(finalSeg) :\n";
   }

   m_script += "\t\tmainLog = open('" + m_log_path + "','w')\n"; 
   m_script += "\t\tmainLog.write(name + ' Registration -> Skipped\\n')\n"; 
   m_script += "\t\tmainLog.close()\n";  
   m_script += "\t\treturn\n"; 

   m_script += "\tmainLog = open('" + m_log_path + "','a')\n"; 
   m_script += "\tmainLog.write(name + ' Registration...\\n')\n"; 
   m_script += "\tmainLog.close()\n";  

   m_script += "\tT1 = '" + m_neo.T1 + "'\n";
   m_script += "\tT2 = '" + m_neo.T2 + "'\n";  
   
   QString modality1 = "CC[' + T1 + ',' + T1Atlas + '," + QString::number(m_parameters->getWeight1()) + "," + QString::number(m_parameters->getRadius1()) + "]";
   QString modality2 = "CC[' + T2 + ',' + T2Atlas + '," + QString::number(m_parameters->getWeight2()) + "," + QString::number(m_parameters->getRadius2()) + "]";
   QString iterations = QString::number(m_parameters->getIterationsJ()) + "x" + QString::number(m_parameters->getIterationsK()) + "x" + QString::number(m_parameters->getIterationsL());
   QString transformation = m_parameters->getTransformationType() + "[" + QString::number(m_parameters->getGradientStepLength()) + "]"; //"," + QString::number(m_parameters->getNumberOfTimeSteps()) + "," + QString::number(m_parameters->getDeltaTime()) + "]";
   QString regularization = m_parameters->getRegularizationType() + "[" + QString::number(m_parameters->getGradientFieldSigma()) + "," + QString::number(m_parameters->getDeformationFieldSigma()) + "]";
   QString output = "' + outbase + '_.nii.gz";

   QString affine = "' + outbase + '_Affine.txt";
   QString warp = "' + outbase + '_Warp.nii.gz";

   m_log = "Calculating transformations";
   m_inputs.insert("mask", m_neo.mask);
   m_inputs.insert("modality1", modality1); 
   m_inputsTests.insert("modality1", "T1Atlas");
   m_inputs.insert("modality2", modality2); 
   m_inputsTests.insert("modality2", "T2Atlas");
   m_inputs.insert("iterations", iterations); 
   m_inputs.insert("transformation", transformation);
   m_inputs.insert("regularization", regularization);  
   m_inputs.insert("output", output );
   m_outputs.insert("affine", affine);
   m_outputs.insert("warp", warp);  
   m_argumentsList << "ANTS" << "'3'" << "'-m'" << "modality1" << "'-m'" << "modality2" << "'-o'" << "output" << "'-i'" << "iterations" << "'-t'" << "transformation" << "'-r'" << "regularization" << "'-x'" << "mask"; 

   QStringList argsTests_T1; 
   argsTests_T1 << "ANTS" << "'3'" << "'-m'" << "modality2" << "'-o'" << "output" << "'-i'" << "iterations" << "'-t'" << "transformation" << "'-r'" << "regularization" << "'-x'" << "mask"; 
   m_argsTests.insert("T1Atlas", argsTests_T1); 

   QStringList argsTests_T2; 
   argsTests_T2 << "ANTS" << "'3'" << "'-m'" << "modality1" << "'-o'" << "output" << "'-i'" << "iterations" << "'-t'" << "transformation" << "'-r'" << "regularization" << "'-x'" << "mask"; 
   m_argsTests.insert("T2Atlas", argsTests_T2); 

   execute(); 

   // Normalizing affine file
   m_log = "Normalizing affine file";
   m_argumentsList << "'text_subst.pl'" << "'MatrixOffsetTransformBase_double_3_3'" << "'AffineTransform_double_3_3'" << "affine"; 
   execute();

   // Applying transformations to T1
   QString T1Reg = "' + outbase + '-T1.nrrd";

   m_log = "Applying transformations to T1";
   m_test = "T1Atlas";
   m_outputs.insert("T1Reg", T1Reg); 
   m_argumentsList << "ResampleVolume2" << "T1Atlas" << "T1Reg" << "'--Reference'" << "T2" << "'-i'" << "'bs'" << "'--hfieldtype'" << "'displacement'" << "'--defField'" << "warp" << "'--transformationFile'" << "affine";
   execute();

   // Applying transformations to T2
   QString T2Reg = "' + outbase + '-T2.nrrd";

   m_log = "Applying transformations to T2";
   m_test = "T2Atlas";
   m_outputs.insert("T2Reg", T2Reg); 
   m_argumentsList << "ResampleVolume2" << "T2Atlas" << "T2Reg" << "'--Reference'" << "T2" << "'-i'" << "'bs'" << "'--hfieldtype'" << "'displacement'" << "'--defField'" << "warp" << "'--transformationFile'" << "affine";
   execute(); 

   if(probabilistic)
   {
      // Applying transformations to the white probability 
      QString whiteReg = "' + outbase + '-white.nrrd";

      m_log = "Applying transformations to the white probability";
      m_outputs.insert("whiteReg", whiteReg); 
      m_argumentsList << "ResampleVolume2" << "whiteAtlas" << "whiteReg" << "'--Reference'" << "T2" << "'-i'" << "'nn'" << "'--hfieldtype'" << "'displacement'" << "'--defField'" << "warp" << "'--transformationFile'" << "affine";
      execute(); 

      // Applying transformations to the gray probability 
      QString grayReg = "' + outbase + '-gray.nrrd";

      m_log = "Applying transformations to the gray probability";
      m_outputs.insert("grayReg", grayReg);
      m_argumentsList << "ResampleVolume2" << "grayAtlas" << "grayReg" << "'--Reference'" << "T2" << "'-i'" << "'nn'" << "'--hfieldtype'" << "'displacement'" << "'--defField'" << "warp" << "'--transformationFile'" << "affine";
      execute(); 

      // Applying transformations to the CSF probability 
      QString csfReg = "' + outbase + '-csf.nrrd";

      m_log = "Applying transformations to the CSF probability";
      m_outputs.insert("csfReg", csfReg);
      m_argumentsList << "ResampleVolume2" << "csfAtlas" << "csfReg" << "'--Reference'" << "T2" << "'-i'" << "'nn'" << "'--hfieldtype'" << "'displacement'" << "'--defField'" << "warp" << "'--transformationFile'" << "affine";
      execute(); 
   }
   
   else
   {
      // Applying transformations to the segmentation
      QString segReg = "' + outbase + '-seg.nrrd";

      m_log = "Applying transformations to the white probability";
      m_outputs.insert("segReg", segReg);
      m_argumentsList << "ResampleVolume2" << "segAtlas" << "segReg" << "'--Reference'" << "T2" << "'-i'" << "'nn'" << "'--hfieldtype'" << "'displacement'" << "'--defField'" << "warp" << "'--transformationFile'" << "affine";
      execute(); 
   }
}

void AtlasRegistration::writeRegisterAtlas() // args = (T1Atlas, T2Atlas, output, log)
{
   initializeScript();

   implementStop();
   implementCheckFileExistence();
   initializeLogging(); 
   implementExecute();

   implementRegisterAtlas(0);

   m_script += "if __name__ == '__main__':\n";
   m_script += "\tmain(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6])\n";

   QString script_path = m_processing_dir->filePath("atlasRegistration.py");

   std::ofstream* script_stream = new std::ofstream((script_path.toStdString()).c_str(), std::ios::out | std::ios::trunc);
   *script_stream << m_script.toStdString() << std::endl; 
}

void AtlasRegistration::writeRegisterProbabilisticAtlas() // args = (T1Atlas, T2Atlas, whiteAtlas, grayAtlas, csfAtlas, outbase, log)
{
   m_script = ""; 

   initializeScript();

   implementStop();
   implementCheckFileExistence();
   initializeLogging(); 
   implementExecute();

   implementRegisterAtlas(1);

   m_script += "if __name__ == '__main__':\n";
   m_script += "\tmain(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6], sys.argv[7], sys.argv[8])\n";

   QString script_path = m_processing_dir->filePath("probabilisticAtlasRegistration.py");

   std::ofstream* script_stream = new std::ofstream((script_path.toStdString()).c_str(), std::ios::out | std::ios::trunc);
   *script_stream << m_script.toStdString() << std::endl; 
}

void AtlasRegistration::update()
{     
   writeRegisterAtlas();
   writeRegisterProbabilisticAtlas();
}


