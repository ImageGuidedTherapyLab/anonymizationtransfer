/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

//  Software Guide : BeginLatex
//
//  This example illustrates how to read a DICOM series into a volume and then
//  save this volume into another DICOM series using the exact same header
//  information. It makes use of the GDCM library.
//
//  The main purpose of this example is to show how to properly propagate the
//  DICOM specific information along the pipeline to be able to correctly write
//  back the image using the information from the input DICOM files.
//
//  Please note that writing DICOM files is quite a delicate operation since we
//  are dealing with a significant amount of patient specific data. It is your
//  responsibility to verify that the DICOM headers generated from this code
//  are not introducing risks in the diagnosis or treatment of patients. It is
//  as well your responsibility to make sure that the privacy of the patient is
//  respected when you process data sets that contain personal information.
//  Privacy issues are regulated in the United States by the HIPAA
//  norms\footnote{The Health Insurance Portability and Accountability Act of
//  1996. \url{http://www.cms.hhs.gov/hipaa/}}. You would probably find similar
//  legislation in every country.
//
//  \index{HIPAA!Privacy}
//  \index{HIPAA!Dicom}
//  \index{Dicom!HIPPA}
//
//  When saving datasets in DICOM format it must be made clear whether these
//  datasets have been processed in any way, and if so, you should inform the
//  recipients of the data about the purpose and potential consequences of the
//  processing. This is fundamental if the datasets are intended to be used for
//  diagnosis, treatment or follow-up of patients. For example, the simple
//  reduction of a dataset from a 16-bits/pixel to a 8-bits/pixel
//  representation may make it impossible to detect certain pathologies and
//  as a result will expose the patient to the risk of remaining untreated for a
//  long period of time while her/his pathology progresses.
//
//  You are strongly encouraged to get familiar with the report on medical
//  errors ``To Err is Human'', produced by the U.S. Institute of
//  Medicine~\cite{ToErrIsHuman2001}. Raising awareness about the high
//  frequency of medical errors is a first step in reducing their occurrence.
//
//  \index{Medical Errors}
//
//  Software Guide : EndLatex

// Software Guide : BeginLatex
//
// After all these warnings, let us now go back to the code and get familiar
// with the use of ITK and GDCM for writing DICOM Series. The first step that
// we must take is to include the header files of the relevant classes. We
// include the GDCMImageIO class, the GDCM fileNames generator, as well as
// the series reader and writer.
//
// Software Guide : EndLatex

// Software Guide : BeginCodeSnippet
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include "gdcmUIDGenerator.h"
// Software Guide : EndCodeSnippet

#include <vector>
#include "itksys/SystemTools.hxx"

int
main(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " DicomDirectory  OutputDicomDirectory [stack split tag]"
              << std::endl;
    return EXIT_FAILURE;
  }

  //  Software Guide : BeginLatex
  //
  //  As a second step, we define the image type to be used in this example. This
  //  is done by explicitly selecting a pixel type and a dimension. Using the
  //  image type we can define the type of the series reader.
  //
  //  Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  using PixelType = signed short;
  constexpr unsigned int Dimension = 3;

  using ImageType = itk::Image<PixelType, Dimension>;
  using ReaderType = itk::ImageSeriesReader<ImageType>;
  // Software Guide : EndCodeSnippet

  // Software Guide : BeginLatex
  //
  //  We also declare types for the \doxygen{GDCMImageIO} object that will
  //  actually read and write the DICOM images, and the
  //  \doxygen{GDCMSeriesFileNames} object that will generate and order all the
  //  fileNames for the slices composing the volume dataset. Once we have the
  //  types, we proceed to create instances of both objects.
  //
  // Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  using ImageIOType = itk::GDCMImageIO;
  using NamesGeneratorType = itk::GDCMSeriesFileNames;

  NamesGeneratorType::Pointer namesGenerator = NamesGeneratorType::New();
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //
  //  Just as the previous example, we get the DICOM fileNames from the
  //  directory. Note however, that in this case we use the
  //  \code{SetInputDirectory()} method instead of the \code{SetDirectory()}.
  //  This is done because in the present case we will use the fileNames
  //  generator for producing both the fileNames for reading and the fileNames
  //  for writing. Then, we invoke the \code{GetInputFileNames()} method in order
  //  to get the list of fileNames to read.
  //
  //  Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  namesGenerator->SetUseSeriesDetails( true );
  //entryId =" 0008|0021" ;   // Series Date
  std::string entryId = "0020|0012" ; // acquisition number
  if( argc > 3 ) // change the stack break criteria
    {
    entryId = argv[3];
    }
  std::string labelId;
  if( itk::GDCMImageIO::GetLabelFromTag( entryId , labelId ) )
    {
    std::cout << labelId << " (" << entryId << "): ";
    }
  else
    {
    std::cerr << "Trying to access inexistant DICOM tag." << entryId<< std::endl;
    return EXIT_FAILURE;
    }
  namesGenerator->AddSeriesRestriction(entryId  ); 
  namesGenerator->SetDirectory(argv[1]);

  try
    {
    std::cout << std::endl << "The directory: " << std::endl;
    std::cout << std::endl << argv[1] << std::endl << std::endl;
    std::cout << "Contains the following DICOM Series: ";
    std::cout << std::endl << std::endl;


// Software Guide : BeginLatex
//
// The GDCMSeriesFileNames object first identifies the list of DICOM series
// present in the given directory. We receive that list in a reference
// to a container of strings and then we can do things like print out all
// the series identifiers that the generator had found. Since the process of
// finding the series identifiers can potentially throw exceptions, it is
// wise to put this code inside a \code{try/catch} block.
//
// Software Guide : EndLatex

// Software Guide : BeginCodeSnippet
    typedef std::vector< std::string >    SeriesIdContainer;

    const SeriesIdContainer & seriesUID   = namesGenerator->GetSeriesUIDs();

    SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
    SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
    while( seriesItr != seriesEnd  )
      {
      std::string seriesIdentifier = seriesItr->c_str();
      std::cout << std::endl << std::endl;
      std::cout << "Now reading series: " << std::endl << std::endl;
      std::cout << seriesIdentifier  << std::endl;
      std::cout << std::endl << std::endl;


// Software Guide : BeginLatex
//
// Given that it is common to find multiple DICOM series in the same directory,
// we must tell the GDCM classes what specific series we want to read. In
// this example we do this by checking first if the user has provided a series
// identifier in the command line arguments. If no series identifier has been
// passed, then we simply use the first series found during the exploration of
// the directory.
//
// Software Guide : EndLatex

// Software Guide : BeginCodeSnippet

// Software Guide : EndCodeSnippet



// Software Guide : BeginLatex
//
// We pass the series identifier to the name generator and ask for all the
// fileNames associated to that series. This list is returned in a container of
// strings by the \code{GetFileNames()} method.
//
// \index{itk::GDCMSeriesFileNames!GetFileNames()}
//
// Software Guide : EndLatex

// Software Guide : BeginCodeSnippet
         typedef std::vector< std::string >   FileNamesContainer;
         FileNamesContainer fileNames;
     
         fileNames = namesGenerator->GetFileNames( seriesIdentifier );

         std::size_t numberOfFileNames = fileNames.size();
         std::cout << numberOfFileNames << std::endl;
         // for (unsigned int fni = 0; fni < numberOfFileNames; ++fni)
         // {
         //   std::cout << "filename # " << fni << " = ";
         //   std::cout << fileNames[fni] << std::endl;
         // }
// Software Guide : EndCodeSnippet
  // Software Guide : BeginLatex
  //
  // We construct one instance of the series reader object. Set the DICOM image
  // IO object to be used with it, and set the list of fileNames to read.
  //
  // Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  ReaderType::Pointer reader = ReaderType::New();
  ImageIOType::Pointer        gdcmIO = ImageIOType::New();

  // keep original UID
  gdcmIO->SetKeepOriginalUID(true);
  reader->SetImageIO(gdcmIO);
  reader->SetFileNames(fileNames);
  // Software Guide : EndCodeSnippet

  // Software Guide : BeginLatex
  //
  // We can trigger the reading process by calling the \code{Update()} method on
  // the series reader. It is wise to put this invocation inside a
  // \code{try/catch} block since the process may eventually throw exceptions.
  //
  // Software Guide : EndLatex

  try
  {
    // Software Guide : BeginCodeSnippet
    reader->Update();
    // Software Guide : EndCodeSnippet
  }
  catch (itk::ExceptionObject & excp)
  {
    std::cerr << "Exception thrown while writing the image" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
  }

  // Software Guide : BeginLatex
  //
  // At this point we have the volumetric data loaded in memory and we can
  // access it by invoking the \code{GetOutput()} method in the reader.
  //
  // Software Guide : EndLatex

  //  Software Guide : BeginLatex
  //
  //  Now we can prepare the process for writing the dataset. First, we take the
  //  name of the output directory from the command line arguments.
  //
  //  Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  const char * outputDirectory = argv[2];
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //
  //  Second, we make sure the output directory exists, using the cross-platform
  //  tools: itksys::SystemTools. In this case we choose to create the directory
  //  if it does not exist yet.
  //
  //  \index{itksys!SystemTools}
  //  \index{itksys!MakeDirectory}
  //  \index{SystemTools}
  //  \index{SystemTools!MakeDirectory}
  //  \index{MakeDirectory!SystemTools}
  //  \index{MakeDirectory!itksys}
  //
  //  Software Guide : EndLatex

  // Software Guide : EndCodeSnippet

  // Software Guide : BeginLatex
  //
  // We explicitly instantiate the image type to be used for writing, and use the
  // image type for instantiating the type of the series writer.
  //
  // Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  using OutputPixelType = signed short;
  constexpr unsigned int OutputDimension = 2;

  using Image2DType = itk::Image<OutputPixelType, OutputDimension>;

  using SeriesWriterType = itk::ImageSeriesWriter<ImageType, Image2DType>;
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //
  //  We construct a series writer and connect to its input the output from the
  //  reader. Then we pass the GDCM image IO object in order to be able to write
  //  the images in DICOM format.
  //
  //  the writer filter.  Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet

  // generate and store new UID for series 
  const std::vector<itk::MetaDataDictionary *> *dictionaryarray = reader->GetMetaDataDictionaryArray();
  //std::size_t numberOfdict = dictionaryarray->size();
  //std::cout << numberOfdict << std::endl;
  gdcm::UIDGenerator uid;
  std::string mySeriesInstanceUID = uid.Generate();
  for (unsigned int jjj = 0; jjj < dictionaryarray->size(); jjj++)
  {
    // each element is a raw pointer
    itk::MetaDataDictionary *mydictionary = dictionaryarray->at(jjj);
    itk::EncapsulateMetaData<std::string>( *mydictionary , "0020|000e", mySeriesInstanceUID );
  }

  SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();
  seriesWriter->SetInput(reader->GetOutput());
  seriesWriter->SetImageIO(gdcmIO);
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //
  //  It is time now to setup the GDCMSeriesFileNames to generate new fileNames
  //  using another output directory.  Then simply pass those newly generated
  //  files to the series writer.
  //
  //  \index{GDCMSeriesFileNames!SetOutputDirectory()}
  //  \index{GDCMSeriesFileNames!GetOutputFileNames()}
  //  \index{ImageSeriesWriter!SetFileNames()}
  //
  //  Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  int myindex = std::distance( seriesUID.begin(), seriesItr  );
  std::ostringstream output4ddir ;
  output4ddir << outputDirectory << myindex ;
  namesGenerator->SetOutputDirectory(output4ddir.str());
  // Software Guide : BeginCodeSnippet
  itksys::SystemTools::MakeDirectory(output4ddir.str());

  seriesWriter->SetFileNames(namesGenerator->GetOutputFileNames());
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The following line of code is extremely important for this process to work
  //  correctly.  The line is taking the MetaDataDictionary from the input reader
  //  and passing it to the output writer. This step is important because the
  //  MetaDataDictionary contains all the entries of the input DICOM header.
  //
  //  \index{itk::ImageSeriesReader!GetMetaDataDictionaryArray()}
  //  \index{itk::ImageSeriesWriter!SetMetaDataDictionaryArray()}
  //
  //  Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  seriesWriter->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());
  // Software Guide : EndCodeSnippet

  // Software Guide : BeginLatex
  //
  // Finally we trigger the writing process by invoking the \code{Update()} method
  // in the series writer. We place this call inside a \code{try/catch} block,
  // in case any exception is thrown during the writing process.
  //
  // Software Guide : EndLatex

  // Software Guide : BeginCodeSnippet
  try
  {
    seriesWriter->Update();
  }
  catch (itk::ExceptionObject & excp)
  {
    std::cerr << "Exception thrown while writing the series " << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
  }
  // Software Guide : EndCodeSnippet

  // Software Guide : BeginLatex
  //
  // Please keep in mind that you should avoid generating DICOM files which have
  // the appearance of being produced by a scanner. It should be clear from the
  // directory or fileNames that these data were the result of the
  // execution of some sort of algorithm. This will prevent your dataset
  // from being used as scanner data by accident.
  //
  // Software Guide : EndLatex
// Software Guide : EndCodeSnippet
         ++seriesItr;
         }
      }
    catch (itk::ExceptionObject &ex)
      {
      std::cout << ex << std::endl;
      return EXIT_FAILURE;
      }

  return EXIT_SUCCESS;
}
