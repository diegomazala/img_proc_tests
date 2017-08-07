
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
			std::cout << fvec(i) << " : " << source[i] << ' ' << target[i] << std::endl;
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

	Eigen::Matrix<Type, 3, 1> functorResult;


	void compute()
	{
		//
		// Compute fitting per channel
		// 
		for (int i = 0; i < 3; ++i)
		{
			Eigen::VectorXd result;
			result.fill(0.0f);
			FittingNumericalDiff functor;
			functor.source = source.col(i);	// per channel
			functor.target = target.col(i);	// per channel

			std::cout << "Size: " << functor.source.rows() << ' ' << functor.source.cols() << std::endl;
			std::cout << "Size: " << functor.target.rows() << ' ' << functor.target.cols() << std::endl;

			Eigen::LevenbergMarquardt<FittingNumericalDiff> lm(functor);
			Eigen::LevenbergMarquardtSpace::Status status = lm.minimize(result);
#if 1
			//Eigen::LevenbergMarquardtSpace::Status status = lm.minimize(x);
			std::cout << "status: " << status << std::endl;
			//std::cout << "info: " << lm.info() << std::endl;
			std::cout << "x that minimizes the function: " << std::endl << result.transpose() << std::endl;
			const auto src_channel = source.col(i);
			const auto tgt_channel = target.col(i);
			for (int j = 0; j < src_channel.rows(); ++j)
			{
				std::cout
					<< std::fixed
					<< src_channel[j] << ' ' << tgt_channel[j] << '\t' << src_channel[j] * result[0] + result[1] << std::endl;
			}
#endif
		}
	}
};


#endif // _COLOR_FITTING_H_
