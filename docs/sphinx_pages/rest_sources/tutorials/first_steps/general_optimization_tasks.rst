

General Optimization Tasks
==========================

Introduction
------------
In the previous tutorial we used the LDA trainer to solve the simple LDA
optimization task in one shot. In Shark, a trainer implements a solution
strategy for a standard problems, often providing an analytic solution in a single step.
Given both data and a model, a trainer finds the solution for its particular task.
To define our own task we can use the general purpose framework of shark.
We have already seen one part of this general framework, the
model. A model is parameterized by a set of real values which define its
behavior. In many simple cases, a model represents a function, mapping inputs to
outputs, chosen from a parametric family (e.g., the family of linear functions).


Regression
----------
In this tutorial, we will focus on a simple regression task. In regression,
the parameters of a model are chosen such that it fits the training data as well
as possible, usually in a least-squares sense. After its parameters have been
found, the model can be used to predict the output values of new input points.
The Shark model also provides a way to calculate various derivatives. The
code for this example can be found in
:doxy:`regressionTutorial.cpp <regressionTutorial.cpp>`.

Linear regression can be solved analytically. As before, this can be done by the
trainer class ``LinearRegression`` which is demonstrated in the tutorial on
:doc:`../algorithms/linearRegression`. 

Data preparation
%%%%%%%%%%%%%%%%
For this tutorial, we need the following include files::


	#include <shark/Data/Csv.h>
	#include <shark/Algorithms/GradientDescent/CG.h>
	#include <shark/ObjectiveFunctions/ErrorFunction.h>
	#include <shark/ObjectiveFunctions/Loss/SquaredLoss.h>
	#include <shark/Models/LinearModel.h>
	

We first write a short function which automates the data loading procedure.
In contrast to the last tutorial, this time we load a supervised learning data
set from two files. The first one stores input points, the other the corresponding
outputs. In the next step we bind the two loaded data items together to create
our ``RegressionDataset`` (which again is a simple typedef for ``LabeledData<RealVector, RealVector>``)::


	RegressionDataset loadData(const std::string& dataFile,const std::string& labelFile){
		//we first load two separate data files for the training inputs and the labels of the data point
		Data<RealVector> inputs;
		Data<RealVector> labels;
		try {
			importCSV(inputs, dataFile, ' ');
			importCSV(labels, labelFile, ' ');
		} catch (...) {
			cerr << "Unable to open file " <<  dataFile << " and/or " << labelFile << ". Check paths!" << endl;
			exit(EXIT_FAILURE);
		}
		//now we create a complete dataset which represents pairs of inputs and labels
		RegressionDataset data(inputs, labels);
		return data;
	}
	

Now we can load the data using this function, and then create a training and test set
from it. We will again use 80% for training and 20% for testing. As in the previous
tutorial, we call  ``splitAtElement``, which splits the last part from our dataset
into the test set. Our original data set from then on only contains the training data::


		RegressionDataset data = loadData("data/regressionInputs.csv","data/regressionLabels.csv");
		RegressionDataset test = splitAtElement(data,static_cast<std::size_t>(0.8*data.numberOfElements()));
		


Setting up the model
%%%%%%%%%%%%%%%%%%%%
In this example, we want to do linear regression. So what we first need
is a linear model. Since we are not using a trainer which nicely sets up the model
for us, we have to configure it for the task. This mainly includes initializing
it with the proper input and output sizes. In our example data set, both
dimensions have size 1. But since this changes with every data set, we just ask
the dataset for the correct values: ::


		LinearModel<> model(inputDimension(data), labelDimension(data));
		

The first argument is the dimensionality, of the data points and the second parameter
that of the outputs.

An affine linear model is a model of the form

.. math::
   f(x) = Ax+b

where :math:`x` is the input, and the matrix :math:`A` and vector
:math:`b` are constant. Thus, the parameters of the model are the
entries of :math:`A` and :math:`b`. Since we only have one output
dimension and one input dimension, this means that our model has two
parameters in total. In the general case, it has :math:`n*m+m`
parameters where :math:`n` is the input dimension and :math:`m` the
output dimension of the model.



Setting up the objective function
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

In order to optimize the model defined above, we need to set up some criterion.
To assess the performance, it applies a loss function to each data point. For linear regression,
we want to use the mean squared error, so we use the :doxy:`SquaredLoss` class.
This loss function computes the squared difference

.. math ::
   L(f(x), y) = (f(x) - y)^2

between the model output :math:`f(x)` and the training label :math:`y`. 
We combine dataset, model and loss function using an :doxy:`ErrorFunction`::


		SquaredLoss<> loss;
		ErrorFunction<> errorFunction(data, &model,&loss);
		

The ``ErrorFunction`` is an instance of ``AbstractObjectiveFunction`` -- see the
concept tutorial on :doc:`../concepts/library_design/objective_functions`. It computes the average
loss of the model on the supplied dataset, i.e. the mean squared error.

Optimization
%%%%%%%%%%%%

To optimize the above instantiated model under the above defined objective function
``ErrorFunction``, we need an optimizer. For our regression task, a conjugate gradient
method is just fine and we run it for 100 iterations::


		CG<> optimizer;
		errorFunction.init();
		optimizer.init(errorFunction);
		for(int i = 0; i != 100; ++i)
		{
			optimizer.step(errorFunction);
		}
		//copy solution parameters into model
		model.setParameterVector(optimizer.solution().point);
		

Evaluation
%%%%%%%%%%


Again, we want to evaluate the model on a test set and print all results. We could
re-use ``errorFunction`` for this by changing the dataset to the test set, but often
it is more convenient to use the loss directly. We let the model evaluate the whole
test set at once and ask the loss how big the error for this set of predictions is::


		Data<RealVector> predictions = model(test.inputs());
		double testError = loss.eval(test.labels(),predictions);
		

Let us see the results (do not forget to include the ``iostream`` header
for this and ``using namespace std;``) ::


		cout << "RESULTS: " << endl;
		cout << "======== \n" << endl;
		cout << "training error " << trainingError << endl;
		cout << "test error: " << testError << endl;
		

The result should read

.. code-block:: none

    RESULTS:
    ========

    training error: 0.0525739
    test error: 0.151367


What you learned
----------------
You should have learned the following aspects in this Tutorial:

* What the main building blocks of a general optimization task are: Data, Error Function, Model, Optimizer
* How to load regression data from two files and split them into training and test set.

What next?
----------
Now you know the basic architecture of Shark. From here on you should follow the more specialized sets ot tutorials:

* :doc:`../algorithms/ffnet`
