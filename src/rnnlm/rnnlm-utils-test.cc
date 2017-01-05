// rnnlm/rnnlm-utils-test.cc

#include <math.h>
#include "rnnlm/rnnlm-utils.h"

namespace kaldi {
namespace rnnlm {

void PrepareVector(int n, int ones_size, std::set<int> &outputs_set, vector<BaseFloat> &selection_probs) {
  BaseFloat prob = 0;
  BaseFloat prob_sum = 0;
  for (int i = 0; i < n; i++) {
    prob = RandUniform();
    prob_sum += prob;
    selection_probs.push_back(prob);
  }
  for (int i = 0; i < n; i++) {
    selection_probs[i] /= prob_sum;
  }
  for (int i = 0; i < ones_size; i++) {
    outputs_set.insert(rand() % n);
  }
}

void UnitTestNChooseKSamplingConvergence(int n, int k, int ones_size) {
  std::set<int> outputs_set;
  vector<BaseFloat> selection_probs;
  PrepareVector(n, ones_size, outputs_set, selection_probs);
  NormalizeVec(k, outputs_set, &selection_probs);

  vector<std::pair<int, BaseFloat> > u(selection_probs.size());
  for (int i = 0; i < u.size(); i++) {
    u[i].first = i;
    u[i].second = selection_probs[i];
  }
  // normalize the selection_probs
  BaseFloat sum = 0;
  for (int i = 0; i < u.size(); i++) {
    sum += selection_probs[i];
  }
  for (int i = 0; i < u.size(); i++) {
    selection_probs[i] /= sum;
  }

  vector<BaseFloat> samples_probs(u.size(), 0);
  int count = 0;
  for (int i = 0; ; i++) {
    count++;
    vector<int> samples;
    SampleWithoutReplacement(u, k, &samples);
    for (int j = 0; j < samples.size(); j++) {
      samples_probs[samples[j]] += 1;
    }
    for (int j = 0; j < samples_probs.size(); j++) {
      samples_probs[j] /= (count * k);
    }
    // update Euclidean distance between the two pdfs every 1000 iters
    BaseFloat distance = 0;
    if (count % 1000 == 0) {
      for (int j = 0; j < u.size(); j++) {
        distance += pow(samples_probs[j] - selection_probs[j], 2);
      }
      distance = sqrt(distance);
    }
    // if the Euclidean distance is small enough, break the loop
    if (distance < 0.05) {
      KALDI_LOG << "Sampling convergence test: passed for sampling " << k <<\
        " items from " << n << " unigrams";
      break;;
    }
  }
}

void UnitTestSamplingConvergence() {
  // number of unigrams 
  int n = rand() % 10000 + 100;
  // sample size
  int k;
  // number of ones
  int ones_size;
  ones_size = rand() % (n / 2);
  k = rand() % (n - ones_size) + ones_size + 1;
  UnitTestNChooseKSamplingConvergence(n, k, ones_size);
  // test when k = 1
  k = 1;
  ones_size = 0;
  UnitTestNChooseKSamplingConvergence(n, k, ones_size);
  // test when k = 2
  k = 2;
  ones_size = rand() % (k - 1);
  UnitTestNChooseKSamplingConvergence(n, k, ones_size);
  // test when k = n 
  ones_size = rand() % (n / 2);
  k = n;
  UnitTestNChooseKSamplingConvergence(n, k, ones_size);
}

// test that probabilities 1.0 are always sampled
void UnitTestSampleWithProbOne(int iters) {
  // number of unigrams 
  int n = rand() % 1000 + 100;
  // generate a outputs_set with ones
  int ones_size = rand() % (n / 2);
  std::set<int> outputs_set;
  vector<BaseFloat> selection_probs;

  PrepareVector(n, ones_size, outputs_set, selection_probs);

  // generate a random number k from ones_size + 1 to n
  int k = rand() % (n - ones_size) + ones_size + 1;
  NormalizeVec(k, outputs_set, &selection_probs);

  vector<std::pair<int, BaseFloat> > u(selection_probs.size());
  for (int i = 0; i < u.size(); i++) {
    u[i].first = i;
    u[i].second = selection_probs[i];
  }

  int N = iters;
  for (int i = 0; i < N; i++) {
    vector<int> samples;
    SampleWithoutReplacement(u, k, &samples);
    if (outputs_set.size() > 0) {
      // assert every item in outputs_set is sampled
      for (set<int>::iterator it = outputs_set.begin(); it != outputs_set.end(); ++it) {
        KALDI_ASSERT(std::find(samples.begin(), samples.end(), *it) != \
            samples.end());
      }
    }
  }
}

void UnitTestSamplingTime(int iters) {
  // number of unigrams 
  int n = rand() % 1000 + 100;
  // generate a outputs_set with ones
  int ones_size = rand() % (n / 2);
  std::set<int> outputs_set;
  vector<BaseFloat> selection_probs;

  PrepareVector(n, ones_size, outputs_set, selection_probs);

  // generate a random number k from ones_size + 1 to n
  int k = rand() % (n - ones_size) + ones_size + 1;
  NormalizeVec(k, outputs_set, &selection_probs);

  vector<std::pair<int, BaseFloat> > u(selection_probs.size());
  for (int i = 0; i < u.size(); i++) {
    u[i].first = i;
    u[i].second = selection_probs[i];
  }

  int N = iters;
  Timer t;
  t.Reset();
  BaseFloat total_time;
  for (int i = 0; i < N; i++) {
    vector<int> samples;
    SampleWithoutReplacement(u, k, &samples);
  }
  total_time = t.Elapsed();
  KALDI_LOG << "Time test: Sampling " << k << " items from " << n << " unigrams for " << N << \
    " times takes " << total_time << " totally.";
}

}  // end namespace rnnlm
}  // end namespace kaldi.

int main() {
  using namespace kaldi;
  using namespace rnnlm;
  int N = 10000;
  UnitTestSampleWithProbOne(N);
  UnitTestSamplingTime(N);
  UnitTestSamplingConvergence();
}

