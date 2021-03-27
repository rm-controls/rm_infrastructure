//
// Created by qiayuan on 1/5/21.
//

#include "rm_common/filters/lp_filter.h"
#include "rm_common/ros_utilities.h"

LowPassFilter::LowPassFilter(ros::NodeHandle &nh) {
  nh.param("lp_cutoff_frequency", cutoff_frequency_, -1.);
  nh.param("lp_debug", is_debug_, false);

  if (is_debug_)
    realtime_pub_.reset(new realtime_tools::RealtimePublisher<rm_msgs::LpData>(nh, "lp_filter", 100));
}

void LowPassFilter::input(double in, ros::Time time) {
  // My filter reference was Julius O. Smith III, Intro. to Digital Filters
  // With Audio Applications.
  // See https://ccrma.stanford.edu/~jos/filters/Example_Second_Order_Butterworth_Lowpass.html
  in_[2] = in_[1];
  in_[1] = in_[0];
  in_[0] = in;

  if (!prev_time_.isZero())  // Not first time through the program
  {
    delta_t_ = time - prev_time_;
    prev_time_ = time;
    if (0 == delta_t_.toSec()) {
      ROS_ERROR("delta_t is 0, skipping this loop. Possible overloaded cpu ""at time: %f", time.toSec());
      return;
    }
  } else {
    prev_time_ = time;
    return;
  }

  if (cutoff_frequency_ != -1 && cutoff_frequency_ > 0) {
    // Check if tan(_) is really small, could cause c = NaN
    tan_filt_ = tan((cutoff_frequency_ * 6.2832) * delta_t_.toSec() / 2.);
    // Avoid tan(0) ==> NaN
    if ((tan_filt_ <= 0.) && (tan_filt_ > -0.01))
      tan_filt_ = -0.01;
    if ((tan_filt_ >= 0.) && (tan_filt_ < 0.01))
      tan_filt_ = 0.01;
    c_ = 1 / tan_filt_;
  }

  out_[2] = out_[1];
  out_[1] = out_[0];
  out_[0] = (1 / (1 + c_ * c_ + M_SQRT2 * c_)) * (in_[2] + 2 * in_[1] + in_[0] -
      (c_ * c_ - M_SQRT2 * c_ + 1) * out_[2] - (-2 * c_ * c_ + 2) * out_[1]);

  if (is_debug_) {
    if (realtime_pub_->trylock()) {
      realtime_pub_->msg_.header.stamp = time;
      realtime_pub_->msg_.real = in_[0];
      realtime_pub_->msg_.filtered = out_[0];
      realtime_pub_->unlockAndPublish();
    }
  }
}

void LowPassFilter::input(double in) {
  input(in, ros::Time::now());
}

double LowPassFilter::output() {
  return out_[0];
}


