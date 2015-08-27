#include "ros/ros.h"
#include "hexa_control/Transfert.h"

#include "robotHexa.hpp"
#include "controllerDuty.hpp"

#include <boost/shared_ptr.hpp>

boost::shared_ptr<RobotHexa> hexapod_p;
ros::Publisher statepub;

bool transfert(hexa_control::Transfert::Request  &req,
               hexa_control::Transfert::Response &res )
{
  if(req.duration<-1)
  {
    hexapod_p->relax();
    return true;
  }
  else if(req.duration ==0)
  {
    hexapod_p->reset();
    return true;
  }
  else if(req.duration ==-1)
  {
    hexapod_p->position_zero();
    return true;
  }

  std::vector<float> params;
  for(int i=0;i<36;i++)
  {
    params.push_back(req.params[i]);
    ROS_INFO("%f",params[i]);
  }

  ControllerDuty ctrl(params,std::vector<int>());

  std_msgs::String msg;
  msg.data = "start";
  statepub.publish(msg);

  //hexapod_p->initial_pos();
  try
  {

    hexapod_p->transfer(ctrl,req.duration,0);

  }
  catch (dynamixel::Error e)
  {
    std::cerr << "error (dynamixel): " << e.msg() << std::endl;
    std::cout<<"closing serials"<<std::endl;
    hexapod_p->close_usb_controllers();
  }

  msg.data = "stop";
  statepub.publish(msg);

  res.covered_distance=hexapod_p->covered_distance();
  ROS_INFO("request:");

  return true;
}

int main(int argc, char **argv)
{
  //boost::shared_ptr<dynamixel::Usb2Dynamixel controller> controller;
  try
  {
    hexapod_p = boost::shared_ptr<RobotHexa>(new RobotHexa);
  }
  catch (dynamixel::Error e)
  {
    std::cerr << "error (dynamixel): " << e.msg() << std::endl;
    std::cout<<"closing serials"<<std::endl;
    hexapod_p->close_usb_controllers();
  }


  ros::init(argc, argv, "hexapod_server");
  boost::shared_ptr<ros::NodeHandle> node_p= boost::shared_ptr<ros::NodeHandle>(new ros::NodeHandle());
  hexapod_p->initRosNode(argc,argv,node_p);
  ros::ServiceServer service = node_p->advertiseService("Transfert", transfert);
  statepub = node_p->advertise<std_msgs::String>("transfertState", 10);

  ROS_INFO("Ready to control the robot.");
  ros::spin();
  hexapod_p->relax();
  return 0;
}
