#include "controllerDuty.hpp"
#define RAD2DYNMX28 651.42
#define RAD2DYN 195.57


typedef boost::array<float,ARRAY_DIM> array_t;

array_t ControllerDuty::control_signal(float amplitude, float phase, float duty_cycle)
{
  // Dorian : initial command; +amplitude at start and -amplitude for the active
  //  part of the duty cycle (when touching the floor)
  array_t temp;
  int up_time=ARRAY_DIM*duty_cycle;
  for(int i=0;i<up_time;i++)
    temp[i]=amplitude;
  for(int i=up_time;i<ARRAY_DIM;i++)
    temp[i]=-amplitude;





  // filtering
  // Dorian : build a gaussian filter kernel
  int kernel_size=ARRAY_DIM/10;

  array_t command;

  std::vector<float> kernel(2*kernel_size+1,0);
  float sigma=kernel_size/3;

  float sum=0;
  for(int i=0;i<kernel.size();i++)
  {
    kernel[i]=exp(-(i-kernel_size)*(i-kernel_size)/(2*sigma*sigma))/(sigma*sqrt(M_PI));
    sum+=kernel[i];
  }


  // Dorian : apply the gaussian filter on the square command
  for(int i=0;i<ARRAY_DIM;i++)
  {
    command[i]=0;
    for(int d=1;d<=kernel_size;d++)
  	{
  	  if(i-d<0)
  	    command[i]+=temp[ARRAY_DIM+i-d]*kernel[kernel_size-d];
  	  else
  	    command[i]+=temp[i-d]*kernel[kernel_size-d];
  	}
    command[i]+=temp[i]*kernel[kernel_size];
    for(int d=1;d<=kernel_size;d++)
  	{
  	  if(i+d>=ARRAY_DIM)
  	    command[i]+=temp[i+d-ARRAY_DIM]*kernel[kernel_size+d];
  	  else
  	    command[i]+=temp[i+d]*kernel[kernel_size+d];
  	}

    command[i]/=sum;
  }

  /*
  for(int i=0;i<ARRAY_DIM;i++)
    {
      command[i]=0;
      for(int d=1;d<=kernel_size;d++)
	{
	  if(i-d<0)
	    command[i]+=temp[ARRAY_DIM+i-d]*(kernel_size-d+1);
	  else
	    command[i]+=temp[i-d]*(kernel_size-d+1);
	}
      command[i]+=temp[i]*(kernel_size+1);
      for(int d=1;d<=kernel_size;d++)
	{
	  if(i+d>=ARRAY_DIM)
	    command[i]+=temp[i+d-ARRAY_DIM]*(kernel_size-d+1);
	  else
	    command[i]+=temp[i+d]*(kernel_size-d+1);
	}

      command[i]/=(kernel_size+1)*(kernel_size+1);
    }
  */


  // apply phase
  array_t final_command;
  int current=0;
  int start=floor(ARRAY_DIM*phase);
  for(int i=start;i<ARRAY_DIM;i++)
  {
    final_command[current]=command[i];
    current++;
  }
  for(int i=0;i<start;i++)
  {
    final_command[current]=command[i];
    current++;
  }


  return final_command;
}



/*void ControllerDuty::moveRobot(robot_t& robot, float t)
{
    size_t leg = 0;
    for (size_t i = 0; i < robot->servos().size(); i+=3)
    {
      //    std::cout<<"dans move"<<std::endl;
       for (int j=0;j<_brokenLegs.size();j++)
        {
            if (leg==_brokenLegs[j])
            {
                leg++;
                if (_brokenLegs.size()>j+1 && _brokenLegs[j+1]!=leg)
                    break;
            }
	}

       switch(leg)
	 {
	 case 0:
	   robot->servos()[i]->set_angle(0,M_PI/8*_legs0commands[0][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+1]->set_angle(0,M_PI/4*_legs0commands[1][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+2]->set_angle(0,-M_PI/4*_legs0commands[2][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   break;

	 case 1:
	   robot->servos()[i]->set_angle(0,M_PI/8*_legs1commands[0][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+1]->set_angle(0,M_PI/4*_legs1commands[1][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+2]->set_angle(0,-M_PI/4*_legs1commands[2][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   break;

	 case 2:
	   robot->servos()[i]->set_angle(0,M_PI/8*_legs2commands[0][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+1]->set_angle(0,M_PI/4*_legs2commands[1][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+2]->set_angle(0,-M_PI/4*_legs2commands[2][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   break;

	 case 3:
	   robot->servos()[i]->set_angle(0,M_PI/8*_legs3commands[0][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+1]->set_angle(0,M_PI/4*_legs3commands[1][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+2]->set_angle(0,-M_PI/4*_legs3commands[2][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   break;

	 case 4:
	   robot->servos()[i]->set_angle(0,M_PI/8*_legs4commands[0][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+1]->set_angle(0,M_PI/4*_legs4commands[1][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+2]->set_angle(0,-M_PI/4*_legs4commands[2][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   break;

	 case 5:
	   robot->servos()[i]->set_angle(0,M_PI/8*_legs5commands[0][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+1]->set_angle(0,M_PI/4*_legs5commands[1][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   robot->servos()[i+2]->set_angle(0,-M_PI/4*_legs5commands[2][((int)floor(t*100))%100]);  // marche que pour ARRAY_DIM =100
	   break;



	 }

	++leg;
    }

}

*/

std::vector<int> ControllerDuty::get_pos_dyna(float t, const std::vector<int>& correction )
{
    std::vector<int> pos;
    //std::cout<<"debut move"<<std::endl;
    size_t leg = 0;
    for (size_t i = 0; i < 24; i+=4)
    {
      //std::cout<<"dans move"<<std::endl;
      for (int j=0;j<_brokenLegs.size();j++)
      {
        if (leg==_brokenLegs[j])
        {
          leg++;
          if (_brokenLegs.size()>j+1 && _brokenLegs[j+1]!=leg)
            break;
        }
      }

      float theta0;
      float theta1;
      float theta2;

      switch(leg)
      {
        std::cout<<"COUCOU"<<std::endl;
        case 0:
          theta0= M_PI/8*_legs0commands[0][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta1= M_PI/4*_legs0commands[1][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta2= M_PI/4*_legs0commands[2][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100

          pos.push_back(2048+correction[0]-RAD2DYNMX28*(theta0));
          pos.push_back(2048+correction[1]-RAD2DYNMX28*(theta1));
          pos.push_back(2048+correction[2]+RAD2DYNMX28*(-theta2));
          break;

        case 1:
          theta0= M_PI/8*_legs1commands[0][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta1= M_PI/4*_legs1commands[1][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta2= M_PI/4*_legs1commands[2][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          pos.push_back(2048+correction[3]-RAD2DYNMX28*(theta0));
          pos.push_back(2048+correction[4]-RAD2DYNMX28*(theta1));
          pos.push_back(2048+correction[5]+RAD2DYNMX28*(-theta2));

          break;

        case 2:
          theta0= M_PI/8*_legs2commands[0][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta1= M_PI/4*_legs2commands[1][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta2= M_PI/4*_legs2commands[2][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100

          pos.push_back(2048+correction[6]-RAD2DYNMX28*(theta0));
          pos.push_back(2048+correction[7]-RAD2DYNMX28*(theta1));
          pos.push_back(2048+correction[8]+RAD2DYNMX28*(-theta2));

          break;

        case 3:
          theta0= M_PI/8*_legs3commands[0][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta1= M_PI/4*_legs3commands[1][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta2= M_PI/4*_legs3commands[2][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100

          pos.push_back(2048+correction[9]-RAD2DYNMX28*(theta0));
          pos.push_back(2048+correction[10]-RAD2DYNMX28*(theta1));
          pos.push_back(2048+correction[11]+RAD2DYNMX28*(-theta2));

          break;

        case 4:
          theta0= M_PI/8*_legs4commands[0][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta1= M_PI/4*_legs4commands[1][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta2= M_PI/4*_legs4commands[2][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          pos.push_back(2048+correction[12]-RAD2DYNMX28*(theta0));
          pos.push_back(2048+correction[13]-RAD2DYNMX28*(theta1));
          pos.push_back(2048+correction[14]+RAD2DYNMX28*(-theta2));

          break;

        case 5:
          theta0= M_PI/8*_legs5commands[0][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta1= M_PI/4*_legs5commands[1][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100
          theta2= M_PI/4*_legs5commands[2][((int)floor(t*100))%100];  // marche que pour ARRAY_DIM =100

          pos.push_back(2048+correction[15]-RAD2DYNMX28*(theta0));
          pos.push_back(2048+correction[16]-RAD2DYNMX28*(theta1));
          pos.push_back(2048+correction[17]+RAD2DYNMX28*(-theta2));

          break;

      }

      ++leg;
    }
    return pos;
}

/*
float ControllerPhase::delayedPhase(float t, float phi)
{

    return tanh(sin(2*M_PI*t+phi*2*M_PI)*4);

}
*/
