/*
 * MecanumAnalyser.cpp
 *
 *  Created on: 19.01.2018
 *      Author: m1ch1
 */

#include "MecanumAnalyser.h"

namespace analyser{

MecanumAnalyser::MecanumAnalyser(const double target_radius, const double target_radius_final, double end_approach) :
    PathAnalyser_base()
{
  _target_radius       = target_radius;
  _target_radius_final = target_radius_final;
  _end_approach_length = end_approach;

  _curr_target_raius   = _target_radius;
  _is_end_approach = false;
}

MecanumAnalyser::~MecanumAnalyser()
{
  // nothing to do
}

analyser::diff_scale MecanumAnalyser::analyse(const analyser::pose& current_pose)
{
  analyser::diff_scale diff_scale;

  std::cout << "---" << std::endl;

  if(_path.empty())
  {
    this->setReachedFinalGoal(false);
    return diff_scale;
  }

  if(this->isReachedFinalGoal())
  {
    return diff_scale;
  }


  Vector3d ori = current_pose.orientation;
  Vector3d pos = current_pose.position;

  Vector3d p = this->currentGoal().position - pos;
  p.z() = 0;

  //take next goal, maybe skip multiple goals if movement was great enough
  //todo what to do with last goal...
  while(p.norm() < _curr_target_raius)
  {
    this->nextGoal();
    p = this->currentGoal().position - pos;
    p.z() = 0;

    //todo rdy stuff...

    if(this->isLastGoal())
    {
      break;
    }
  }

  Vector3d e(1,0,0);


  int direction_angular = this->getDirection(this->currentGoal().orientation, ori);
  int direction_linear  = this->getDirection(e, ori);
  Vector3d p_ang = this->currentGoal().orientation;


  //get scalfactor angular
  double diff_max = M_PI_2;
  double diff_ang = ::acos(static_cast<double>(ori.dot(p_ang) / (ori.norm() * p_ang.norm())));
  double diff_lin = ::acos(static_cast<double>(ori.dot(e) / (ori.norm() * e.norm())));

  //reached last pose if arrived last goal
//  if(_reachedLastPose && std::abs(tmp_diff) < _ang_reached_range)
//  {
//    diff_scale.angular = 0;
//    this->local_reachedFinalGoal(true);
//  }
//  else
  diff_scale.angular = (diff_ang / diff_max) * direction_angular;  //scale between -1..1

  //linear scale...

  //p is xy diff to next goal...
  Vector2d p_2d(p.x(), p.y());

  //roate p by ori...
  Rotation2D<double> rot_2d(diff_lin * direction_linear);

  p_2d = rot_2d * p_2d;


  //scale length to 1
  //todo scale force set higher prio to move towards path... //todo check if needen i think not...

  if((this->getPathLengthRest() + p.norm()) < _end_approach_length)
  {
    p_2d /= _end_approach_length;
  }
  else
  {
    p_2d /= p_2d.norm();
  }

  diff_scale.linear_x = p_2d.x();
  diff_scale.linear_y = p_2d.y();

  //todo set min vel...

  return diff_scale;
}

} //namespace analyser
