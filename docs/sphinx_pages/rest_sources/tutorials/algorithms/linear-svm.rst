==============================
Linear Support Vector Machines
==============================

Support vector machines (SVMs) are traditionally viewed as a
kernel-based learning method. However, flexible non-linear models are
not always needed, and in particular for huge scale feature spaces
(e.g., in text mining and bioinformatics) linear models are sufficiently
rich. The decisive advantage of linear SVMs is that they can be trained
significantly faster than kernel-based models. Coordinate Descent (CD)
training of the dual problem is per iteration faster by a factor up to
the number of data points [HCLKS-2008]_, which can make a big difference.
The algorithms implemented in Shark closely follow [GU-2013]_.

Shark provides various trainers for linear SVMs. These are mostly
analogous to the non-linear case. Therefore you may wish to read the
tutorial on SVMs first if you have not yet done so: :doc:`svm`

As usual we start with the necessary includes ::


	#include <shark/Data/Dataset.h>
	#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h>
	#include <shark/Algorithms/Trainers/CSvmTrainer.h>
	

	#include <shark/Algorithms/Trainers/CSvmTrainer.h>
	

where the last line is one possible choice for multi-class problems,
see below.

For a linear SVM to be applicable the input components of the data need
to be vector valued. We consider one of the following, depending on
whether inputs are sparse or not::


		typedef RealVector VectorType;
		// or:
		// typedef CompressedRealVector VectorType;
	

With this definition in place we instanciate a linear classifier model::


		LinearClassifier<VectorType> model;
	

mapping inputs to `unsigned int` class labels. This is just a standard
:doxy:`LinearModel`, whose outputs are converted to a class label
with an :doxy:`Classifier`. So it computes one linear function per class 
and predicts the class that got the highest score (a single value is 
computed and thresholded at zero in case of only two classes).


Machine Training
----------------

Given a classification data set ::


		LabeledData<VectorType, unsigned int> training;
	

and a regulariztion constant C > 0 we can train a linear SVM.
Assuming binary labels the default is to use a C-SVM: ::


		LinearCSvmTrainer<VectorType> trainer(C, false);
	

		trainer.train(model, training);
	

A major difference to the non-linear case is that we do not need to
define a kernel (and thus there is no need to tune kernel parameters).
Of course we can make predictions and evaluate the model with a loss
function in the usual way: ::


		ZeroOneLoss<unsigned int> loss;
		Data<unsigned int> output = model(training.inputs());
		double train_error = loss.eval(training.labels(), output);
		cout << "training error:\t" <<  train_error << endl;
	

For SVMs the case of more than two classes is very different from the
binary classification setting. For this so-called multi-class case there
is a broad variety of methods available, varying mostly in the loss
employed for training. The one-versus-all (OVA) method is a strong
baseline: ::


		LinearCSvmTrainer<VectorType> trainer(C, epsilon);
		trainer.setMcSvmType(McSvm::OVA);
	

Other possibilities are the machines proposed by Weston and Watkins
(`McSvm::WW`), Crammer and Singer (`McSvm::CS`),
Lee, Lin and Wahba (`McSvm::LLW`), and many more
(`McSvm::ReinforcedSvm`, `McSvm::MMR`, `McSvm::ADM`,
`McSvm::ATM`, and `McSvm::ATS`).

Being left with this much choice is probably not helpful. In our
experience the best strategy for picking a machine is to try WW and CS
first, and LLW and ATS for high-dimensional feature spaces. OVA can be
useful at times, and MMR, ADM and ATM can usually be ignored. The
default choice by the Trainer is WW.

Currently Shark does not provide a specialized trainer for linear SVM
regression. Of course the non-linear :doxy:`EpsilonSvmTrainer` can be
used with a :doxy:`LinearKernel` for this purpose.


References
----------

.. [HCLKS-2008] C. J. Hsieh, K. W. Chang, C. J. Lin, S. S. Keerthi, and S. Sundararajan.
	A dual coordinate descent method for large-scale linear SVM.
	In Proceedings of the 30th International Conference on Machine learning (ICML), 2008.

.. [GU-2013] T. Glasmachers and Ü. Dogan.
   Accelerated Coordinate Descent with Adaptive Coordinate Frequencies.
   In Proceedings of the fifth Asian Conference on Machine Learning (ACML), 2013.
