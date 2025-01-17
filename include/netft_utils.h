#ifndef NET_FT_UTILS_H
#define NET_FT_UTILS_H

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "tf/transform_listener.h"
#include "geometry_msgs/WrenchStamped.h"
#include "netft_utils/SetBias.h"
#include "netft_utils/FindToolParams.h"
#include "netft_utils/SetMax.h"
#include "netft_utils/SetThreshold.h"
#include "netft_utils/SetToolData.h"
#include "netft_utils/SetBiasData.h"
#include "netft_utils/SetToolTipFrame.h"
#include "netft_utils/SetFilter.h"
#include "netft_utils/GetDouble.h"
#include "netft_utils/Cancel.h"
#include "lpfilter.h"
#include <math.h>

/**
 * This program takes force/torque data and applies transforms to usable data
 */

class NetftUtils
{
public:
  NetftUtils(ros::NodeHandle nh);
  ~NetftUtils();

  void initialize();
  void setUserInput(std::string world, std::string ft);
  void update();

private:
  //Node handle
  ros::NodeHandle n;                               // ROS node handle
  
  //LPFilter
  LPFilter* lp;                                    // Filter
  bool isFilterOn;
  double deltaTFilter;
  double cutoffFrequency;
  bool newFilter;

  // Transform listener
  tf::TransformListener* listener;
  tf::StampedTransform ft_to_world;                // Transform from ft frame to world frame
  std::string world_frame;
  std::string ft_frame;
  std::string tool_tip_frame;

  // tool tip frame
  tf::StampedTransform toolTipTransform;
 
  // Wrenches used to hold force/torque and bias data
  geometry_msgs::WrenchStamped tool_bias;               // Wrench containing the current bias data in tool frame
  geometry_msgs::WrenchStamped weight_bias;        // Wrench containing the bias at a measurement pose (to measure the weight)
  geometry_msgs::WrenchStamped raw_data_world;     // Wrench containing the current raw data from the netft sensor transformed into the world frame
  geometry_msgs::WrenchStamped raw_data_tool;      // Wrench containing the current raw data from the netft sensor in the tool frame
  geometry_msgs::WrenchStamped tf_data_world;      // Wrench containing the transformed (world frame) data with bias and threshold applied
  geometry_msgs::WrenchStamped tf_data_tool;       // Wrench containing the transformed (tool frame) data with bias and threshold applied
  geometry_msgs::WrenchStamped tf_data_tool_tip;       // Wrench containing the transformed (tool frame) data with bias and threshold applied
  geometry_msgs::WrenchStamped tf_data_world_tip;       // Wrench containing the transformed (tool frame) data with bias and threshold applied
  geometry_msgs::WrenchStamped zero_wrench;        // Wrench of all zeros for convenience
  
  double payloadWeight;				   // Used in gravity compensation
  double payloadLeverArm;			   // Used in gravity compensation. The z-coordinate to payload CoM (in sensor's raw frame)
  
  bool isBiased;                                   // True if sensor is biased
  bool isNewBias;                                  // True if sensor was biased this pass
  bool isNewGravityBias;			   // True if gravity compensation was applied this pass
  bool isGravityBiased;				   // True if gravity is compensate
  bool isDifferentToolFrame;				   // True if different Tool tip frame set

  
  // ROS subscribers
  ros::Subscriber raw_data_sub;
  
  // ROS publishers
  ros::Publisher netft_raw_world_data_pub;
  ros::Publisher netft_world_data_pub;
  ros::Publisher netft_tool_data_pub;
  ros::Publisher netft_cancel_pub;
  ros::Publisher trajectory_pub;
  ros::Publisher marker_pub;
  
  ////////////////
  // ROS services
  ////////////////
  ros::ServiceServer bias_service;
  ros::ServiceServer gravity_comp_service;
  ros::ServiceServer weight_bias_service;
  ros::ServiceServer set_tool_data;
  ros::ServiceServer set_bias_data;
  ros::ServiceServer find_tool_params;
  ros::ServiceServer filter_service;
  ros::ServiceServer set_tool_tip_frame_service;

  ////////////////////
  // Callback methods
  ////////////////////
  
  // Runs when a new datapoint comes in
  void netftCallback(const geometry_msgs::WrenchStamped::ConstPtr& data);
  
  // Set the readings from the sensor to zero at this instant and continue to apply the bias on future readings.
  // This doesn't account for gravity i.e. it will not change if the sensor's orientation changes.
  // Run this method when the sensor is stationary to avoid inertial effects.
  bool fixedOrientationBias(netft_utils::SetBias::Request &req, netft_utils::SetBias::Response &res);

  bool findToolParams(netft_utils::FindToolParams::Request &req, netft_utils::FindToolParams::Response &res);
  
  // Set the readings from the sensor to zero at this instant.
  // Calculate the payload's mass and center of mass so gravity can be compensated for, even as the sensor changes orientation.
  // It's assumed that the payload's center of mass is located on the sensor's central access.
  // Run this method when the sensor is stationary to avoid inertial effects.
  // It assumes the Z-axis of the World tf frame is up.
  bool compensateForGravity(netft_utils::SetBias::Request &req, netft_utils::SetBias::Response &res);
  bool settooldata(netft_utils::SetToolData::Request &req, netft_utils::SetToolData::Response &res);
  bool setbiasdata(netft_utils::SetBiasData::Request &req, netft_utils::SetBiasData::Response &res);
  
  bool setWeightBias(netft_utils::SetBias::Request &req, netft_utils::SetBias::Response &res);
  bool setFilter(netft_utils::SetFilter::Request &req, netft_utils::SetFilter::Response &res);
  bool setToolTipFrame(netft_utils::SetToolTipFrame::Request &req, netft_utils::SetToolTipFrame::Response &res);

  // Convenience methods
  void copyWrench(geometry_msgs::WrenchStamped &in, geometry_msgs::WrenchStamped &out, geometry_msgs::WrenchStamped &bias);
  void transformFrame(geometry_msgs::WrenchStamped in_data, geometry_msgs::WrenchStamped &out_data, char target_frame);
};

#endif