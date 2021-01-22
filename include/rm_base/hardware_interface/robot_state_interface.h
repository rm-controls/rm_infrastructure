//
// Created by qiayuan on 1/3/21.
//

#ifndef RM_BASE_INCLUDE_RM_BASE_HARDWARE_INTERFACE_ROBOT_STATE_INTERFACE_H_
#define RM_BASE_INCLUDE_RM_BASE_HARDWARE_INTERFACE_ROBOT_STATE_INTERFACE_H_

#pragma once
#include <utility>

#include <hardware_interface/internal/hardware_resource_manager.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/transform_broadcaster.h>

#include "rm_base/robot_state_controller/tf_rt_broadcaster.h"

namespace hardware_interface {

class RobotStateHandle {
 public:
  RobotStateHandle() = default;
  RobotStateHandle(std::string name, tf2_ros::Buffer *buffer,
                   robot_state_controller::TfRtBroadcaster *tf_bro,
                   robot_state_controller::StaticTfRtBroadcaster *static_tf_bro)
      : name_(std::move(name)), buffer_(buffer), tf_broadcaster_(tf_bro), static_tf_broadcaster_{static_tf_bro} {};

  geometry_msgs::TransformStamped lookupTransform(const std::string &target_frame, const std::string &source_frame,
                                                  const ros::Time &time) {
    return buffer_->lookupTransform(target_frame, source_frame, time);
  }

  bool setTransform(const geometry_msgs::TransformStamped &transform, const std::string &authority,
                    bool is_static = false) const {
    return buffer_->setTransform(transform, authority, is_static);
  }

  bool setTransform(const std::vector<geometry_msgs::TransformStamped> &transforms, const std::string &authority,
                    bool is_static = false) const {
    for (const auto &transform: transforms)
      buffer_->setTransform(transform, authority, is_static);
    return true;
  }

  void pubTransform(const geometry_msgs::TransformStamped &transform) {
    tf_broadcaster_->sendTransform(transform);
  }

  void pubStaticTransform(const geometry_msgs::TransformStamped &transform) {
    static_tf_broadcaster_->sendTransform(transform);
  }

  std::string getName() const { return name_; }

 private:
  std::string name_;
  tf2_ros::Buffer *buffer_;
  robot_state_controller::TfRtBroadcaster *tf_broadcaster_;
  robot_state_controller::StaticTfRtBroadcaster *static_tf_broadcaster_;

};

class RobotStateInterface : public HardwareResourceManager<RobotStateHandle, ClaimResources> {};
}
#endif //SRC_RM_SOFTWARE_RM_BASE_INCLUDE_RM_BASE_HARDWARE_INTERFACE_ROBOT_STATES_INTERFACE_H_
