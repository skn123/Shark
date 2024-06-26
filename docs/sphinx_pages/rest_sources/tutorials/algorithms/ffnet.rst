Training Feed-Forward Neural Networks
========================================
This tutorial will show you how to construct a feed-forward
multi-layer neural network and how to train it efficiently using minibatch training
and the Adam Optimization algorithm. It is recommended to read the
getting started section, especially the introduction about :doc:`../first_steps/general_optimization_tasks`.
The full example program can be found here: :doxy:`FFNNBasicTutorial.cpp`.

For this tutorial the following includes are needed::


	//the model
	#include <shark/Models/LinearModel.h>//single dense layer
	#include <shark/Models/ConcatenatedModel.h>//for stacking layers, provides operator>>
	//training the  model
	#include <shark/ObjectiveFunctions/ErrorFunction.h>//error function, allows for minibatch training
	#include <shark/ObjectiveFunctions/Loss/CrossEntropy.h> // loss used for supervised training
	#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h> // loss used for evaluation of performance
	#include <shark/Algorithms/GradientDescent/Adam.h> //optimizer: simple gradient descent.
	#include <shark/Data/SparseData.h> //loading the dataset
	using namespace shark;
	

Loading the Data
------------------------------
In this tutorial, we want to learn to recognize handwriten digits from the mnist dataset
using a simple feed-forward network. This is done similar to the previous tutorials
by loading a dataset and splitting it into training and test part::


		std::size_t batchSize = 256;
		LabeledData<RealVector,unsigned int> data;
		importSparseData( data, argv[1], 0, batchSize );
		data.shuffle(); //shuffle data randomly
		auto test = splitAtElement(data, 70 * data.numberOfElements() / 100);//split a test set
		std::size_t numClasses = numberOfClasses(data);
		std::size_t inputDim = inputDimension(data);
	

Note that we define the batchsize of the dataset during loading of the dataset.
The batchsize is important for minibatch learning as we pick one random batch from the dataset in each iteration.

Defining the Network topology
------------------------------

For this, we stack several layers on top of each other. For simplicity
we will use only :doxy:`LinearModel` with nonlinear :doxy:`activation functions<activations>`. The activation
function is encoded in an optional second template argument of the linear model.
For this tutorial we decide that the two hidden layers should use
Rectified-Linear Units (ReLu). For classification tasks, output neurons should be
linear. All layers should use a constant offset term. The layers are then chained together using `operator>>`::


		//We use a dense linear model with rectifier activations
		typedef LinearModel<RealVector, RectifierNeuron> DenseLayer;
		
		//build the network
		DenseLayer layer1(inputDim,hidden1, true);
		DenseLayer layer2(hidden1,hidden2, true);
		LinearModel<RealVector> output(hidden2,numClasses, true);
		auto network = layer1 >> layer2 >> output;
	

As you can see, we define the number of inputs and outputs and usage of the offset term for each layer during construction.

Training the Network
----------------------

After we loaded the dataset and defined the topology of the network,
we can train it. Since we use a classification task, we can
use the :doxy:`CrossEntropy` error to maximize the class probability.
The cross entropy assumes the inputs to be the log of
the unnormalized probability :math:`p(y=c|x)`, i.e. the probability of
the input to belong to class :math:`c`. The cross entropy uses an
exponential normalisation to transform the inputs into proper
normalised probabilities. This is done in a numericaly stable way.

The c-th output neuron of the network encodes the probability of class c.
In case of a binary problem, we can omit one
output neuron and in this case, it is assumed that the output of the
`imaginary` second neuron is just the negative of the first. The loss
function takes care of the normalisation. After training, the most
likely class label of an input can be evaluated by picking the class
of the neuron with highest activation value.  In the case of only one
output neuron, the sign decides: negative activation is class 0,
positive is class 1.

We will setup our error function to use minibatch training. Every time
the error function is evaluated, a random batch in the dataset is evaluated.
Thus the batch size defined during dataset creation is an important parameter and a trade-off betwen evaluation
speed and noise of the evaluation::


		//create the supervised problem. 
		CrossEntropy<unsigned int, RealVector> loss;
		ErrorFunction<> error(data, &network, &loss, true);//enable minibatch training
		
		//optimize the model
		std::cout<<"training network"<<std::endl;
		initRandomNormal(network,0.001);
		Adam<> optimizer;
		error.init();
		optimizer.init(error);
		for(std::size_t i = 0; i != iterations; ++i){
			optimizer.step(error);
			std::cout<<i<<" "<<optimizer.solution().value<<std::endl;
		}
		network.setParameterVector(optimizer.solution().point);
	


What you learned
----------------

You should have learned the following aspects in this Tutorial:

* How to stack models to create a feed-forward neural network
* How to choose activation functions
* How to perform batch learning using the :doxy:`ErrorFunction`


What next?
---------------------
The network architecture used for training MNIST is not state of the art. In real applications, deep convolutional architectures are used.
This is covered in the next tutorial, :doc:`DeepMNIST`.