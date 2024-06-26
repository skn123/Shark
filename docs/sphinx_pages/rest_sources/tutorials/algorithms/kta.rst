=======================
Kernel Target Alignment
=======================

Just about every kernel of practical interest (maybe up to the linear
kernel) actually forms a family of kernels with a number of parameters.
Such parameters generally need problem specific tuning. One way of
achieving this is by optimization of an objective function that measures
the suitability of the kernel for the task. Here we focus on
classification tasks.

Kernel target alignment (KTA) and related measures such as kernel
polarization define such quality scores. They are relatively cheap to
compute since they do not involve machine training. This also means that
they cannot tune too specifically to a single classifier, but rather to
the task of classifying the given specific type of data with any method.

The general idea of kernel target alignment is that a data embedding
that nicely clusters classes in feature space is beneficial for any type
of classifier. The perfect embedding thought for by kernel target
alignment is to map all points of the same class to the same point and
all classes to different points. This means that within-class distances
in feature space are zero and between-class distances are positive. Let
:math:`k(x, x')` denote a kernel function with corresponding feature map
:math:`\phi`, then this is equivalent to

.. math ::
	\|\phi(x) - \phi(x')\|^2 = k(x, x) - 2 k(x, x') + k(x', x') = \begin{cases}
		0 & \text{ for } y = y' \\
		c & \text{ for } y \not= y' \\
	\end{cases}

where c > 0 is any constant.

Actually realizing a feature mapping with this exact property in practice
most probably means heavy over-fitting. Still, within a parametric family
of kernels aiming in this direction usually is a good idea. This preference
is expressed by kernel target alignment objective function

.. math ::
	\frac{\langle K, Y \rangle}{\sqrt{\langle K, K \rangle}}

:math:`K_{ij} = k(x_i, x_j)` is the kernel Gram matrix of the inputs.
where the entries :math:`Y_{ij}` of matrix Y are one if :math:`y_i = y_j`
and :math:`-1/(d-1)` otherwise, where d denotes the number of classes.
It is the Gram matrix of an "ideal" kernel mapping each class to a single
point, with these points forming the vertices of a symmetric simplex.
:math:`\langle A, B \rangle = \sum_{i,j} A_{ij} B_{ij}` is the standard
inner product on matrices, interpreted as vectors. This is the original
KTA objective function proposed in [CSK2001]_.

This objective minimizes within-class spread of the data, normalized by
overall spread. However, the objective is not translation invariant.
This flaw was corrected in [CMR2010]_ where it was proposed to center
the data in feature space before computing the above formula. The
resulting objective function is implemented in Shark as
:doxy:`KernelTargetAlignment`. It is a real-valued objective function,
interpreted as a function of the parameters of a kernel function object.
The negative KTA is implemented since by convention Shark optimizers
solves minimization problems. Any optimization strategy can be used to
adjust the kernel parameters.

As an example we adjustment of the bandwidth parameter of a Gaussian
RBF kernel. ::


	#include <shark/Data/DataDistribution.h>
	#include <shark/Models/Kernels/GaussianRbfKernel.h>
	#include <shark/ObjectiveFunctions/KernelTargetAlignment.h>
	#include <shark/Algorithms/GradientDescent/Rprop.h>
	
	using namespace shark;
	using namespace std;
	
	int main(int argc, char** argv)
	{
		// generate dataset
		Chessboard problem;          // artificial benchmark problem
		LabeledData<RealVector, unsigned int> data = problem.generateDataset(1000);
	
		// define the family of kernel functions
		double gamma = 0.5;          // initial guess of the parameter value
		GaussianRbfKernel<RealVector> kernel(gamma);
	
		// set up kernel target alignment as a function of the kernel parameters
		// on the given data
		KernelTargetAlignment<RealVector> kta(data,&kernel);
	
		// optimize parameters for best alignment
		Rprop<> rprop;
		rprop.init(kta);
		cout << "initial parameter: " << kernel.gamma() << endl;
		for (size_t i=0; i<50; i++)
		{
			rprop.step(kta);
			cout << "parameter after step " << (i+1) << ": " << kernel.gamma() << endl;
		}
		cout << "final parameter: " << kernel.gamma() << endl;
	}
	

The example first constructs a toy data set and a :doxy:`GaussianRbfKernel`
instance with bandwidth parameter :math:`\gamma`. The parameter is set to
the initial guess of 0.5. Then the KTA objective function is constructed,
and the kernel object as well as the data is made know to it. The program
runs 50 iterations of the gradient-based :doxy:`Rprop` optimizer to
tune the parameter.

Running the program may take a few seconds. Notice that the derivative of
the :math:`1000 \times 1000` kernel matrix w.r.t. the kernel parameter
needs to be computed in every iteration. The complexity of this process
grows quadratically with the data set size. For large data it is rather
safe to sub-sample the data in order to reduce the complexity. As an
experiment the data set size in the above example can be reduced to, say,
150, which should give a similar result.


References
----------

.. [CSK2001] N. Cristianini, J. Shawe-Taylor, and J. Kandola.
	On Kernel Target Alignment.
	In Proceedings of Neural Information Processing Systems (NIPS), 2001.

.. [CMR2010] C. Cortes, M. Mohri, and A. Rostamizadeh.
	Two-Stage Learning Kernel Algorithms.
	In Proceedings of the 27th International Conference on Machine Learning (ICML), 2010.
