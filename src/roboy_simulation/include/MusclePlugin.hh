#pragma once
// common definitions
#include "CommonDefinitions.h"
#include "CommunicationData.h"
// gazebo
#include <gazebo/gazebo.hh>
#include <gazebo/common/Plugin.hh>
#include <gazebo/common/common.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/util/system.hh>
#include <gazebo/math/Vector3.hh>
#include <gazebo/math/Pose.hh>
// ros
#include <ros/ros.h>
#include <pluginlib/class_list_macros.h>
// boost
#include <boost/numeric/odeint.hpp>
#include <boost/bind.hpp>
//std
#include <math.h>
#include <map>
#include <stdio.h>
#include <sstream>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>

namespace roboy_simulation {

	using namespace std;
	using namespace boost::numeric::odeint;
	using namespace gazebo;

	const int linkNumber = 3; //later read this number from some table or .txt files

	struct Motor {
		double current;
		double torqueConst;
		double resistance;
		double inductance;
		double voltage;
		double BEMFConst; // back electromagnetic force constant
		double inertiaMoment;
	};

	struct Gear {
		double inertiaMoment;
		double ratio;
		double efficiency; // gear efficciency
		double appEfficiency; // approximated efficiency
	};

	struct Spindle {
		double angVel; // angular velocity of the spindle
		double radius;
	};

	struct SEE {
		double stiffness;
		double length;
		double lengthRest;
	};

	struct tendonType {
		vector<math::Vector3> MidPoint;
		vector<math::Vector3> Vector;
		//might need it to calculate length
		vector<math::Vector3> Orientation;
		vector<double> Pitch;
		vector<double> Roll;
	};

	struct MyoMuscleInfo{
		string name;
		vector<string> joint;
		map<string,vector<math::Vector3>> viaPoints;
		Motor motor;
		Gear gear;
		Spindle spindle;
		SEE see;
	};

	struct PIDcontroller{
		double calc_output(double cmd, double pos, double timePeriod);
		sint32 outputPosMax = 24; /*!< maximum control output in the positive direction in counts, max 4000*/
		sint32 outputNegMax = -24; /*!< maximum control output in the negative direction in counts, max -4000*/
		float32 spPosMax = 100;/*<!Positive limit for the set point.*/
		float32 spNegMax = 100;/*<!Negative limit for the set point.*/
		parameters_t params;
		float32 integral;
		float32 lastError;
	};

	class ITendon {
		////////////////////////////////////////
		/// \brief Calculate torque for an electric motor model.
		/// \param[in] _current Input electric current
		/// \param[in] _torqueConstant Motor's torque constant
		/// \param[in] _spindleRadius Radius of the spindle that coils up the tendon
		/// \return Calculated force according to the model
	public:
		double ElectricMotorModel(const double _current, const double _torqueConstant,
								  const double _spindleRadius);

		////////////////////////////////////////
		/// \brief Calculate elastic force of the series elastic element
		/// \param[in] _length0 Resting length of the SEE
		/// \param[in] _length Current length of the SEE
		/// \param[in] _stiffness Deafault values for stiffness of the SEE
		/// \return Elastic force in N
		double ElasticElementModel(const double _length0, const double _length,
								   double _stiffness, const double _speed,
								   const double _spindleRadius, const double _time);

		////////////////////////////////////////
		/// \brief Calculate forces generated by the motor and SEE
		/// \param[in] _elasticForce
		/// \param[in] _motorForce
		/// \param[in] position and orientation of the tendon
		/// \return Motor force in N
		math::Vector3 CalculateForce(double _elasticForce, double _motorForce,
									 const math::Vector3 &_tendonOrien);

		static void GetTendonInfo(vector<math::Vector3> &viaPointPos, tendonType *tendon_p);

		SEE see;
	private:
		////////////////////////////////////////
		/// \brief Calculate the dot product between two vectors
		/// \param[in] _v1 vector 1 coordinates
		/// \param[in] _v2 vector 2 coordinates
		/// \return Dot product
		double DotProduct(const math::Vector3 &_v1, const math::Vector3 &_v2);

		////////////////////////////////////////
		/// \brief Calculate the angle between two vectors
		/// \param[in] _v1 vector 1 coordinates
		/// \param[in] _v2 vector 2 coordinates
		/// \return Angle between two vectors in radians
		double Angle(const math::Vector3 &_v1, const math::Vector3 &_v2);
	};


	class IActuator {
		// state vector for differential model
	public:
		typedef std::vector<double> state_type;
		// private: std::vector< double > x(2);

		// stepper for integration
		boost::numeric::odeint::runge_kutta4<state_type> stepper;

		////////////////////////////////////////
		/// \brief Approximates gear's velocity
		/// according to the direction of the rotation of the gear, i.e.
		/// eta or 1/eta
		/// \return Approximated value for gear efficiency
		double EfficiencyApproximation();
		// private: void DiffModel(const state_type &x , state_type &dxdt , const double /* t */);

		void DiffModel(const state_type &x, state_type &dxdt, const double /* t */);

		Motor motor;
		Gear gear;
		Spindle spindle;
		double elasticForce;
	};


	class MusclePlugin{

	public:
		MusclePlugin();

		void Init(MyoMuscleInfo &myoMuscle);
		void Update(ros::Time &time, ros::Duration &period,
					vector<math::Vector3> &viaPointInGobalFrame,
					vector<math::Vector3> &force);
		string name;
		vector<string> joint;
		map<string,vector<math::Vector3>> viaPoints;
		map<string,math::Pose> linkPose;
		PIDcontroller pid;
		double cmd;
	private:
		event::ConnectionPtr connection;
		common::Time prevUpdateTime;
		physics::ModelPtr model;
		std::vector<physics::LinkPtr> links;

		IActuator::state_type x;
		ITendon tendon;
		IActuator actuator;

		double actuatorForce;

		transport::NodePtr node;
		transport::PublisherPtr visPub;

//		msgs::Visual visualMsg[linkNumber - 1];
	};
}

