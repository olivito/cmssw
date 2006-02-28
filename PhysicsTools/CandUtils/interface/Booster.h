#ifndef CandUtils_Booster_h
#define CandUtils_Booster_h
#include "DataFormats/Candidate/interface/Candidate.h"

struct Booster : public reco::Candidate::setup {
  typedef reco::Candidate::Vector Vector;
  Booster( const Vector & b ) : 
    reco::Candidate::setup( setupCharge( false ), setupP4( true ) ), 
    boost( b ) { }
  virtual ~Booster();
  virtual void set( reco::Candidate& c );
  const Vector & boostVector() { return boost; }
private:
  const Vector boost;
};

struct CenterOfMassBooster : public Booster {
  CenterOfMassBooster( const reco::Candidate & c ) : Booster( c.boostToCM() ) {
  }
};

#endif
