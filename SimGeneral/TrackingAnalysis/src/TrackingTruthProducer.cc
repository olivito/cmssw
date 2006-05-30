#include "CLHEP/Vector/LorentzVector.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "SimDataFormats/HepMCProduct/interface/HepMCProduct.h"
#include "SimDataFormats/Track/interface/EmbdSimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/EmbdSimVertexContainer.h"

#include "SimGeneral/TrackingAnalysis/interface/TrackingTruthProducer.h"

using namespace edm;
using namespace std; 

string MessageCategory = "TrackingTruth";

TrackingTruthProducer::TrackingTruthProducer(const edm::ParameterSet &conf) {
  produces<TrackingVertexContainer>();
  conf_ = conf;
  distanceCut_ = conf_.getParameter<double>("distanceCut");
  dataLabels_  = conf_.getParameter<vector<string> >("dataLabels");
  edm::LogInfo (MessageCategory) << "Vertex distance cut set to " << distanceCut_ << " mm";
}

void TrackingTruthProducer::produce(Event &event, const EventSetup &) {
  edm::Handle<edm::HepMCProduct>           hepMC;
  edm::Handle<EmbdSimVertexContainer>      G4VtxContainer;
  edm::Handle<edm::EmbdSimTrackContainer>  G4TrkContainer;
  CLHEP::HepLorentzVector                  position;
  auto_ptr<TrackingVertexContainer>        tVC( new TrackingVertexContainer );  

  // Get information out of event record
  
  for (vector<string>::const_iterator source = dataLabels_.begin(); source !=
      dataLabels_.end(); ++source) {
    try {
      event.getByLabel(*source,hepMC);
      edm::LogInfo (MessageCategory) << "Using HepMC source " << *source;
      break;
    } catch (std::exception &e) {
      
    }    
  }
  
  event.getByType(G4VtxContainer);
  event.getByType(G4TrkContainer);
  const edm::HepMCProduct          *mcp =          hepMC.product();
  const edm::EmbdSimTrackContainer *etc = G4TrkContainer.product();

  if (mcp == 0) {
    edm::LogWarning (MessageCategory) << "No HepMC source found";
    return;
  }  
  
  // Find and loop over vertices from HepMC
  const HepMC::GenEvent *hme = mcp -> GetEvent();
//  hme -> print();
  
  // Find and loop over EmbdSimVertex vertices
    
  int index = 0;
  for (edm::EmbdSimVertexContainer::const_iterator itVtx = G4VtxContainer->begin(); 
       itVtx != G4VtxContainer->end(); 
       ++itVtx) {
         
    position = itVtx -> position();  // Get position of ESV
    math::XYZPoint mPosition = math::XYZPoint(position.x(),position.y(),position.z());
    int vtxParent = itVtx -> parentIndex(); // Get incoming track (EST)
    
    string InOut;
    int partHepMC = -1;
    
    if (position.perp() < 1200 && abs(position.z()) < 3000) { // In or out of Tracker
      InOut = "I";
    } else {
      InOut = "O";
    }
    
    if (vtxParent >= 0) {                     // If there is parent track, figure out HEPMC Vertex 
      EmbdSimTrack est = etc->at(vtxParent);  // Pull track out from vector
      partHepMC =     est.genpartIndex(); // Get HepMC particle barcode
      HepMC::GenParticle *hmp = hme -> barcode_to_particle(partHepMC); // Convert barcode
      if (hmp != 0) {
       HepMC::GenVertex *hmpv = hmp -> production_vertex(); 
       if (hmpv != 0) {
         int  vb = hmpv  -> barcode();
       }  
      }  
    }  

    double closest = 9e99;
    int iTVC = 0;
    int cTVC = 0;
    for (TrackingVertexContainer::const_iterator v =
        tVC -> begin();
        v != tVC ->end(); ++v) {
      math::XYZPoint vPosition = v->position();   
      double distance = sqrt(pow(vPosition.X()-mPosition.X(),2) +  
                             pow(vPosition.Y()-mPosition.Y(),2) + 
                             pow(vPosition.Z()-mPosition.Z(),2)); 
      if (distance < closest) {
        closest = distance;
        cTVC = iTVC;
        // flag which one so we can associate them
      }   
      ++iTVC;      
    }

    if (closest > distanceCut_) {
      TrackingVertex tV = TrackingVertex(mPosition);
      tV.addG4Vertex(EmbdSimVertexRef(G4VtxContainer, index) ); // Add G4 vertex
      // Add HepMC vertex
      // Add TrackingParticle (or maybe elsewhere)
      tVC -> push_back(tV);
      cTVC = tVC->size() - 1;
    } else {
      (tVC -> at(cTVC)).addG4Vertex(EmbdSimVertexRef(G4VtxContainer, index) ); // Add G4 vertex
      // Add HepMC vertex
      // Add TrackingParticle
    }  

    ++index;     
  }

  edm::LogInfo (MessageCategory) << "TrackingTruth found " << tVC->size() << " unique vertices";
  
  index = 0;
  for (edm::EmbdSimTrackContainer::const_iterator p = G4TrkContainer->begin(); 
       p != G4TrkContainer->end(); 
       ++p) {
         
    int partHepMC =    p -> genpartIndex();  
    HepMC::GenParticle *hmp = hme -> barcode_to_particle(partHepMC);
    
    if (hmp != 0) {
      HepMC::GenVertex *hmpv = hmp -> production_vertex(); 
      if (hmpv != 0) {
        int vb = hmpv  -> barcode();
      }  
    }  
    ++index;  
  }

  index = 0;
  for (TrackingVertexContainer::const_iterator v =
       tVC -> begin();
       v != tVC ->end(); ++v) {
//    edm::LogInfo (MessageCategory) << "TrackingVertex " << index << " has " << (v -> g4Vertices()).size() << " G4 vertices";
    ++index;  
  }        
  
  // Put new info into event record  
  
  event.put(tVC);
}

DEFINE_FWK_MODULE(TrackingTruthProducer)
