Normalization of Input Data
===========================

This short tutorial will demonstrate how data can be normalized using
Shark. Read the basic :ref:`data tutorials <label_for_data_tutorials>`
first if you are not familiar with the :doxy:`Data` containers.

In this tutorial the term *data normalization* refers to the process of
pre-processing feature values of a data set. The usual purpose is to
make all features comparable, in a sense, by transforming them to a
uniform scale.

Shark distinguished between feature-wise normalization and more complex
methods.


Feature-wise Normalization
--------------------------

For feature-wise rescaling Shark applies a :doxy:`Normalizer`
model to the data. This model applies an affine linear function
:math:`x \mapsto a x + b` to each feature component; this is a special
type of a linear model. Two different trainers for two different types
of normalization are available. The trainers are
:doxy:`NormalizeComponentsUnitInterval` and
:doxy:`NormalizeComponentsUnitVariance`. The first one normalizes
every input dimension to the range [0,1]. This operation is meaningful
if it is known that feature values are bounded. The second class adjusts
the variance of each component to one, and it can optionally remove the
mean. This is no whitening, because correlations remain unchanged.
Removal of the mean is optional. Note that without removal of the mean
(essentially :math:`x \mapsto a x` with :math:`b = 0`) this operation
can be applied efficiently even to extremely high-dimensional sparse
feature vectors.

In the following we will normalize data to unit variance and remove the
mean. This requires the includes::


	#include <shark/Models/Normalizer.h>
	#include <shark/Algorithms/Trainers/NormalizeComponentsUnitVariance.h>
	using namespace shark;
	

First we have to train our normalizer model so that it can perform
the normalization operation::


		// create and train data normalizer
		bool removeMean = true;
		Normalizer<RealVector> normalizer;
		NormalizeComponentsUnitVariance<RealVector> normalizingTrainer(removeMean);
		normalizingTrainer.train(normalizer, data);
	

Now the normalizer is ready to use and we can transform the data::


		// transform data
		UnlabeledData<RealVector> normalizedData = transform(data, normalizer);
	

In order to apply such a normalization to :doxy:`LabeledData`, the
methods :doxy:`transformInputs` and :doxy:`transformLabels` can be used.
Of course, training and test data can be normalized either independently
or with the same model.

A normalizer can be concatenated with another model. This comes handy
when a model should handle a stream of new input data. Only one call is
needed to use the normalization followed by the trained model::

  #include<shark/Models/ConcatenatedModel.h>
  //...

  YourModel model;
  ConcatenatedModel<RealVector,RealVector> completeModel = normalizer >> model;

Whitening
---------

As noted above the feature-wise rescaling to unit variance does not
remove correlations between features. A full whitening of the data is
requires for this purpose. The resulting transformation is represented
by a :doxy:`LinearModel`, which is trained by a
:doxy:`NormalizeComponentsWhitening` trainer. The following code is
analog of the above feature-wise normalization::


	#include <shark/Models/LinearModel.h>
	#include <shark/Algorithms/Trainers/NormalizeComponentsWhitening.h>
	


		// create and train data normalizer
		LinearModel<RealVector> whitener;
		NormalizeComponentsWhitening whiteningTrainer;
		whiteningTrainer.train(whitener, data);
	

Now the normalizer is ready to use and we can transform the data::


		// transform data
		UnlabeledData<RealVector> whitenedData = transform(data, whitener);
	

A few notes are in place. First of all note that different types of
pre-processing steps are suitable for different types of data. Second,
not all procedures scale well to high-dimensional feature spaces. Third,
it usually makes sense to combine feature rescaling with a feature
selection (e.g., filtering) technique. Such techniques are beyond the
scope of this tutorial.
