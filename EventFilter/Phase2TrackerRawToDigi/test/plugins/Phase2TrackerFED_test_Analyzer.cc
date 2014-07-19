#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"
#include "EventFilter/Phase2TrackerRawToDigi/interface/utils.h"
#include "EventFilter/Phase2TrackerRawToDigi/test/plugins/Phase2TrackerFED_test_Analyzer.h"
#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerFEDBuffer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include <sstream>
#include <string>
#include <map>

using namespace Phase2Tracker;
using namespace std;

// -----------------------------------------------------------------------------
// 
Phase2TrackerFED_test_Analyzer::Phase2TrackerFED_test_Analyzer( const edm::ParameterSet& pset )
{
  LogDebug("Phase2TrackerFED_test_Analyzer")
    << "[Phase2TrackerFED_test_Analyzer::"
    << __func__ << "]"
    << "Constructing object...";
  token_ = consumes<FEDRawDataCollection>(pset.getParameter<edm::InputTag>("ProductLabel"));
}

// -----------------------------------------------------------------------------
// 
Phase2TrackerFED_test_Analyzer::~Phase2TrackerFED_test_Analyzer() 
{
  LogDebug("Phase2TrackerFED_test_Analyzer")
    << "[Phase2TrackerFED_test_Analyzer::"
    << __func__ << "]"
    << " Destructing object...";
}

// -----------------------------------------------------------------------------
// 
void Phase2TrackerFED_test_Analyzer::beginJob() 
{
  LogDebug("Phase2TrackerFED_test_Analyzer")
    << "[Phase2TrackerFED_test_Analyzer::"
    << __func__ << "]";
}

// -----------------------------------------------------------------------------
// 
void Phase2TrackerFED_test_Analyzer::endJob() 
{
  LogDebug("Phase2TrackerFED_test_Analyzer")
    << "[Phase2TrackerFED_test_Analyzer::"
    << __func__ << "]";
}

// -----------------------------------------------------------------------------
// 
void Phase2TrackerFED_test_Analyzer::analyze( const edm::Event& event, const edm::EventSetup& setup ) 
{
  // Retrieve FEDRawData collection
  edm::Handle<FEDRawDataCollection> buffers;
  event.getByToken( token_, buffers );



  // Analyze strip tracker FED buffers in data
  size_t fedIndex;
  for( fedIndex=0; fedIndex<Phase2Tracker::CMS_FED_ID_MAX; ++fedIndex )
  { 
    const FEDRawData& fed = buffers->FEDData(fedIndex);
    if(fed.size()!=0 && fedIndex >= Phase2Tracker::FED_ID_MIN && fedIndex <= Phase2Tracker::FED_ID_MAX)
    {
      // construct buffer
      Phase2Tracker:: Phase2TrackerFEDBuffer* buffer = 0;
      buffer = new Phase2Tracker::Phase2TrackerFEDBuffer(fed.data(),fed.size());

      cout << " -------------------------------------------- " << endl;
      cout << " buffer debug ------------------------------- " << endl;
      cout << " -------------------------------------------- " << endl;
      cout << " buffer size : " << buffer->bufferSize() << endl;
      cout << " fed id      : " << fedIndex << endl;
      cout << " -------------------------------------------- " << endl;
      cout << " tracker header debug ------------------------" << endl;
      cout << " -------------------------------------------- " << endl;
      
      Phase2TrackerHeader tr_header = buffer->trackerHeader();
      cout << " Version  : " << hex << setw(2) << (int) tr_header.getDataFormatVersion() << endl;
      cout << " Mode     : " << hex << setw(2) << (int) tr_header.getDebugMode() << endl;
      cout << " Type     : " << hex << setw(2) << (int) tr_header.getEventType() << endl;
      cout << " Readout  : " << hex << setw(2) << (int) tr_header.getReadoutMode() << endl;
      cout << " Status   : " << hex << setw(16)<< (int) tr_header.getGlibStatusCode() << endl;
      cout << " FE stat  : " ;
      for(int i=15; i>=0; i--)
      {
        if((tr_header.frontendStatus())[i])
        {
          cout << "1";
        }
        else
        { 
          cout << "0";
        }
      } 
      cout << endl;
      cout << " Nr CBC   : " << hex << setw(16)<< (int) tr_header.getNumberOfCBC() << endl;
      cout << " CBC stat : ";
      for(int i=0; i<tr_header.getNumberOfCBC(); i++)
      {
        cout << hex << setw(2) << (int) tr_header.CBCStatus()[i] << " ";
      }
      cout << endl;
      cout << " -------------------------------------------- " << endl;
      cout << " Payload  ----------------------------------- " << endl;
      cout << " -------------------------------------------- " << endl;

      // loop channels
      int ichan = 0;
      for ( int ife = 0; ife < 16; ife++ ) 
      {
        for ( int icbc = 0; icbc < 16; icbc++ )
        {
          const FEDChannel& channel = buffer->channel(ichan);
          if(channel.length() > 0) 
          {
            cout << dec << " reading channel : " << icbc << " on FE " << ife;
            cout << dec << " with length  : " << (int) channel.length() << endl;
            Phase2TrackerFEDRawChannelUnpacker unpacker = Phase2TrackerFEDRawChannelUnpacker(channel);
            while (unpacker.hasData())
            {
	      std::cout << ( unpacker.stripOn() ? "1" : "_" );
              unpacker++;
            }
            std::cout << std::endl;
          }
          ichan ++;
        }
      } // end loop on channels
    }
  }
}
