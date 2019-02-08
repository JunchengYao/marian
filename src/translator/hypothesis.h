#pragma once
#include <memory>

#include "common/definitions.h"
#include "data/alignment.h"

namespace marian {

// one single (possibly partial) hypothesis in beam search
// key elements:
//  - the word that this hyp ends with
//  - the aggregate score up to and including the word
//  - back pointer to previous hypothesis for traceback
class Hypothesis {
public:
  Hypothesis() : prevHyp_(nullptr), beamHypIdx_(0), prevBeamHypIdx_(0), word_(Word::ZERO), pathScore_(0.0) {}

  Hypothesis(const Ptr<Hypothesis> prevHyp,
             Word word,
             size_t beamHypIdx, // EXPERIMENTAL, UNUSED. which beam does this come from? (beamHypIdx, word) indexes the logit tensor
             size_t prevBeamHypIdx, // beam-hyp index that this hypothesis originated from
             float pathScore)
      : prevHyp_(prevHyp), beamHypIdx_(beamHypIdx), prevBeamHypIdx_(prevBeamHypIdx), word_(word), pathScore_(pathScore) {}

  const Ptr<Hypothesis> getPrevHyp() const { return prevHyp_; }

  Word getWord() const { return word_; }

  size_t getPrevStateIndex() const { return prevBeamHypIdx_; }

  float getPathScore() const { return pathScore_; }

  std::vector<float>& getScoreBreakdown() { return scoreBreakdown_; }
  void setScoreBreakdown(const std::vector<float>& scoreBreaddown) { scoreBreakdown_ = scoreBreaddown; }

  const std::vector<float>& getAlignment() { return alignment_; }
  void setAlignment(const std::vector<float>& align) { alignment_ = align; };

  // helpers to trace back paths referenced from this hypothesis
  Words tracebackWords()
  {
      Words targetWords;
      for (auto hyp = this; hyp->getPrevHyp(); hyp = hyp->getPrevHyp().get()) {
        targetWords.push_back(hyp->getWord());
        // std::cerr << hyp->getWord() << " " << hyp << std::endl;
      }
      std::reverse(targetWords.begin(), targetWords.end());
      return targetWords;
  }

  // get soft alignments for each target word starting from the hyp one
  typedef data::SoftAlignment SoftAlignment;
  SoftAlignment tracebackAlignment()
  {
      SoftAlignment align;
      for (auto hyp = this; hyp->getPrevHyp(); hyp = hyp->getPrevHyp().get()) {
          align.push_back(hyp->getAlignment());
      }
      std::reverse(align.begin(), align.end());
      return align;
  }

private:
  const Ptr<Hypothesis> prevHyp_;
  const size_t beamHypIdx_; // EXPERIMENTAL, UNUSED
  const size_t prevBeamHypIdx_;
  const Word word_;
  const float pathScore_;

  std::vector<float> scoreBreakdown_; // [num scorers]
  std::vector<float> alignment_;
};

typedef std::vector<Ptr<Hypothesis>> Beam;                // Beam = vector [beamSize] of hypotheses
typedef std::vector<Beam> Beams;                          // Beams = vector [batchDim] of vector [beamSize] of hypotheses
typedef std::tuple<Words, Ptr<Hypothesis>, float> Result; // (word ids for hyp, hyp, normalized sentence score for hyp)
typedef std::vector<Result> NBestList;                    // sorted vector of (word ids, hyp, sent score) tuples
}  // namespace marian
