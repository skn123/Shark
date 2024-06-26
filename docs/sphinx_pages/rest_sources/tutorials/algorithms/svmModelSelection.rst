===============================================================================
Support Vector Machines: Model Selection Using Cross-Validation and Grid-Search
===============================================================================

Please read the :doc:`svm` tutorial first to follow the SVM example.
However, the part on cross-validation and grid-search works of course
also for other classifiers.

The performance of your SVM classifier depends on the choice of the
regularization parameter :math:`C` and the kernel parameters.
For a standard radial Gaussian kernel

.. math ::
  k(x, z) = \exp(-\gamma \|x- z\|^2) = \exp( - \|x- z\|^2 / (2\sigma^2))

the bandwidth parameter :math:`\gamma` (or :math:`\sigma`) is the
only kernel parameter.  Adapting the "hyperparameters" is referred
to as SVM model selection.


The Shark library offers many algorithms for SVM model selection.
In this tutorial, we consider the most basic approach.



Cross-validation
----------------


Cross-validation (CV) is a standard technique for adjusting
hyperparameters of predictive models.  In K-fold CV, the available
data :math:`S` is partitioned into K subsets :math:`S_1,\dots,
S_K`. Each data point in :math:`S` is randomly assigned to one of the
subsets such that these are of almost equal size (i.e., :math:`\lfloor
|S|/K\rfloor \leq |S_i|\leq \lceil |S|/K\rceil`).  Further, we define
:math:`S_{\setminus i}=\bigcup_{j=1,\dots,K \wedge j\neq i} S_i` as
the union of all data points except those in :math:`S_i`.  For each
:math:`i=1,\dots,K`, an individual model is built by applying the
algorithm to the training data :math:`S_{\setminus i}`. This model is
then evaluated by means of a cost function using the test data in
:math:`S_i`. The average of the K outcomes of the model evaluations is
called *cross-validation (test) performance* or
*cross-validation (test) error* and is used a predictor of the
performance of the algorithm when applied to :math:`S`.  Typical
values for K are 5 and 10 [HastieTibshiraniFriedman-2008]_.

To choose :math:`C` and :math:`\gamma` using K-fold CV, we first split
the available data into K subsets.  Then we compute the CV error using
this split error for the SVM classifiers using different values for
:math:`C` and :math:`\gamma`.  Finally, we pick the :math:`C` and
:math:`\gamma` with the lowest CV error and use it for training an SVM
on the complete data set :math:`S`.



Jaakkola's heuristic
--------------------

Jaakkola's heuristic [JaakkolaDiekhausHaussler1999]_ provides a reasonable initial guess for the
bandwidth parameter :math:`\gamma` or :math:`\sigma` of a Gaussian
kernel. 

To estimate a good value for :math:`\sigma`, consider all pairs
consisting of an training input vector from the positive class and a
training input vector from the negative class.  Compute the difference
in input space between all pairs.  The median of these distances can
be used as a measure of scale and therefore as a guess for :math:`\sigma`.
More formally, compute

.. math ::
  G=\{  \|x_i - x_j\|\,|\, (x_i, y_i), (x_j,y_j)\in S \wedge y_i\neq y_j\}

based on your training data :math:`S`.
Then set  :math:`\sigma_{\text{Jaakkola}}` equal to the median of the values
in :math:`G`:

.. math ::
  \sigma_{\text{Jaakkola}} = \operatorname{median}(G)



SVM Model Selection in Shark
----------------------------

We consider the same toy problem and the same models as in the tutorial
:doc:`svm`. We additionally include::


	#include <shark/ObjectiveFunctions/CrossValidationError.h>
	#include <shark/Algorithms/DirectSearch/GridSearch.h>
	#include <shark/Algorithms/JaakkolaHeuristic.h>
	

for computing the cross-validation error, for calculating Jaakkola's
Heuristic, and for optimizing the parameters using grid-search,
respectively.


Preparing the SVM for unconstraint optimization
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Our model and trainer are now given by::


		GaussianRbfKernel<> kernel(0.5, true); //unconstrained?
		KernelClassifier<RealVector> svm;
		bool offset = true;
		bool unconstrained = true;
		CSvmTrainer<RealVector> trainer(&kernel, 1.0, offset,unconstrained);
		

The Boolean flags set to true in the constructors of
:doxy:`GaussianRbfKernel` and :doxy:`CSvmTrainer` indicate that the
kernel parameter :math:`\gamma` and the regularization parameter
:math:`C`, which "belongs" to the trainer, are *internally* encoded as
:math:`\ln \gamma` and :math:`\ln C`.  Because both parameters have to
be positive, this encoding allows for unconstraint optimization (e.g.,
if the model parameter encoding the kernel width is set to -1, we have
:math:`\gamma =1/e`).  This encoding affects the interface between
model, objective function, and optimizer, but *not* functions such as
``setGamma``, ``setSigma`` or ``setC``. These behave always the same
regardless of the internal encoding.


Cross-validation
^^^^^^^^^^^^^^^^

Now we define the 
cross-validation error. First let us define the folds we need :: 


		const unsigned int K = 5;  // number of folds
		ZeroOneLoss<unsigned int> loss;
		CVFolds<ClassificationDataset> folds = createCVSameSizeBalanced(dataTrain, K);
		

The first line sets the number of folds :math:`K` to 5. Then we define the
basic error measure underlying the cross-validation error, here the
standard 0-1 loss. Note that we do classification here, so we need
to use unsigned int as the template parameter for the 0-1 loss. 
After that we split the available training data
into :math:`K` folds using the function :doxy:`createCVSameSizeBalanced` from
``Data/CVDatasetTools.h``.  

Then we need to instantiate :doxy:`CrossValidationError`:: 


		CrossValidationError<KernelClassifier<RealVector>, unsigned int> cvError(
			folds, &trainer, &svm, &trainer, &loss
		);
		

The template arguments of
:doxy:`CrossValidationError` specify the model and that the
given labels are unsigned integers (encoding classes).  The first parameter of the 
constructor is just the data in form of the folds we defined. 
The last parameter is the loss function on which the CV error
is based. But what about the
other three parameters?  The :doxy:`CrossValidationError` works as follows.
A new parameter configuration is written into an "meta" object *A*
that is :doxy:`IParameterizable` (such as a regularizer or a trainer, in
our case the trainer we defined earlier).
Then the specified model *B* is trained with the specified trainer
*C*.  The pointers to *A*, *B*, and *C* are the arguments 2, 3, and 4
of the constructor.  In our case of SVM model selection, the meta
object and the trainer are the same, this is why the trainer 
occurs twice as a parameter.




Jaakkola's heuristic
^^^^^^^^^^^^^^^^^^^^

To find a good starting point for :math:`\gamma`, we apply Jaakkola's heuristic ::


		JaakkolaHeuristic ja(dataTrain);
		double ljg = log(ja.gamma());
		

as defined above.

Grid-search
^^^^^^^^^^^

We have two hyperparameters.
To adapt them using grid-search, we have to define a two-dimensional
grid. Let us consider 9 grid points for 
:math:`\ln \gamma` and 11 for :math:`\ln C`.
Let 

.. math ::
  \ln \gamma\in\{\ln\gamma_{\text{Jaakkola}}-4, \ln\gamma_{\text{Jaakkola}}-3,\dots,\ln\gamma_{\text{Jaakkola}},\dots,\ln\gamma_{\text{Jaakkola}}+4\}

and  

.. math ::
   \ln C\in\{0,1,\dots,10\} .


and define the grid accordingly: ::


		GridSearch grid;
		vector<double> min(2);
		vector<double> max(2);
		vector<size_t> sections(2);
		// kernel parameter gamma
		min[0] = ljg-4.; max[0] = ljg+4; sections[0] = 9;
		// regularization parameter C
		min[1] = 0.0; max[1] = 10.0; sections[1] = 11;
		grid.configure(min, max, sections);
		

The optimizer :doxy:`GridSearch` "sees" the parameter in the
logarithmic encoding we activated in the model and trainier definition
above. Therefore, we specify a linear grid while searching on
logarithmic scale. Now we do the grid search by ::


		grid.step(cvError);
		

and finally train the model using all data and the best parameters ::


		trainer.setParameterVector(grid.solution().point);
		trainer.train(svm, dataTrain);
		

and evaluate the model as described in :doc:`svm`.



Full example program
--------------------

The full example program for tutorial is :doxy:`CSvmGridSearchTutorial.cpp`. 


References
----------


.. [HastieTibshiraniFriedman-2008] T. Hastie, R. Tibshirani and
   J. Friedman.  `The Elements of Statistical Learning
   <http://www-stat.stanford.edu/~tibs/ElemStatLearn>`_, section 4.3. Springer-Verlag,
   2008.

.. [JaakkolaDiekhausHaussler1999] T. Jaakkola, M. Diekhaus, and D. Haussler. Using the Fisher kernel method to detect remote protein homologies. In T. Lengauer, R. Schneider, P. Bork, D. Brutlad, J. Glasgow, H.- W. Mewes, and R. Zimmer, editors, Proceedings of the Seventh International Conference on Intelligent Systems for Molecular Biology, pages 149-158. AAAI Press, 1999.

