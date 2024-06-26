//===========================================================================
/*!
 * 
 *
 * \brief       Random Forest Trainer
 * 
 * 
 *
 * \author      K. N. Hansen, J. Kremer
 * \date        2011-2012
 *
 *
 * \par Copyright 1995-2017 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <https://shark-ml.github.io/Shark/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
//===========================================================================


#ifndef SHARK_ALGORITHMS_TRAINERS_RFTRAINER_H
#define SHARK_ALGORITHMS_TRAINERS_RFTRAINER_H

#include <shark/Algorithms/Trainers/AbstractWeightedTrainer.h>
#include <shark/Models/Trees/RFClassifier.h>
#include <shark/Algorithms/Trainers/Impl/CART.h>

#include <vector>
#include <limits>

namespace shark {

/// \brief Random Forest
///
/// Random Forest is an ensemble learner, that builds multiple binary decision trees.
/// The trees are built using a variant of the CART methodology
///
/// Typically 100+ trees are built, and classification/regression is done by combining
/// the results generated by each tree. Typically the a majority vote is used in the
/// classification case, and the mean is used in the regression case
///
/// Each tree is built based on a random subset of the total dataset. Furthermore
/// at each split, only a random subset of the attributes are investigated for
/// the best split
///
/// The node impurity is measured by the Gini criteria in the classification
/// case, and the total sum of squared errors in the regression case
///
/// After growing a maximum sized tree, the tree is added to the ensemble
/// without pruning.
///
/// For detailed information about Random Forest, see Random Forest
/// by L. Breiman et al. 2001.
/// \ingroup supervised_trainer


template<class LabelType>
class RFTrainer;

template<>
class RFTrainer<unsigned int>
: public AbstractWeightedTrainer<RFClassifier<unsigned int> >, public IParameterizable<RealVector>
{
public:
	/// Construct and compute feature importances when training or not
	RFTrainer(bool computeFeatureImportances = false, bool computeOOBerror = false){
		m_computeFeatureImportances = computeFeatureImportances;
		m_computeOOBerror = computeOOBerror;
		m_numTrees = 100; 
		m_min_samples_leaf = 1; 
		m_min_split = 2 * m_min_samples_leaf; 
		m_max_depth = 10000;
		m_min_impurity_split = 1e-10; 
		m_epsilon = 1e-10;
		m_max_features = 0;
	}

	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "RFTrainer"; }

	/// Set the number of random attributes to investigate at each node.
	///
	/// Defualt is 0 which is translated to sqrt(inputDim(data)) during training
	void setMTry(std::size_t mtry) { m_max_features = mtry; }

	/// Set the number of trees to grow. (default 100)
	void setNTrees(std::size_t numTrees) {m_numTrees = numTrees;}
	
	/// Set Minimum number of samples that is split (default 2)
	void setMinSplit(std::size_t numSamples) {m_min_split = numSamples;}

	/// Set Maximum depth of the tree (default 10000)
	void setMaxDepth(std::size_t maxDepth) {m_max_depth = maxDepth;}
	
	/// Controls when a node is considered pure. If set to 1, a node is pure
	/// when it only consists of a single node.(default 1)
	void setNodeSize(std::size_t nodeSize) { m_min_samples_leaf = nodeSize; }

	/// The minimum impurity below which a a node is considere pure (default 1.e-10)
	void minImpurity(double impurity) {m_min_impurity_split = impurity;}
	
	/// The minimum dtsnace of features to be considered different (detault 1.e-10)
	void epsilon(double distance) {m_epsilon = distance;}
	
	/// Return the parameter vector.
	RealVector parameterVector() const{return RealVector();}

	/// Set the parameter vector.
	void setParameterVector(RealVector const& newParameters){
		SHARK_ASSERT(newParameters.size() == 0);
	}
	
	
	/// Train a random forest for classification.
	using AbstractWeightedTrainer<RFClassifier<unsigned int> >::train;
	void train(RFClassifier<LabelType>& model, WeightedLabeledData<RealVector,LabelType> const& dataset){
		model.clearModels();
		
		//setup treebuilder
		CART::TreeBuilder<unsigned int,CART::ClassificationCriterion> builder;
		builder.m_min_samples_leaf = m_min_samples_leaf;
		builder.m_min_split = m_min_split;
		builder.m_max_depth = m_max_depth;
		builder.m_min_impurity_split = m_min_impurity_split;
		builder.m_epsilon = m_epsilon;
		builder.m_max_features = m_max_features? m_max_features: std::sqrt(inputDimension(dataset));
		
		//copy data into single batch for easier lookup
		blas::matrix<double, blas::column_major> data_train = createBatch<RealVector>(dataset.inputs().elements().begin(),dataset.inputs().elements().end());
		auto labels_train = createBatch<LabelType>(dataset.labels().elements().begin(),dataset.labels().elements().end());
		auto weights_train = createBatch<double>(dataset.weights().elements().begin(),dataset.weights().elements().end());

		//Setup seeds for the rng in the different threads
		std::vector<unsigned int> seeds(m_numTrees);
		for (auto& seed: seeds) {
			seed = random::discrete(random::globalRng, 0u,std::numeric_limits<unsigned int>::max());
		}
		
		std::vector<std::vector<std::size_t> > complements;

		//Generate trees
		SHARK_PARALLEL_FOR(int t = 0; t < m_numTrees; ++t){
			random::rng_type rng(seeds[t]);
			
			//Setup data for this tree
			CART::Bootstrap<blas::matrix<double, blas::column_major>, UIntVector> bootstrap(rng, data_train,labels_train, weights_train);
			auto const& tree = builder.buildTree(rng, bootstrap);
			
			SHARK_CRITICAL_REGION{
				model.addModel(tree);
				complements.push_back(std::move(bootstrap.complement));
			}
		}
		
		if(m_computeOOBerror)
			model.computeOOBerror(complements, dataset.data());
		
		if(m_computeFeatureImportances)
			model.computeFeatureImportances(complements,dataset.data(), random::globalRng);
	}
	
	
private:
	bool m_computeFeatureImportances;///< set true if the feature importances should be computed
	bool m_computeOOBerror;///< set true if OOB error should be computed

	long m_numTrees; ///< number of trees in the forest
	std::size_t m_max_features;///< number of attributes to randomly test at each inner node
	std::size_t m_min_samples_leaf; ///< minimum number of samples in a leaf node
	std::size_t m_min_split; ///< minimum number of samples to be considered a split
	std::size_t m_max_depth;///< maximum depth of the tree
	double m_epsilon;///< Minimum difference between two values to be considered different
	double m_min_impurity_split;///< stops splitting when the impority is below a threshold
};


template<>
class RFTrainer<RealVector>
: public AbstractWeightedTrainer<RFClassifier<RealVector> >, public IParameterizable<RealVector>
{
public:
	/// Construct and compute feature importances when training or not
	RFTrainer(bool computeFeatureImportances = false, bool computeOOBerror = false){
		m_computeFeatureImportances = computeFeatureImportances;
		m_computeOOBerror = computeOOBerror;
		m_numTrees = 100; 
		m_min_samples_leaf = 1; 
		m_min_split = 2 * m_min_samples_leaf; 
		m_max_depth = 10000;
		m_min_impurity_split = 1e-10; 
		m_epsilon = 1e-10;
		m_max_features = 0;
	}

	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "RFTrainer"; }

	/// Set the number of random attributes to investigate at each node.
	///
	/// Defualt is 0 which is translated to inputDim(data)/3 during training
	void setMTry(std::size_t mtry) { m_max_features = mtry; }

	/// Set the number of trees to grow. (default 100)
	void setNTrees(std::size_t numTrees) {m_numTrees = numTrees;}
	
	/// Set Minimum number of samples that is split (default 10)
	void setMinSplit(std::size_t numSamples) {m_min_split = numSamples;}

	/// Set Maximum depth of the tree (default 10000)
	void setMaxDepth(std::size_t maxDepth) {m_max_depth = maxDepth;}
	
	/// Controls when a node is considered pure. If set to 1, a node is pure
	/// when it only consists of a single node.(default 5)
	void setNodeSize(std::size_t nodeSize) { m_min_samples_leaf = nodeSize; }

	/// The minimum impurity below which a a node is considere pure (default 1.e-10)
	void minImpurity(double impurity) {m_min_impurity_split = impurity;}
	
	/// The minimum dtsnace of features to be considered different (detault 1.e-10)
	void epsilon(double distance) {m_epsilon = distance;}
	
	/// Return the parameter vector.
	RealVector parameterVector() const{ return RealVector();}

	/// Set the parameter vector.
	void setParameterVector(RealVector const& newParameters){
		SHARK_ASSERT(newParameters.size() == 0);
	}
	
	
	/// Train a random forest for classification.
	void train(RFClassifier<LabelType>& model, WeightedLabeledData<RealVector,LabelType> const& dataset){
		model.clearModels();
		//setup treebuilder
		CART::TreeBuilder<RealVector,CART::MSECriterion> builder;
		builder.m_min_samples_leaf = m_min_samples_leaf;
		builder.m_min_split = m_min_split;
		builder.m_max_depth = m_max_depth;
		builder.m_min_impurity_split = m_min_impurity_split;
		builder.m_epsilon = m_epsilon;
		builder.m_max_features = m_max_features? m_max_features: inputDimension(dataset)/3;
		//copy data into single batch for easier lookup
		blas::matrix<double, blas::column_major> data_train = createBatch<RealVector>(dataset.inputs().elements().begin(),dataset.inputs().elements().end());
		auto labels_train = createBatch<LabelType>(dataset.labels().elements().begin(),dataset.labels().elements().end());
		auto weights_train = createBatch<double>(dataset.weights().elements().begin(),dataset.weights().elements().end());
		
		//Setup seeds for the rng in the different threads
		std::vector<unsigned int> seeds(m_numTrees);
		for (auto& seed: seeds) {
			seed = random::discrete(random::globalRng, 0u,std::numeric_limits<unsigned int>::max());
		}
		
		std::vector<std::vector<std::size_t> > complements;

		//Generate trees
		SHARK_PARALLEL_FOR(int t = 0; t < m_numTrees; ++t){
			random::rng_type rng{seeds[t]};
			
			//Setup data for this tree
			CART::Bootstrap<blas::matrix<double, blas::column_major>, RealMatrix> bootstrap(rng, data_train,labels_train, weights_train);
			auto const& tree = builder.buildTree(rng, bootstrap);
			
			SHARK_CRITICAL_REGION{
				model.addModel(tree);
				complements.push_back(std::move(bootstrap.complement));
			}
		}
		
		if(m_computeOOBerror)
			model.computeOOBerror(complements,dataset.data());
		
		if(m_computeFeatureImportances)
			model.computeFeatureImportances(complements,dataset.data(), random::globalRng);
	}
	
	
private:
	bool m_computeFeatureImportances;///< set true if the feature importances should be computed
	bool m_computeOOBerror;///< set true if OOB error should be computed

	long m_numTrees; ///< number of trees in the forest
	std::size_t m_max_features;///< number of attributes to randomly test at each inner node
	std::size_t m_min_samples_leaf; ///< minimum number of samples in a leaf node
	std::size_t m_min_split; ///< minimum number of samples to be considered a split
	std::size_t m_max_depth;///< maximum depth of the tree
	double m_epsilon;///< Minimum difference between two values to be considered different
	double m_min_impurity_split;///< stops splitting when the impority is below a threshold
};


}
#endif
