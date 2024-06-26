================
LASSO Regression
================


Background
----------

LASSO stands for "Least Absolute Shrinkage and Selection Operator".
The term is often used synonymous with 1-norm regularization. In its
most basic form it refers to 1-norm regularized linear regression.
For the basics on linear regression we refer to the
:doc:`linearRegression` tutorial.

In contrast to plain linear regression or ridge regression LASSO
solves a 1-norm regularized problem. For training data

.. math ::
   S=\{(x_1, y_1), \dots, (x_l, y_l)\} ,

with vector valued inputs *x* and real valued labels *y* the following
optimization problem is solved:

.. math ::
   \min_w \,\, \lambda \|w\|_1 + \sum_i \|y_i - \langle w, x_i \rangle\|_2^2

The decisive property of LASSO regression is that the one-norm term
enforces sparseness of the solution. In particular for rather large
values of :math:`\lambda` the solution *w* has only few non-zero
components. This allows regression to be meaningful even if the
feature dimension greatly exceeds the number of data points, since the
method reduces the linear predictor to few variables. Therefore the
method is often used for the identification of (hopefully) causal
relationships: it is hoped that the label is mainly caused by the
values of rather few features. These are often termed explanatory
variables.

LASSO problems can be solved quickly with coordinate descent algorithms,
see [FHHT2007]_. Shark implements the optimized algorithm from [GU2013]_.


LASSO in Shark
--------------

The method in implemented by the :doxy:`LassoRegression` trainer.
It operates on a standard :doxy:`LinearModel`: ::


	#include <shark/Algorithms/Trainers/LassoRegression.h>
	
		double lambda = 1.0;
		
		LinearModel<> model;
		LassoRegression<> trainer(lambda);
		
		trainer.train(model, data);
		

Of course, `data` is assumed to be training data of appropriate type,
i.e., `RealVector` inputs and one-dimensional regression labels.
After training the weight vector of the linear model can be examined
for non-zero coefficients and therefore for explanatory variables.
Shark comes with a fully operational example program:

:doxy:`LassoRegression.cpp <LassoRegression.cpp>`


References
----------

.. [FHHT2007] J. Friedman, T. Hastie, H. Höfling, and R. Tibshirani.
   Pathwise Coordinate Optimization. The Annals of Applied Statistics, 1(2):302-332, 2007.

.. [GU2013] T. Glasmachers and Ü. Dogan.
   Accelerated Coordinate Descent with Adaptive Coordinate Frequencies.
   In Proceedings of the fifth Asian Conference on Machine Learning (ACML), 2013.
