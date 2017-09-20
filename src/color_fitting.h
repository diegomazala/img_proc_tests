
#ifndef _COLOR_FITTING_H_
#define _COLOR_FITTING_H_

#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <vector>
#include <iostream>


// Generic functor
template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct Functor
{
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};
	typedef Eigen::Matrix<Scalar, InputsAtCompileTime, 1> InputType;
	typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, 1> ValueType;
	typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, InputsAtCompileTime> JacobianType;

	int m_inputs, m_values;

	Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	int inputs() const { return m_inputs; }
	int values() const { return m_values; }

};


template<typename Type>
struct FittingFunctor : Functor<double>
{
	int operator()(const Eigen::VectorXd &x, Eigen::VectorXd &fvec) const
	{
		eigen_assert(source.rows() == target.rows());

		// "a" in the model is x(0), and "b" is x(1)
		//for (unsigned int i = 0; i < this->Points.size(); ++i)
		for (unsigned int i = 0; i < this->source.rows(); ++i)
		{
			//fvec(i) = this->Points[i][1] - (x(0) * this->Points[i][0] + x(1));
			fvec(i) = this->target[i] - (x(0) * this->source[i] + x(1));
			//std::cout << fvec(i) << " : " << source[i] << ' ' << target[i] << std::endl;
		}


		return 0;
	}

	Eigen::Matrix<Type, Eigen::Dynamic, 1> source;
	Eigen::Matrix<Type, Eigen::Dynamic, 1> target;

	int inputs() const { return 2; } // There are two parameters of the model
	int values() const { return this->source.rows(); } // The number of observations
};

struct FittingNumericalDiff : Eigen::NumericalDiff<FittingFunctor<double>> 
{
};


template<typename Type>
struct RgbFitting
{
	Eigen::Matrix<Type, Eigen::Dynamic, 3> source;
	Eigen::Matrix<Type, Eigen::Dynamic, 3> target;
	//Eigen::Matrix<double, 2, 1> result;

	Eigen::Matrix<Type, 3, 2> functorResult;

	// LM minimize for the model y = a x + b
	void compute()
	{
		//
		// Compute fitting per channel
		// 
		for (int i = 0; i < 3; ++i)
		{
			Eigen::VectorXd result(2);
			result.fill(0.0f);
			FittingNumericalDiff functor;
			functor.source = source.col(i);	// per channel
			functor.target = target.col(i);	// per channel
			Eigen::LevenbergMarquardt<FittingNumericalDiff> lm(functor);
			Eigen::LevenbergMarquardtSpace::Status status = lm.minimize(result);
#if 0
			std::cout << "status: " << status << std::endl;
			std::cout << "info: " << lm.info() << std::endl;
			std::cout << "x that minimizes the function: " << std::endl << result.transpose() << std::endl;
#endif
			functorResult(i, 0) = result[0];
			functorResult(i, 1) = result[1];
		}


		//std::cout
		//	<< "Functor Result: " << std::endl
		//	<< functorResult << std::endl << std::endl;

		
		//for (int i = 0; i < source.rows(); ++i)
		//{
		//	std::cout
		//		<< std::fixed << std::setprecision(0)
		//		<< source.row(i) << " <-> "
		//		<< target.row(i) << "  =  "
		//		<< std::fixed << std::setprecision(2)
		//		<< source(i, 0) * functorResult(0, 0) + functorResult(0, 1) << ", "
		//		<< source(i, 1) * functorResult(1, 0) + functorResult(1, 1) << ", "
		//		<< source(i, 2) * functorResult(2, 0) + functorResult(2, 1) << std::endl;
		//}
	}
};


#endif // _COLOR_FITTING_H_
