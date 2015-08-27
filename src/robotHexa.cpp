#include <iostream>
#include <limits>
#include "robotHexa.hpp"

#include <fstream>


bool msg_recv;

/*void RobotHexa :: posCallback(const rgbdslam::XtionDisplacement& msg)
  {
  msg_recv=true;

  ROS_INFO("Commandes recues: \nX:%f \n Y:%f \n Z:%f \n duraction: %f  ",msg.x,msg.y,msg.z,msg.duration);
  _covered_distance=round(msg.x * 100) / 100.0f;
  //2-sqrt((2-msg.x)*(2-msg.x)+(0-msg.y)*(0-msg.y)+(0-msg.z)*(0-msg.z));
  _final_pos.resize(2);
  _final_pos[0]=-msg.y;
  _final_pos[1]=msg.x;

  _final_angle=msg.z*180/M_PI;

  _slam_duration = msg.duration;
  ROS_INFO("distance parcourue: %f",_covered_distance);
  }*/



void RobotHexa :: posCallback(const nav_msgs::Odometry& msg)
{
  ros::Duration tdiff=this->_request_time - msg.header.stamp;
  ROS_INFO("reception delay: %f sec",tdiff.toSec());
  if(tdiff<=ros::Duration(0,0))// responce received after end of movement.
  {

    msg_recv=true;
  }
  else
    return;
  tf::Transform temp;
  tf::poseMsgToTF(msg.pose.pose,temp);
  _final_pos=_prev_pos.inverse()*temp;
  ROS_INFO("Start position: \nX:%f \n Y:%f \n Z:%f \n",_prev_pos.getOrigin()[0],_prev_pos.getOrigin()[1],_prev_pos.getOrigin()[2]);
  _prev_pos=temp;
  ROS_INFO("move performed: \nX:%f \n Y:%f \n Z:%f \n",_final_pos.getOrigin()[0],_final_pos.getOrigin()[1],_final_pos.getOrigin()[2]);
  _covered_distance=round(_final_pos.getOrigin()[0]*100)/100.0f;
  ROS_INFO("distance parcourue: %f",_covered_distance);
  /*

    if(_prev_pos.size()==0)
    {
    _prev_pos.push_back(msg.pose.pose.position.x);
    _prev_pos.push_back(msg.pose.pose.position.y);
    _prev_pos.push_back(msg.pose.pose.position.z);

    }
    else
    {
    _final_pos.clear();
    _final_pos.push_back(msg.pose.pose.position.x-_prev_pos[0]);
    _final_pos.push_back(msg.pose.pose.position.y-_prev_pos[1]);
    _final_pos.push_back(msg.pose.pose.position.z-_prev_pos[2]);

    _prev_pos.clear();
    _covered_distance=round(_final_pos[0]*100)/100.0f;
    ROS_INFO("distance parcourue: %f",_covered_distance);
    }*/
  return;

}


void RobotHexa :: init()
{
  try
  {
    _controller.open_serial("/dev/ttyACM0",B1000000);

    // Scan actuators IDs
    _controller.scan_ax12s();
    const std::vector<byte_t>& ax12_ids = _controller.ax12_ids();
    if (!ax12_ids.size())
    {
      std::cerr<<"[ax12] no ax12 detected"<<std::endl;
      return;
    }
    std::cout << "[dynamixel] " << ax12_ids.size()
	            << " dynamixel are connected" << std::endl;

    // Set AX-12+ ids : [1, 2, 3]
    _actuators_ids.push_back(1);
    _actuators_ids.push_back(11);
    _actuators_ids.push_back(21);


    _actuators_ids.push_back(2);
    _actuators_ids.push_back(12);
    _actuators_ids.push_back(22);


    _actuators_ids.push_back(3);
    _actuators_ids.push_back(13);
    _actuators_ids.push_back(23);


    _actuators_ids.push_back(4);
    _actuators_ids.push_back(14);
    _actuators_ids.push_back(24);


    _actuators_ids.push_back(5);
    _actuators_ids.push_back(15);
    _actuators_ids.push_back(25);


    _actuators_ids.push_back(6);
    _actuators_ids.push_back(16);
    _actuators_ids.push_back(26);


    std::cout << "initialisation completed" << std::endl;
  }
  catch (dynamixel::Error e)
  {
    std::cerr << "error (dynamixel): " << e.msg() << std::endl;
  }

#ifdef IMU
  try
  {

    _imu.open_serial("/dev/ttyUSB1");

  }
  catch (imu::Error e)
  {
    std::cerr << "error (imu): " << e.msg() << std::endl;
  }

#endif
  // motor position correctio (offset)
  _correction=std::vector<int>(18,0);
  _correction[0]=-256;
  _correction[2]=-300;
  _correction[4]=-240;
  _correction[6]=256;
  _correction[7]=50;
  _correction[9]=-256;
  _correction[10]=-150;
  _correction[13]=-40;
  _correction[15]=256;


  //  setPID();

}

void RobotHexa::setPID()
{

    // no compliance on ax18
  int cw_compliance_margin = 1;
  int ccw_compliance_margin = 1;
  int cw_compliance_slope = 32;
  int ccw_compliance_slope = 32;
  for(int i=1; i<=6;i++)//ax18
  {
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl::cw_compliance_margin,cw_compliance_margin));
    _controller.recv(READ_DURATION, _status);
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl::ccw_compliance_margin,ccw_compliance_margin));
    _controller.recv(READ_DURATION, _status);
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl:: cw_compliance_slope,cw_compliance_slope));
    _controller.recv(READ_DURATION, _status);
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl:: ccw_compliance_slope,ccw_compliance_slope));
    _controller.recv(READ_DURATION, _status);


  }

  int P1=50;
  int I1=30;//254;
  int D1=0;

  int P2=P1;
  int I2=I1;
  int D2=D1;

  for(int i=11; i<=16;i++)
  {
    //D
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl::cw_compliance_margin,D1));
    _controller.recv(READ_DURATION, _status);
    //I
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl::ccw_compliance_margin,I1));
    _controller.recv(READ_DURATION, _status);
    //P
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl:: cw_compliance_slope,P1));
    _controller.recv(READ_DURATION, _status);



  }
  for(int i=21; i<=26;i++)
  {
    //D
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl::cw_compliance_margin,D2));
    _controller.recv(READ_DURATION, _status);

    //I
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl::ccw_compliance_margin,I2));
    _controller.recv(READ_DURATION, _status);

    //P
    _controller.send(dynamixel::ax12::WriteData(i,dynamixel::ax12::ctrl:: cw_compliance_slope,P2));
    _controller.recv(READ_DURATION, _status);

  }
}



void RobotHexa::applyCorrection(std::vector<int>& pos)
{
  assert(pos.size()==_correction.size());
  for(int i=0;i<pos.size();i++)
    pos[i]+= _correction[i];
}

void RobotHexa :: reset()
{
  try
  {
    if(_controller.isOpen()==false)
  	{
  	  std::cout<<"re-opening dynamixel's serial"<<std::endl;
  	  _controller.open_serial("/dev/ttyACM0",B1000000);
  	}
    _controller.flush();
  }
  catch (dynamixel::Error e)
  {
    std::cerr << "error (dynamixel): " << e.msg() << std::endl;
  }
#ifdef IMU
  try
  {

    if(_imu.isOpen()==false)
    {
      std::cout<<"re-opening imu's serial"<<std::endl;
      _imu.open_serial("/dev/ttyUSB1");
    }
    _imu.flush();
  }
  catch (imu::Error e)
  {
    std::cerr << "error (imu): " << e.msg() << std::endl;
  }
#endif
  std::cout << "setting all dynamixel to zero" << std::endl;
  //  setPID();

  enable();

  std::vector<int> pos(_actuators_ids.size());

  for (size_t i = 0; i < _actuators_ids.size(); ++i)
    if(_actuators_ids[i]>=20)
      pos[i]= 2048;
    else if (_actuators_ids[i] >= 10) // mx28
      pos[i] = 1024;
    else
      pos[i] = 2048;

  applyCorrection(pos);
  _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, pos));
  _controller.recv(READ_DURATION, _status);

  pos.clear();
  pos.resize(_actuators_ids.size());
  usleep(0.5e6);
  for (size_t i = 0; i < _actuators_ids.size(); ++i)
    if(_actuators_ids[i]>=20)
      pos[i]= 2048-512-256;
    else if (_actuators_ids[i] >= 10) // mx28
      pos[i] = 1024;
    else
      pos[i] = 2048;

  applyCorrection(pos);
  _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, pos));
  _controller.recv(READ_DURATION, _status);

  pos.clear();
  pos.resize(_actuators_ids.size());
  usleep(0.5e6);
  for (size_t i = 0; i < _actuators_ids.size(); ++i)
    if(_actuators_ids[i]>=20)
      pos[i]= 2048-256;
    else if (_actuators_ids[i] >= 10) // mx28
      pos[i] = 2048;
    else
      pos[i] = 2048;

  applyCorrection(pos);
  _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, pos));
  _controller.recv(READ_DURATION, _status);




  pos.clear();
  pos.resize(_actuators_ids.size());
  usleep(0.5e6);
  for (size_t i = 0; i < _actuators_ids.size(); ++i)
  {
	  pos[i] = 2048;
  }

  applyCorrection(pos);
  _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, pos));
  _controller.recv(READ_DURATION, _status);

  sleep(1);

  std::cout << "done" << std::endl;

}


void RobotHexa:: position_zero()
{
  std::cout << "initial position" << std::endl;
  enable();

  std::vector<int> pos(_actuators_ids.size());

  /*  for (size_t i = 0; i < _actuators_ids.size(); ++i)
      if(_actuators_ids[i]>=20)
      pos[i]= 2048;
      else if (_actuators_ids[i] >= 10) // mx28
      pos[i] = 1024;

      else
      pos[i] = 512;
      applyCorrection(pos);
      _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, pos));
      _controller.recv(READ_DURATION, _status);
  */



  pos.clear();
  pos.resize(_actuators_ids.size());
  sleep(1);
  for (size_t i = 0; i < _actuators_ids.size(); ++i)
  {
	  pos[i] = 2048;
  }

  applyCorrection(pos);
  _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, pos));
  _controller.recv(READ_DURATION, _status);

  usleep(0.5e6);


  std::cout << "done" << std::endl;


}




void RobotHexa :: _read_contacts()
{

  _controller.send(dynamixel::ax12::ReadData(11,dynamixel::ax12::ctrl::present_load_lo,2));
  if(  _controller.recv(READ_DURATION, _status))
  {
    if ((int)_status.decode16()>1024)
	  {
      _behavior_contact_0.push_back(-(int)_status.decode16()+1024);
	  }
    else
  	{

  	  _behavior_contact_0.push_back((int)_status.decode16());
  	}
  }
  else
    _behavior_contact_0.push_back(1024);//pas de reponse>>pas de contact



  _controller.send(dynamixel::ax12::ReadData(12,dynamixel::ax12::ctrl::present_load_lo,2));
  if(  _controller.recv(READ_DURATION, _status))
  {
    if ((int)_status.decode16()>1024)
	   _behavior_contact_1.push_back(-(int)_status.decode16()+1024);
    else
	   _behavior_contact_1.push_back((int)_status.decode16());
  }
  else
    _behavior_contact_1.push_back(1024);//pas de reponse>>pas de contact


  _controller.send(dynamixel::ax12::ReadData(13,dynamixel::ax12::ctrl::present_load_lo,2));
  if(  _controller.recv(READ_DURATION, _status))
  {
    if ((int)_status.decode16()>1024)
	   _behavior_contact_2.push_back(-(int)_status.decode16()+1024);
    else
	   _behavior_contact_2.push_back((int)_status.decode16());
  }
  else
    _behavior_contact_2.push_back(1024);//pas de reponse>>pas de contact


  _controller.send(dynamixel::ax12::ReadData(14,dynamixel::ax12::ctrl::present_load_lo,2));
  if(  _controller.recv(READ_DURATION, _status))
  {
    if ((int)_status.decode16()>1024)
	   _behavior_contact_3.push_back(-(int)_status.decode16()+1024);
    else
	   _behavior_contact_3.push_back((int)_status.decode16());
  }
  else
    _behavior_contact_3.push_back(1024);//pas de reponse>>pas de contact


  _controller.send(dynamixel::ax12::ReadData(15,dynamixel::ax12::ctrl::present_load_lo,2));
  if(  _controller.recv(READ_DURATION, _status))
  {
    if ((int)_status.decode16()>1024)
	   _behavior_contact_4.push_back(-(int)_status.decode16()+1024);
    else
	   _behavior_contact_4.push_back((int)_status.decode16());
  }
  else
    _behavior_contact_4.push_back(1024);//pas de reponse>>pas de contact


  _controller.send(dynamixel::ax12::ReadData(16,dynamixel::ax12::ctrl::present_load_lo,2));
  if(  _controller.recv(READ_DURATION, _status))
  {
    if ((int)_status.decode16()>1024)
	   _behavior_contact_5.push_back(-(int)_status.decode16()+1024);
    else
	   _behavior_contact_5.push_back((int)_status.decode16());
  }
  else
    _behavior_contact_5.push_back(1024);//pas de reponse>>pas de contact

}


void RobotHexa::contactSmoothing(int length)
{
  std::vector<float> smooth(_behavior_contact_0.size());

  for (int i=0;i<_behavior_contact_0.size();i++)
  {
    int k=0;
    for (int j=-length;j<=length;j++)
    {
      if (i+j>=0 && i+j<_behavior_contact_0.size())
      {
        smooth[i]+=_behavior_contact_0[i+j];
        k++;
      }
    }
    smooth[i]/=(float)k;

  }
  _behavior_contact_0.clear();




  for (int i=0;i<smooth.size();i++)
  {
    if (smooth[i]>0)
      _behavior_contact_0.push_back(0);
    else
      _behavior_contact_0.push_back(1);
  }

  smooth.clear();
  smooth.resize(_behavior_contact_1.size());
  for (int i=0;i<_behavior_contact_1.size();i++)
  {
    int k=0;
    for (int j=-length;j<=length;j++)
    {
	    if (i+j>=0 && i+j<_behavior_contact_1.size())
      {
	      smooth[i]+=_behavior_contact_1[i+j];
	      k++;
      }
    }
    smooth[i]/=(float)k;
  }
  _behavior_contact_1.clear();
  for (int i=0;i<smooth.size();i++)
  {
    if (smooth[i]>0)
	   _behavior_contact_1.push_back(0);
    else
	   _behavior_contact_1.push_back(1);
  }

  smooth.clear();
  smooth.resize(_behavior_contact_2.size());
  for (int i=0;i<_behavior_contact_2.size();i++)
  {
    int k=0;
    for (int j=-length;j<=length;j++)
    {
      if (i+j>=0 && i+j<_behavior_contact_2.size())
      {
        smooth[i]+=_behavior_contact_2[i+j];
        k++;
      }
    }
      smooth[i]/=(float)k;
    }
  _behavior_contact_2.clear();
  for (int i=0;i<smooth.size();i++)
  {
    if (smooth[i]>0)
	   _behavior_contact_2.push_back(0);
    else
	   _behavior_contact_2.push_back(1);
  }

  smooth.clear();
  smooth.resize(_behavior_contact_3.size());
  for (int i=0;i<_behavior_contact_3.size();i++)
  {
    int k=0;
    for (int j=-length;j<=length;j++)
    {
      if (i+j>=0 && i+j<_behavior_contact_3.size())
      {
	      smooth[i]+=_behavior_contact_3[i+j];
	      k++;
      }
    }
    smooth[i]/=(float)k;
  }
  _behavior_contact_3.clear();
  for (int i=0;i<smooth.size();i++)
  {
    if (smooth[i]>0)
	   _behavior_contact_3.push_back(0);
    else
	   _behavior_contact_3.push_back(1);
  }

  smooth.clear();
  smooth.resize(_behavior_contact_4.size());
  for (int i=0;i<_behavior_contact_4.size();i++)
  {
    int k=0;
    for (int j=-length;j<=length;j++)
    {
      if (i+j>=0 && i+j<_behavior_contact_4.size())
      {
        smooth[i]+=_behavior_contact_4[i+j];
        k++;
      }
    }
    smooth[i]/=(float)k;
  }
  _behavior_contact_4.clear();
  for (int i=0;i<smooth.size();i++)
  {
    if (smooth[i]>0)
	   _behavior_contact_4.push_back(0);
    else
	   _behavior_contact_4.push_back(1);
  }

  smooth.clear();
  smooth.resize(_behavior_contact_5.size());
  for (int i=0;i<_behavior_contact_5.size();i++)
  {
    int k=0;
    for (int j=-length;j<=length;j++)
    {
	    if (i+j>=0 && i+j<_behavior_contact_5.size())
      {
	      smooth[i]+=_behavior_contact_5[i+j];
	      k++;
      }
    }
    smooth[i]/=(float)k;
  }
  _behavior_contact_5.clear();
  for (int i=0;i<smooth.size();i++)
  {
    if (smooth[i]>0)
	   _behavior_contact_5.push_back(0);
    else
	   _behavior_contact_5.push_back(1);
  }
}

void RobotHexa ::write_contact(std::string const name)
{


  std::ofstream workingFile(name.c_str());

  if (workingFile)
  {
    for (int i =0;i<_behavior_contact_0.size();i++)
    {
      workingFile<<_behavior_contact_0[i]<<" "<<_behavior_contact_1[i]<<" "<<_behavior_contact_2[i]<<" "<<_behavior_contact_3[i]<<" "<<_behavior_contact_4[i]<<" "<<_behavior_contact_5[i]<<std::endl;
    }
  }
  else
  {
    std::cout << "ERROR: Impossible to open the file." << std::endl;
  }


}



void RobotHexa :: initRosNode(  int argc ,char** argv,boost::shared_ptr<ros::NodeHandle> node_p)
{
  //  ros::init(argc, argv, "AlgoTransf",ros::init_options::NoSigintHandler);
  _node_p = node_p;
  //    _chatter_pub  = _node_p->advertise<rgbdslam::SferesCmd>("sferes_cmd", 1);

}
/*void RobotHexa ::send_ros_start(int nbrun,int nbtrans)
  {
  rgbdslam::SferesCmd msg;
  msg.msg = "start";
  msg.run_number=nbrun;
  msg.transfer_number=nbtrans;


  _chatter_pub.publish(msg);
  }
  void RobotHexa ::send_ros_stop(int nbrun,int nbtrans)
  {
  rgbdslam::SferesCmd msg;
  msg.msg = "stop";
  msg.run_number=nbrun;
  msg.transfer_number=nbtrans;

  _chatter_pub.publish(msg);
  }*/


void RobotHexa:: getSlamInfo()
{
  this->_request_time=ros::Time::now();
  msg_recv=false;
  while(msg_recv ==false)
    ros::spinOnce();

}


void RobotHexa :: transfer(ControllerDuty& controller, float duration,int transfer_number)
{


  _behavior_contact_0.clear();
  _behavior_contact_1.clear();
  _behavior_contact_2.clear();
  _behavior_contact_3.clear();
  _behavior_contact_4.clear();
  _behavior_contact_5.clear();
  std::vector<int>pos=controller.get_pos_dyna(0,_correction);

  _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, pos));
  _controller.recv(READ_DURATION, _status);

  _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, pos));
  _controller.recv(READ_DURATION, _status);



  //std::cout<<"Waiting key " <<std::endl;
  //sleep();
  //std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
  // Init timers //////////////////////////
  struct timeval timev_init;  // Initial absolute time (static)
  struct timeval timev_prev;  // Previous tick absolute time
  struct timeval timev_cur;   // Current absolute time
  struct timeval timev_diff;  // Current tick position (curent - previous)
  struct timeval timev_duration;  // Duration of the movement (current - initial)
  unsigned int sampling_interval_us = 30000;
  float t=0;
  // Ticks loop ///////////////////////////
  bool first=true;
  _sub=_node_p->subscribe("vo",1,&RobotHexa::posCallback,this);
  getSlamInfo();
  //    send_ros_start(1,transfer_number);


  usleep(0.5e6);
  timerclear(&timev_init);
  gettimeofday(&timev_init, NULL);

  timev_prev = timev_init;


  int index=0;
  while (true)
  {
    gettimeofday(&timev_cur, NULL);
    timersub(&timev_cur, &timev_prev, &timev_diff);

    timersub(&timev_cur, &timev_init, &timev_duration);

    if (timev_duration.tv_sec >= duration)//*/index>=duration*1000000/sampling_interval_us)
    {
      std::cout<<"time duration "<<timev_duration.tv_sec<<"."<<timev_duration.tv_usec<<std::endl;


  	  usleep(0.5e6);
  	  //send_ros_stop(1,transfer_number);
  	  getSlamInfo();
  	  _sub.shutdown();
  	  contactSmoothing(2);
  	  break;
    }
    if (first)
    {
      first=false;

      _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, controller.get_pos_dyna(timev_duration.tv_sec+timev_duration.tv_usec/(float)1000000,_correction)));

      _read_contacts();


#ifdef IMU
      std::vector<float> vect;

      _imu.recv(READ_DURATION,vect);

      _imu_angles.push_back(vect);
#endif
  	  index++;
  	  //_controller.send(dynamixel::ax12::SetSpeeds(_wheels_ids, controller.get_directions_dyna(),controller.get_speeds_dyna()));
  	  //_controller.recv(READ_DURATION, _status);


    }



    // On fait un step de sampling_interval_us (on suppose qu'une step ne dépasse pas 1 sec)
    if (timev_diff.tv_usec >= sampling_interval_us)
    {
      //std::cout<<timev_diff.tv_usec<<std::endl;
      _controller.send(dynamixel::ax12::SetPositions(_actuators_ids, controller.get_pos_dyna(timev_duration.tv_sec+timev_duration.tv_usec/(float)1000000,_correction)));

	    _read_contacts();
#ifdef IMU
	    std::vector<float> vect;

  	  _imu.recv(READ_DURATION,vect);
  	  _imu_angles.push_back(vect);
#endif
  	  timev_prev = timev_cur;
  	  t+=0.030;
  	  index++;
    }

  }

}
